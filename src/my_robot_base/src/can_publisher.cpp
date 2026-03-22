#include <my_robot_base/can_publisher.hpp>

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CanPublisherNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}