#ifndef MY_ROBOT_INTERFACE_HPP
#define MY_ROBOT_INTERFACE_HPP

#include <rclcpp/rclcpp.hpp>
#include <hardware_interface/system_interface.hpp>
#include <rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp>
#include <rclcpp_lifecycle/state.hpp>
#include "my_robot_firmware/can_node.hpp"

#include <vector>
#include <mutex>

namespace carrierbot_firmware
{
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
    class CarrierbotInterface : public hardware_interface::SystemInterface
    {
        public: 
            CarrierbotInterface();
            virtual ~CarrierbotInterface();

            virtual CallbackReturn on_activate(const rclcpp_lifecycle::State &previous_state) override;

            virtual CallbackReturn on_deactivate(const rclcpp_lifecycle::State &previous_state) override;

            virtual CallbackReturn on_init(const hardware_interface::HardwareInfo &hardware_info) override;

            virtual std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

            virtual std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

            virtual hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) override;
            
            virtual hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override;



        private:
            WaveshareCAN* can_interface_;
            
            std::string port_;
            std::vector<double> velocity_command_;
            std::vector<double> position_state_;
            std::vector<double> velocity_state_;
            std::mutex state_mutex_;

            rclcpp::Time last_run_;
    };
}

#endif