#ifndef CAN_SUBSCRIBER_HPP_
#define CAN_SUBSCRIBER_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "rclcpp/rclcpp.hpp"
#define PI 3.14159265358979323846

class CanSubscriberNode : public rclcpp::Node {
public:
    CanSubscriberNode() : Node("can_subscriber_node") {
        // 1. Create a socket
        can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_socket_ < 0) {
            RCLCPP_ERROR(this->get_logger(), "Error while opening socket");
            return;
        }

        // 2. Specify the interface name (e.g., vcan0)
        std::strcpy(ifr_.ifr_name, "vcan0");
        ioctl(can_socket_, SIOCGIFINDEX, &ifr_);

        std::memset(&addr_, 0, sizeof(addr_));
        addr_.can_family = AF_CAN;
        addr_.can_ifindex = ifr_.ifr_ifindex;

        // 3. Bind the socket to the network interface
        if (bind(can_socket_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) {
            RCLCPP_ERROR(this->get_logger(), "Error in socket bind");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Successfully bound to vcan0. Waiting for messages...");

        // 4. Create a timer to poll for CAN frames (non-blocking or small timeout)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(10), 
            std::bind(&CanSubscriberNode::receive_can_frame, this)
        );
    }

    ~CanSubscriberNode() {
        if (can_socket_ >= 0) {
            close(can_socket_);
        }
    }

private:
    void receive_can_frame() {
        struct can_frame frame;
        // Using recv with MSG_DONTWAIT to avoid blocking the ROS2 executor
        int nbytes = recv(can_socket_, &frame, sizeof(struct can_frame), MSG_DONTWAIT);

        if (nbytes > 0) {
            // RCLCPP_INFO(this->get_logger(), "Received CAN Frame - ID: [0x%X] DLC: [%d]", 
            //             frame.can_id, frame.can_dlc);
            
            std::stringstream ss;
            for (int i = 0; i < frame.can_dlc; i++) {
                ss << std::hex << static_cast<int>(frame.data[i]) << " ";
            }
            // RCLCPP_INFO(this->get_logger(), "Data: %s", ss.str().c_str());

            // Switch case để xử lý từng CAN ID
            switch(frame.can_id) {
                case 0x11:
                    // Xử lý ID 0x11
                    handle_can_0x11(frame);
                    break;

                case 0x200:
                    // Xử lý ID 0x200
                    handle_can_0x200(frame);
                    break;

                case 0x300:
                    // Xử lý ID 0x300
                    handle_can_0x300(frame);
                    break;

                case 0x400:
                    // Xử lý ID 0x400
                    handle_can_0x400(frame);
                    break;

                default:
                    RCLCPP_WARN(this->get_logger(), "Unknown CAN ID: 0x%X", frame.can_id);
                    break;
            }
        }
    }

    // Nhận vận tốc từ vi điều khiển (CAN ID 0x11)
    void handle_can_0x11(const struct can_frame& frame) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x11");
        if (frame.can_dlc != 8) {
            RCLCPP_WARN(this->get_logger(), "Unexpected data length for CAN ID 0x11: %d", frame.can_dlc);
            return;
        }

        // Extract left velocity from first 4 bytes
        std::memcpy(&received_left_vel, &frame.data[0], sizeof(int));
        // Extract right velocity from last 4 bytes
        std::memcpy(&received_right_vel, &frame.data[4], sizeof(int));

        RCLCPP_INFO(this->get_logger(), "Received Velocities - Left: %d, Right: %d", 
        received_left_vel, received_right_vel);

        // Convert pulses to linear velocity (m/s)
        left_mps = ConvertVelocityFromPulse(received_left_vel);
        right_mps = ConvertVelocityFromPulse(received_right_vel);  
        RCLCPP_INFO(this->get_logger(), "Converted Velocities - Left: %.2f m/s, Right: %.2f m/s",
        left_mps, right_mps);

    }

    // Hàm xử lý cho CAN ID 0x200
    void handle_can_0x200(const struct can_frame& frame) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x200");
        // TODO: Thêm logic xử lý cho ID 0x200
    }

    // Hàm xử lý cho CAN ID 0x300
    void handle_can_0x300(const struct can_frame& frame) {
        RCLCPP_INFO(this->get_logger(), "Processing CAN ID 0x300");
        // TODO: Thêm logic xử lý cho ID 0x300
    }

    // Hàm xử lý cho CAN ID 0x400
    void handle_can_0x400(const struct can_frame& frame) {
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

    int can_socket_;
    struct sockaddr_can addr_;
    struct ifreq ifr_;
    rclcpp::TimerBase::SharedPtr timer_;
    int received_left_vel;
    int received_right_vel;

    float left_mps;
    float right_mps;

};

#endif  // CAN_SUBSCRIBER_HPP_