#include "carrierbot_firmware/my_robot_interface.hpp"
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <cstring>

namespace carrierbot_firmware
{

    CarrierbotInterface::CarrierbotInterface()
        : rclcpp::Node("carrierbot_interface")
    {
        can_interface_ = nullptr;
    }

    CarrierbotInterface::~CarrierbotInterface()
    {
        if (can_interface_ != nullptr)
        {
            try
            {
                can_interface_->close();
            }
            catch (const std::exception &e)
            {
                RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                                    "Failed to close CAN port: " << e.what());
            }
            delete can_interface_;
            can_interface_ = nullptr;
        }
    }

    //
    // 🔥 FOXy: configure thay cho on_init
    //
    hardware_interface::return_type CarrierbotInterface::configure(
        const hardware_interface::HardwareInfo &info)
    {
        info_ = info;

        try
        {
            port_ = info_.hardware_parameters.at("port");
        }
        catch (const std::out_of_range &)
        {
            RCLCPP_FATAL(rclcpp::get_logger("CarrierbotInterface"), "No CAN port specified!");
            return hardware_interface::return_type::ERROR;
        }

        if (can_interface_ != nullptr)
        {
            delete can_interface_;
        }

        can_interface_ = new WaveshareCAN(port_, 2000000, 2.0);

        velocity_command_.resize(info_.joints.size(), 0.0);
        position_state_.resize(info_.joints.size(), 0.0);
        velocity_state_.resize(info_.joints.size(), 0.0);

        last_run_ = rclcpp::Clock().now();

        return hardware_interface::return_type::OK;
    }

    //
    // Export state
    //
    std::vector<hardware_interface::StateInterface>
    CarrierbotInterface::export_state_interfaces()
    {
        std::vector<hardware_interface::StateInterface> state_interfaces;

        for (size_t i = 0; i < info_.joints.size(); ++i)
        {
            state_interfaces.emplace_back(
                info_.joints[i].name,
                hardware_interface::HW_IF_POSITION,
                &position_state_[i]);

            state_interfaces.emplace_back(
                info_.joints[i].name,
                hardware_interface::HW_IF_VELOCITY,
                &velocity_state_[i]);
        }

        return state_interfaces;
    }

    //
    // Export command
    //
    std::vector<hardware_interface::CommandInterface>
    CarrierbotInterface::export_command_interfaces()
    {
        std::vector<hardware_interface::CommandInterface> command_interfaces;

        for (size_t i = 0; i < info_.joints.size(); ++i)
        {
            command_interfaces.emplace_back(
                info_.joints[i].name,
                hardware_interface::HW_IF_VELOCITY,
                &velocity_command_[i]);
        }

        return command_interfaces;
    }

    //
    // 🔥 FOXy: start thay on_activate
    //
    hardware_interface::return_type CarrierbotInterface::start()
    {
        RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "Starting robot hardware");

        try
        {
            if (can_interface_ != nullptr)
            {
                can_interface_->open();

                // Create subscription
                cmd_vel_wheels_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
                    "/cmd_vel_wheels",
                    10,
                    std::bind(&CarrierbotInterface::cmdVelWheelsCallback, this, std::placeholders::_1));
                
                // Create executor and add this node properly
                executor_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
                
                stop_subscription_thread_ = false;
                subscription_thread_ = std::thread(&CarrierbotInterface::subscriptionSpinnerThread, this);
                
                RCLCPP_INFO(get_logger(), "Created background thread for /cmd_vel_wheels subscription");

                auto callback = [this](uint16_t can_id, const std::vector<uint8_t> &data)
                {
                    if (can_id == 0x11 && data.size() >= 8)
                    {
                        std::lock_guard<std::mutex> lock(state_mutex_);

                        auto now = rclcpp::Clock().now();
                        double dt = (now - last_run_).seconds();

                        int32_t left_velocity_raw = 0;
                        std::memcpy(&left_velocity_raw, &data[0], sizeof(int32_t));
                        velocity_state_.at(1) = left_velocity_raw / 100.0;
                        position_state_.at(1) += velocity_state_.at(1) * dt;

                        int32_t right_velocity_raw = 0;
                        std::memcpy(&right_velocity_raw, &data[4], sizeof(int32_t));
                        velocity_state_.at(0) = right_velocity_raw / 100.0;
                        position_state_.at(0) += velocity_state_.at(0) * dt;

                        last_run_ = now;
                    }
                };

                can_interface_->start_receive_loop(callback);
            }
            else
            {
                RCLCPP_FATAL(rclcpp::get_logger("CarrierbotInterface"),
                             "CAN interface not initialized!");
                return hardware_interface::return_type::ERROR;
            }
        }
        catch (const std::exception &e)
        {
            RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                                "Failed to open CAN port: " << port_);
            return hardware_interface::return_type::ERROR;
        }

        return hardware_interface::return_type::OK;
    }

    //
    // Callback for wheel velocity commands from /cmd_vel_wheels topic
    //
    void CarrierbotInterface::cmdVelWheelsCallback(const std::shared_ptr<const std_msgs::msg::Float64MultiArray> msg)
    {
        if (msg->data.size() < 2)
        {
            RCLCPP_WARN(get_logger(), "Invalid cmd_vel_wheels message. Expected [left_velocity, right_velocity]");
            return;
        }

        std::lock_guard<std::mutex> lock(state_mutex_);
        
        RCLCPP_INFO_STREAM(get_logger(), "CALLBACK INPUT: msg->data[0]=" << msg->data[0] 
                                         << " msg->data[1]=" << msg->data[1]);
        
        velocity_command_.at(0) = msg->data[1]; // right wheel velocity
        velocity_command_.at(1) = msg->data[0]; // left wheel velocity

        RCLCPP_INFO_STREAM(get_logger(), 
            "CALLBACK SET: velocity_command_[0](right)=" << velocity_command_.at(0) 
            << " velocity_command_[1](left)=" << velocity_command_.at(1));

        RCLCPP_INFO_STREAM(get_logger(), 
            "CMD Received: Left=" << velocity_command_.at(1) 
            << " m/s, Right=" << velocity_command_.at(0) << " m/s");
    }

    //
    // 🔥 FOXy: stop thay on_deactivate
    //
    hardware_interface::return_type CarrierbotInterface::stop()
    {
        RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "Stopping robot hardware");

        // Stop subscription spinner thread
        if (executor_)
        {
            stop_subscription_thread_ = true;
            if (subscription_thread_.joinable())
            {
                subscription_thread_.join();
            }
            executor_ = nullptr;
        }

        try
        {
            if (can_interface_ != nullptr)
            {
                can_interface_->close();
            }
        }
        catch (const std::exception &e)
        {
            RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                                "Failed to close CAN port: " << port_);
            return hardware_interface::return_type::ERROR;
        }

        return hardware_interface::return_type::OK;
    }

    //
    // Background thread to spin subscriptions
    //
    void CarrierbotInterface::subscriptionSpinnerThread()
    {
        RCLCPP_INFO(get_logger(), "Subscription spinner thread started");
        
        try
        {
            // Try to add this node to the executor
            auto node_base = get_node_base_interface();
            if (node_base)
            {
                executor_->add_node(node_base);
                RCLCPP_INFO(get_logger(), "Node added to executor successfully");
            }
            
            while (!stop_subscription_thread_ && rclcpp::ok())
            {
                executor_->spin_some(std::chrono::milliseconds(50));
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        catch (const std::exception& e)
        {
            RCLCPP_ERROR_STREAM(get_logger(), "Error in subscription spinner: " << e.what());
        }
        
        RCLCPP_INFO(get_logger(), "Subscription spinner thread stopped");
    }

    //
    // 🔥 FOXy: read không có time, period
    //
    hardware_interface::return_type CarrierbotInterface::read()
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        return hardware_interface::return_type::OK;
    }

    //
    // 🔥 FOXy: write không có time, period
    //
    hardware_interface::return_type CarrierbotInterface::write()
    {
        try
        {
            if (can_interface_ == nullptr)
            {
                return hardware_interface::return_type::OK;
            }

            std::lock_guard<std::mutex> lock(state_mutex_);
            
            RCLCPP_DEBUG_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                "WRITE: velocity_command_[0]=" << velocity_command_.at(0)
                << " velocity_command_[1]=" << velocity_command_.at(1));
            
            std::vector<uint8_t> can_data(8, 0);

            int32_t left_velocity = static_cast<int32_t>(velocity_command_.at(1));
            std::memcpy(&can_data[0], &left_velocity, sizeof(int32_t));

            int32_t right_velocity = static_cast<int32_t>(velocity_command_.at(0));
            std::memcpy(&can_data[4], &right_velocity, sizeof(int32_t));

            can_interface_->send(0x11, can_data);

            // Log CAN frame details
            RCLCPP_INFO_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                "CAN Send: ID=0x11 | "
                "Left Vel=" << velocity_command_.at(1) << " m/s | "
                "Right Vel=" << velocity_command_.at(0) << " m/s | "
                "LeftRaw=" << left_velocity << " RightRaw=" << right_velocity << " | "
                "Data=[" << std::hex 
                << (int)can_data[0] << " " << (int)can_data[1] << " " 
                << (int)can_data[2] << " " << (int)can_data[3] << " "
                << (int)can_data[4] << " " << (int)can_data[5] << " " 
                << (int)can_data[6] << " " << (int)can_data[7] << std::dec << "]");
        }
        catch (const std::exception &e)
        {
            RCLCPP_ERROR_STREAM(rclcpp::get_logger("CarrierbotInterface"),
                                "Failed to send CAN command: " << e.what());
            return hardware_interface::return_type::ERROR;
        }

        return hardware_interface::return_type::OK;
    }

    //
    // 🔥 BẮT BUỘC TRONG FOXY
    //
    std::string CarrierbotInterface::get_name() const
    {
        return info_.name;
    }

    hardware_interface::status CarrierbotInterface::get_status() const
    {
        return hardware_interface::status::STARTED;
    }

} // namespace carrierbot_firmware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(carrierbot_firmware::CarrierbotInterface,
                       hardware_interface::SystemInterface)