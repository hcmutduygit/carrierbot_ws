#include <my_robot_base/can_subscriber.hpp>


int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CanSubscriberNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}