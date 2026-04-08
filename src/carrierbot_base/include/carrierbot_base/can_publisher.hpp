#ifndef CAN_PUBLISHER_HPP_
#define CAN_PUBLISHER_HPP_

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "rclcpp/rclcpp.hpp"
#include "carrierbot_msgs/msg/can_frame.hpp"  // Include CanFrame message

class CanPublisherNode : public rclcpp::Node {
public:
    CanPublisherNode() : Node("can_publisher_node") {
        // 1. Create a socket for CAN communication
        can_socket_ = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (can_socket_ < 0) {
            RCLCPP_ERROR(this->get_logger(), "Socket creation failed!");
            return;
        }

        // 2. Locate the vcan0 interface
        std::strcpy(ifr_.ifr_name, "vcan0");
        if (ioctl(can_socket_, SIOCGIFINDEX, &ifr_) < 0) {
            RCLCPP_ERROR(this->get_logger(), "Failed to find vcan0 interface!");
            return;
        }

        addr_.can_family = AF_CAN;
        addr_.can_ifindex = ifr_.ifr_ifindex;

        // 3. Bind the socket to the interface
        if (bind(can_socket_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0) {
            RCLCPP_ERROR(this->get_logger(), "Socket bind failed!");
            return;
        }

        RCLCPP_INFO(this->get_logger(), "CAN Publisher started on vcan0.");

        // 4. Create a subscriber to listen to /can_frame topic
        subscription_ = this->create_subscription<carrierbot_msgs::msg::CanFrame>(
            "/can_frame",
            10,
            std::bind(&CanPublisherNode::callback_velocity, this, std::placeholders::_1)
        );

        RCLCPP_INFO(this->get_logger(), "Subscribed to /can_frame topic");
    }

    ~CanPublisherNode() {
        if (can_socket_ >= 0) close(can_socket_);
    }

private:
    // Callback function khi nhận được message từ topic
    void callback_velocity(const carrierbot_msgs::msg::CanFrame::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Received CAN Frame - ID: 0x%X, Data Length: %zu",
                    msg->can_id, msg->data.size());
        
        // Gửi CAN frame với dữ liệu từ message
        send_can_frame(msg);
    }

    void send_can_frame(const carrierbot_msgs::msg::CanFrame::SharedPtr msg) {
        struct can_frame frame;

        // Use CAN ID from message
        frame.can_id = msg->can_id;
        frame.can_dlc = msg->data.size();

        // Copy data from message to CAN frame
        for (size_t i = 0; i < msg->data.size() && i < 8; i++) {
            frame.data[i] = msg->data[i];
        }

        // Send CAN frame
        if (write(can_socket_, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            RCLCPP_ERROR(this->get_logger(), "Failed to send CAN frame!");
        } else {
            RCLCPP_INFO(this->get_logger(), "Sent CAN Frame: ID 0x%X, DLC: %d",
                       frame.can_id, frame.can_dlc);
        }
    }

    int can_socket_;
    struct sockaddr_can addr_;
    struct ifreq ifr_;
    rclcpp::Subscription<carrierbot_msgs::msg::CanFrame>::SharedPtr subscription_;
};

#endif  // CAN_PUBLISHER_HPP_