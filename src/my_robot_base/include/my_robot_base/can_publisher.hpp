#ifndef CAN_PUBLISHER_HPP_
#define CAN_PUBLISHER_HPP_

#include <iostream>
#include <cstring>
#include "rclcpp/rclcpp.hpp"
#include "my_robot_msgs/msg/can_frame.hpp"
#include "my_robot_base/can_node.hpp"

class CanPublisherNode : public rclcpp::Node {
public:
    CanPublisherNode() : Node("can_publisher_node"), can_("/dev/usbcan", 2000000, 2.0) {
        try {
            // Open CAN interface
            RCLCPP_INFO(this->get_logger(), "Opening CAN interface...");
            can_.open();
            
            RCLCPP_INFO(this->get_logger(), "CAN Publisher started on /dev/usbcan");

            // Create a subscriber to listen to /can_frame topic
            subscription_ = this->create_subscription<my_robot_msgs::msg::CanFrame>(
                "/can_frame",
                10,
                std::bind(&CanPublisherNode::callback_can_frame, this, std::placeholders::_1)
            );

            RCLCPP_INFO(this->get_logger(), "Subscribed to /can_frame topic");
        }
        catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error initializing CAN Publisher: %s", e.what());
        }
    }

    ~CanPublisherNode() {
        try {
            can_.close();
        }
        catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error closing CAN: %s", e.what());
        }
    }

private:
    WaveshareCAN can_;
    rclcpp::Subscription<my_robot_msgs::msg::CanFrame>::SharedPtr subscription_;

    // Callback function khi nhận được message từ topic
    void callback_can_frame(const my_robot_msgs::msg::CanFrame::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Received CAN Frame - ID: 0x%X, Data Length: %zu",
                    msg->can_id, msg->data.size());
        
        // Gửi CAN frame với dữ liệu từ message
        send_can_frame(msg);
    }

    void send_can_frame(const my_robot_msgs::msg::CanFrame::SharedPtr msg) {
        try {
            // Convert to std::vector<uint8_t>
            std::vector<uint8_t> data(msg->data.begin(), msg->data.end());
            
            // Send CAN frame
            can_.send(msg->can_id, data);
            
            RCLCPP_INFO(this->get_logger(), "Sent CAN Frame: ID 0x%X, Data Length: %zu",
                       msg->can_id, data.size());
        }
        catch (const std::exception& e) {
            RCLCPP_ERROR(this->get_logger(), "Error sending CAN frame: %s", e.what());
        }
    }
};

#endif  // CAN_PUBLISHER_HPP_