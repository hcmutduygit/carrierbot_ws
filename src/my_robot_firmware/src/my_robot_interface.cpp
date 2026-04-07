#include "my_robot_firmware/my_robot_interface.hpp"
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <cstring>

namespace carrierbot_firmware
{
    CarrierbotInterface::CarrierbotInterface()
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
            catch (const std::exception& e)
            {
                RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"), "Failed to close CAN port: " << e.what());
            }
            delete can_interface_;
            can_interface_ = nullptr;
        }
    }

    CallbackReturn CarrierbotInterface::on_init(const hardware_interface::HardwareInfo &hardware_info)
    {
        CallbackReturn result = hardware_interface::SystemInterface::on_init(hardware_info);
        if (result != CallbackReturn::SUCCESS)
        {
            return result;
        }

        try
        {
            port_ = info_.hardware_parameters.at("port");
        }
        catch (const std::out_of_range &e)
        {
            RCLCPP_FATAL(rclcpp::get_logger("CarrierbotInterface"), "No CAN port specified!");
            return CallbackReturn::FAILURE;
        }

        // Create WaveshareCAN instance with the specified port
        if (can_interface_ != nullptr)
        {
            delete can_interface_;
        }
        can_interface_ = new WaveshareCAN(port_, 2000000, 2.0);

        velocity_command_.reserve(info_.joints.size());
        position_state_.reserve(info_.joints.size());
        velocity_state_.reserve(info_.joints.size());
        last_run_ = rclcpp::Clock().now();

        return CallbackReturn::SUCCESS;
    }

    std::vector<hardware_interface::StateInterface> CarrierbotInterface::export_state_interfaces()
    {
        std::vector<hardware_interface::StateInterface> state_interfaces;
        for (size_t i = 0; i < info_.joints.size(); ++i)
        {
            state_interfaces.emplace_back(hardware_interface::StateInterface(info_.joints[i].name,
                                                                             hardware_interface::HW_IF_POSITION, &position_state_[i]));
            state_interfaces.emplace_back(hardware_interface::StateInterface(info_.joints[i].name,
                                                                             hardware_interface::HW_IF_VELOCITY, &velocity_state_[i]));
        }
        return state_interfaces;
    }

    std::vector<hardware_interface::CommandInterface> CarrierbotInterface::export_command_interfaces()
    {
        std::vector<hardware_interface::CommandInterface> command_interfaces;
        for (size_t i = 0; i < info_.joints.size(); ++i)
        {
            command_interfaces.emplace_back(hardware_interface::CommandInterface(info_.joints[i].name,
                                                                                 hardware_interface::HW_IF_VELOCITY, &velocity_command_[i]));
        }
        return command_interfaces;
    }

    CallbackReturn CarrierbotInterface::on_activate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "Starting robot hardware");
        velocity_command_ = {0.0, 0.0};
        position_state_ = {0.0, 0.0};
        velocity_state_ = {0.0, 0.0};

        try
        {
            if (can_interface_ != nullptr)
            {
                can_interface_->open();
                
                // Start background receive loop with callback
                auto callback = [this](uint16_t can_id, const std::vector<uint8_t>& data) {
                    if (can_id == 0x11 && data.size() >= 8)
                    {
                        std::lock_guard<std::mutex> lock(state_mutex_);
                        
                        auto dt = (rclcpp::Clock().now() - last_run_).seconds();
                        
                        // Extract left velocity from first 4 bytes
                        int32_t left_velocity_raw = 0;
                        std::memcpy(&left_velocity_raw, &data[0], sizeof(int32_t));
                        velocity_state_.at(1) = left_velocity_raw / 100.0;
                        position_state_.at(1) += velocity_state_.at(1) * dt;

                        // Extract right velocity from last 4 bytes
                        int32_t right_velocity_raw = 0;
                        std::memcpy(&right_velocity_raw, &data[4], sizeof(int32_t));
                        velocity_state_.at(0) = right_velocity_raw / 100.0;
                        position_state_.at(0) += velocity_state_.at(0) * dt;
                        
                        last_run_ = rclcpp::Clock().now();
                    }
                };
                
                can_interface_->start_receive_loop(callback);
            }
            else
            {
                RCLCPP_FATAL(rclcpp::get_logger("CarrierbotInterface"), "CAN interface not initialized!");
                return CallbackReturn::FAILURE;
            }
        }
        catch (const std::exception &e)
        {
            RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"), "Failed to open CAN port: " << port_);
            return CallbackReturn::FAILURE;
        }

        RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "Robot hardware started successfully");
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn CarrierbotInterface::on_deactivate(const rclcpp_lifecycle::State &previous_state)
    {
        RCLCPP_INFO(rclcpp::get_logger("CarrierbotInterface"), "Stopping robot hardware");
        try
        {
            if (can_interface_ != nullptr)
            {
                can_interface_->close();
            }
        }
        catch (const std::exception& e)
        {
            RCLCPP_FATAL_STREAM(rclcpp::get_logger("CarrierbotInterface"), "Failed to close CAN port: " << port_);
            return CallbackReturn::FAILURE;
        }
        return CallbackReturn::SUCCESS;
    }

    hardware_interface::return_type CarrierbotInterface::read(const rclcpp::Time &time, const rclcpp::Duration &period)
    {
        // State is updated by background receive loop thread
        // Just lock and read the state - no blocking
        try
        {
            if (can_interface_ == nullptr)
            {
                return hardware_interface::return_type::OK;
            }

            std::lock_guard<std::mutex> lock(state_mutex_);
            // State variables are already updated by receive callback
            // Nothing else to do here
        }
        catch (const std::exception& e)
        {
            RCLCPP_ERROR_STREAM(rclcpp::get_logger("CarrierbotInterface"), "Read error: " << e.what());
        }

        return hardware_interface::return_type::OK;
    }

    hardware_interface::return_type CarrierbotInterface::write(const rclcpp::Time &time, const rclcpp::Duration &period)
    {
        try
        {
            if (can_interface_ == nullptr)
            {
                return hardware_interface::return_type::OK;
            }

            // Prepare CAN frame with both wheel velocities (CAN ID 0x11)
            std::vector<uint8_t> can_data(8, 0);

            // Left wheel velocity (bytes 0-3)
            int32_t left_velocity = static_cast<int32_t>(velocity_command_.at(1) * 100);
            std::memcpy(&can_data[0], &left_velocity, sizeof(int32_t));

            // Right wheel velocity (bytes 4-7)
            int32_t right_velocity = static_cast<int32_t>(velocity_command_.at(0) * 100);
            std::memcpy(&can_data[4], &right_velocity, sizeof(int32_t));

            // Send via CAN ID 0x11
            can_interface_->send(0x11, can_data);

            // Send value 1 via CAN ID 0x50
            std::vector<uint8_t> control_data(8, 0);
            control_data[0] = 1;  // Send value 1
            can_interface_->send(0x50, control_data);
        }
        catch (const std::exception& e)
        {
            RCLCPP_ERROR_STREAM(rclcpp::get_logger("CarrierbotInterface"), "Failed to send CAN command: " << e.what());
            return hardware_interface::return_type::ERROR;
        }
        return hardware_interface::return_type::OK;
    }
}

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(carrierbot_firmware::CarrierbotInterface, hardware_interface::SystemInterface)