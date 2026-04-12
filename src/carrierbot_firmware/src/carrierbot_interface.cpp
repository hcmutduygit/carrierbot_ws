#include "carrierbot_firmware/carrierbot_interface.hpp"
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <cstring>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace carrierbot_firmware
{
    CarrierbotInterface::CarrierbotInterface()
        : rclcpp::Node("carrierbot_interface")
    {
        can_interface_ = nullptr;
    }

    CarrierbotInterface::~CarrierbotInterface()
    {
        // Close CSV file
        if (csv_file_.is_open())
        {
            csv_file_.close();
            RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "CSV file closed");
        }

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
    // Initialize CSV file for logging
    //
    void CarrierbotInterface::initCSV()
    {
        start_time_ = std::chrono::system_clock::now();
        
        auto time = std::chrono::system_clock::to_time_t(start_time_);
        struct tm* timeinfo = localtime(&time);
        
        char buffer[100];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeinfo);
        
        std::ostringstream filename;
        std::string pkg_path = ament_index_cpp::get_package_share_directory("carrierbot_firmware");
        filename << pkg_path << "/data/carrierbot_data_" << buffer << ".csv";

        csv_file_.open(filename.str(), std::ios::app);
        
        if (csv_file_.is_open())
        {
            // Write CSV header
            csv_file_ << "timestamp_ms,left_encoder_rps,right_encoder_rps,left_cmd_rps,right_cmd_rps\n";
            csv_file_.flush();
            RCLCPP_INFO_STREAM(rclcpp::get_logger("CarrierbotInterface"), "CSV file opened: " << filename.str());
        }
        else
        {
            RCLCPP_ERROR(rclcpp::get_logger("CarrierbotInterface"), "Failed to open CSV file");
        }
    }

    //
    // Log data to CSV
    //
    void CarrierbotInterface::logToCSV(float left_encoder, float right_encoder)
    {
        if (!csv_file_.is_open())
            return;

        auto now = std::chrono::system_clock::now();
        auto elapsed = now - start_time_;
        auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        csv_file_ << timestamp_ms << ","
                  << std::fixed << std::setprecision(4)
                  << left_encoder << ","
                  << right_encoder << ","
                  << velocity_command_.at(1) << ","
                  << velocity_command_.at(0) << "\n";
        csv_file_.flush();
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
            baudrate_ = std::stoi(info_.hardware_parameters.at("baudrate"));
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

        can_interface_ = new WaveshareCAN(port_, baudrate_, 2.0);

        velocity_command_.resize(info_.joints.size(), 0.0);
        position_state_.resize(info_.joints.size(), 0.0);
        velocity_state_.resize(info_.joints.size(), 0.0);

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

                // Initialize CSV logging
                initCSV();

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
                    if (can_id == 0x80)
                    {
                        handleEncoderData(data);
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
            << " RPS, Right=" << velocity_command_.at(0) << " RPS");
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
        rclcpp::Time now = this->get_clock()->now();
        
        // 👉 INIT lần đầu
        if (last_run_.nanoseconds() == 0)
        {
            last_run_ = now;
            return hardware_interface::return_type::OK;
        }

        double dt = (now - last_run_).seconds();
        if (dt <= 0.0 || dt > 1.0){
            dt = 0.01;
        }

        // Convert to rad/s
        double left_rad_s  = raw_left_rps_  / 10.0 * 2 * M_PI;
        double right_rad_s = raw_right_rps_ / 10.0 * 2 * M_PI;

        // Update velocity
        velocity_state_[0] = left_rad_s;
        velocity_state_[1] = right_rad_s;

        // Integrate to position (rad)
        position_state_[0] += velocity_state_[0] * dt;
        position_state_[1] += velocity_state_[1] * dt;

        last_run_ = now;
        return hardware_interface::return_type::OK;
    }

    //
    // Handle encoder data from CAN 0x80
    //
    void CarrierbotInterface::handleEncoderData(const std::vector<uint8_t> &data)
    {
        if (data.size() < 8)
            return;

        std::lock_guard<std::mutex> lock(state_mutex_);

        // auto now = rclcpp::Clock().now();
        // double dt = (now - last_run_).seconds();

        // // Parse left wheel encoder (bytes 0-3) - FLOAT
        // float left_velocity_float = 0.0f;
        // std::memcpy(&left_velocity_float, &data[0], sizeof(float));
        // velocity_state_.at(1) = left_velocity_float;
        // position_state_.at(1) += velocity_state_.at(1) * dt;

        // // Parse right wheel encoder (bytes 4-7) - FLOAT
        // float right_velocity_float = 0.0f;
        // std::memcpy(&right_velocity_float, &data[4], sizeof(float));
        // velocity_state_.at(0) = right_velocity_float;
        // position_state_.at(0) += velocity_state_.at(0) * dt;

        // left_velocity_float = left_velocity_float / 10.0 * 2*M_PI; 
        // right_velocity_float = right_velocity_float / 10.0 * 2*M_PI;

        // // Publish encoder velocities
        // carrierbot_msgs::msg::EncoderVelocity encoder_velocity_msg;
        // encoder_velocity_msg.left_rps = left_velocity_float;
        // encoder_velocity_msg.right_rps = right_velocity_float;
        // encoder_velocity_pub_->publish(encoder_velocity_msg);

        // // // Log encoder data
        // // RCLCPP_INFO_STREAM(rclcpp::get_logger("CarrierbotInterface"),
        // //     "CAN Recv Encoder 0x80: "
        // //     "[" << std::hex << std::setw(2) << std::setfill('0')
        // //     << (int)data[0] << " " << (int)data[1] << " " 
        // //     << (int)data[2] << " " << (int)data[3] << " "
        // //     << (int)data[4] << " " << (int)data[5] << " " 
        // //     << (int)data[6] << " " << (int)data[7] << std::dec << "] | "
        // //     "Left: " << std::fixed << std::setprecision(1) << left_velocity_float/10.0 << " RPS | "
        // //     "Right: " << std::fixed << std::setprecision(1) << right_velocity_float/10.0 << " RPS");

        // // Log to CSV
        // logToCSV(left_velocity_float, right_velocity_float);

        // last_run_ = now;

        std::memcpy(&raw_left_rps_,  &data[0], sizeof(float));
        std::memcpy(&raw_right_rps_, &data[4], sizeof(float));
    }

    //
    // Send wheel velocities to CAN 0x60 and 0x70
    //
    void CarrierbotInterface::sendWheelVelocities()
    {
        if (can_interface_ == nullptr)
            return;

        std::lock_guard<std::mutex> lock(state_mutex_);
        
        RCLCPP_DEBUG_STREAM(rclcpp::get_logger("CarrierbotInterface"),
            "WRITE: velocity_command_[0]=" << velocity_command_.at(0)
            << " velocity_command_[1]=" << velocity_command_.at(1));
        
        std::vector<uint8_t> right_vel_data(8, 0);
        std::vector<uint8_t> left_vel_data(8, 0);

        float left_velocity = static_cast<float>(velocity_command_.at(1)) * 10.0;
        std::memcpy(&left_vel_data[0], &left_velocity, sizeof(float));

        float right_velocity = static_cast<float>(velocity_command_.at(0)) * 10.0;
        std::memcpy(&right_vel_data[0], &right_velocity, sizeof(float));

        can_interface_->send(0x60, right_vel_data);
        can_interface_->send(0x70, left_vel_data);

        // // Log CAN send
        // RCLCPP_INFO_STREAM(rclcpp::get_logger("CarrierbotInterface"),
        //     "CAN Send 0x60(R): " << velocity_command_.at(0) << " RPS | "
        //     "0x70(L): " << velocity_command_.at(1) << " RPS");
    }

    //
    // 🔥 FOXy: write không có time, period
    //
    hardware_interface::return_type CarrierbotInterface::write()
    {
        try
        {
            sendWheelVelocities();
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