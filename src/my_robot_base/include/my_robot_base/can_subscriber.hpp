#ifndef CAN_SUBSCRIBER_HPP_
#define CAN_SUBSCRIBER_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include "rclcpp/rclcpp.hpp"
#include "my_robot_base/can_node.hpp"

#define PI 3.14159265358979323846

class CanSubscriberNode : public rclcpp::Node {
public:
    CanSubscriberNode() : Node("can_subscriber_node"), can_("/dev/usbcan", 2000000, 2.0) {
        RCLCPP_INFO(this->get_logger(), "Initializing CAN Subscriber Node...");
        
        try {
            // Open CAN interface
            RCLCPP_INFO(this->get_logger(), "Opening CAN interface...");
            can_.open();
            
            // Start receiving CAN frames in a separate thread
            RCLCPP_INFO(this->get_logger(), "Starting CAN receive loop...");
            can_.start_receive_loop([this](uint16_t can_id, const std::vector<uint8_t>& data) {
                this->process_can_frame(can_id, data);
            });
            
            RCLCPP_INFO(this->get_logger(), "CAN Subscriber initialized and waiting for messages...");
        }
        catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error initializing CAN: %s", e.what());
        }
    }

    ~CanSubscriberNode() {
        try {
            can_.close();
        }
        catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error closing CAN: %s", e.what());
        }
    }

private:
    WaveshareCAN can_;
    int received_left_vel;
    int received_right_vel;
    float left_mps;
    float right_mps;

    void process_can_frame(uint16_t can_id, const std::vector<uint8_t>& data) {
        if (data.size() == 0) return;
        
        // Switch case để xử lý từng CAN ID
        switch(can_id) {
            case 0x11:
                handle_can_0x11(data);
                break;

            case 0x200:
                handle_can_0x200(data);
                break;

            case 0x300:
                handle_can_0x300(data);
                break;

            case 0x400:
                handle_can_0x400(data);
                break;

            default:
                RCLCPP_WARN(this->get_logger(), "Unknown CAN ID: 0x%X", can_id);
                break;
        }
    }
    
    // Nhận vận tốc từ vi điều khiển (CAN ID 0x11)
    void handle_can_0x11(const std::vector<uint8_t>& data) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x11");
        
        if (data.size() < 8) {
            RCLCPP_WARN(this->get_logger(), "Unexpected data length for CAN ID 0x11: %zu", data.size());
            return;
        }

        // Extract left velocity from first 4 bytes
        std::memcpy(&received_left_vel, &data[0], sizeof(int));
        // Extract right velocity from last 4 bytes
        std::memcpy(&received_right_vel, &data[4], sizeof(int));

        RCLCPP_INFO(this->get_logger(), "Received Velocities - Left: %d, Right: %d", 
            received_left_vel, received_right_vel);

        // Convert pulses to linear velocity (m/s)
        left_mps = ConvertVelocityFromPulse(received_left_vel);
        right_mps = ConvertVelocityFromPulse(received_right_vel);  
        RCLCPP_INFO(this->get_logger(), "Converted Velocities - Left: %.2f m/s, Right: %.2f m/s",
            left_mps, right_mps);
    }

    // Hàm xử lý cho CAN ID 0x200
    void handle_can_0x200(const std::vector<uint8_t>& data) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x200");
        // TODO: Thêm logic xử lý cho ID 0x200
    }

    // Hàm xử lý cho CAN ID 0x300
    void handle_can_0x300(const std::vector<uint8_t>& data) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x300");
        // TODO: Thêm logic xử lý cho ID 0x300
    }

    // Hàm xử lý cho CAN ID 0x400
    void handle_can_0x400(const std::vector<uint8_t>& data) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x400");
        // TODO: Thêm logic xử lý cho ID 0x400
    }

    int ConvertPulse(float &velocity)
    {
        // Convert m/s to rounds per second (assuming wheel radius is 0.1 m)
        const float wheel_radius = 100;                                                                 // in millimeters
        const int pulse_per_revolution = 10000;                                                         // Assuming 360 pulses per revolution
        int pulse = static_cast<int>(pulse_per_revolution * velocity * 1000 / (2 * PI * wheel_radius)); // Convert m/s to pulses
        return pulse;                                                                                   // Pulse per second
    }

    // Inverse: convert pulses per second back to linear velocity (m/s)
    float ConvertVelocityFromPulse(int pulse)
    {
        // v (m/s) = (pulse / PPR) * circumference(mm) / 1000
        const float wheel_radius = 100.0f;      // in millimeters
        const int pulse_per_revolution = 10000; // pulses per wheel revolution
        const float circumference_mm = 2.0f * PI * wheel_radius;

        float velocity_mps = (static_cast<float>(pulse) * circumference_mm) /
                             (static_cast<float>(pulse_per_revolution) * 1000);
        return velocity_mps;
    }

};

#endif  // CAN_SUBSCRIBER_HPP_