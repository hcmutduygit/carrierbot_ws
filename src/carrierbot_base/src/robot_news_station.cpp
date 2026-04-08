#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

class RobotNewsStationNode : public rclcpp::Node
{
public:
  RobotNewsStationNode() : Node("robot_news_station"), robot_name_("carrierbot") // Name
  {
    publisher_ = this->create_publisher<example_interfaces::msg::String>("robot_news", 10); // Topic
    timer_ = this->create_wall_timer(std::chrono::milliseconds(500),
                                     std::bind(&RobotNewsStationNode::publishNew, this)); // Timer

    RCLCPP_INFO(this->get_logger(), "Robot News Station has been started!");
  }

private:
  void publishNew()
  {
    auto msg = example_interfaces::msg::String();
    msg.data = std::string("Hello this is ") + robot_name_ + std::string(" nigga from the Robot News Station");
    publisher_->publish(msg); // Publish the message
  }
  std::string robot_name_;
  rclcpp::Publisher<example_interfaces::msg::String>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<RobotNewsStationNode>(); // Initialize the node
  rclcpp::spin(node);                     // Hold the program
  rclcpp::shutdown();
  return 0;
}
