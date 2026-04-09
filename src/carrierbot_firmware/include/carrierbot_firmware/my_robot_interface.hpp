#pragma once

#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/handle.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/executors/single_threaded_executor.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include "carrierbot_firmware/can_node.hpp"

#include <vector>
#include <string>
#include <mutex>
#include <thread>

namespace carrierbot_firmware
{
    class CarrierbotInterface : public hardware_interface::SystemInterface, public rclcpp::Node
    {
    public:
        CarrierbotInterface();
        virtual ~CarrierbotInterface();

        // Foxy uses configure instead of on_init
        hardware_interface::return_type configure(
            const hardware_interface::HardwareInfo & info) override;

        // Export interfaces
        std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
        std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

        // Start/Stop instead of activate/deactivate
        hardware_interface::return_type start() override;
        hardware_interface::return_type stop() override;

        // Read/Write (no time, period in Foxy)
        hardware_interface::return_type read() override;
        hardware_interface::return_type write() override;

        // REQUIRED in Foxy
        std::string get_name() const override;
        hardware_interface::status get_status() const override;

    private:
        // Callback for wheel velocity commands
        void cmdVelWheelsCallback(const std::shared_ptr<const std_msgs::msg::Float64MultiArray> msg);
        void subscriptionSpinnerThread();

        // Save hardware info manually (Foxy không có sẵn info_)
        hardware_interface::HardwareInfo info_;

        WaveshareCAN* can_interface_;

        std::string port_;
        std::vector<double> velocity_command_;
        std::vector<double> position_state_;
        std::vector<double> velocity_state_;
        std::mutex state_mutex_;

        rclcpp::Time last_run_;
        rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr cmd_vel_wheels_sub_;
        
        // Background thread for subscription processing
        std::thread subscription_thread_;
        std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
        bool stop_subscription_thread_ = false;
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base_interface_;
    };
}