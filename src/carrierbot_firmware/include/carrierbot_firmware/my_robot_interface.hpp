#pragma once

#include <hardware_interface/system_interface.hpp>
#include <hardware_interface/types/hardware_interface_return_values.hpp>
#include <hardware_interface/handle.hpp>
#include <rclcpp/rclcpp.hpp>
#include "carrierbot_firmware/can_node.hpp"

#include <vector>
#include <string>
#include <mutex>

namespace carrierbot_firmware
{
    class CarrierbotInterface : public hardware_interface::SystemInterface
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
        // Save hardware info manually (Foxy không có sẵn info_)
        hardware_interface::HardwareInfo info_;

        WaveshareCAN* can_interface_;

        std::string port_;
        std::vector<double> velocity_command_;
        std::vector<double> position_state_;
        std::vector<double> velocity_state_;
        std::mutex state_mutex_;

        rclcpp::Time last_run_;
    };
}