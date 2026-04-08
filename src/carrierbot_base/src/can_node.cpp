#include "rclcpp/rclcpp.hpp"

class MyNode: public rclcpp::Node {
  public: 
    MyNode(): Node("cpp_test"), counter_(0){
      RCLCPP_INFO(this->get_logger(), "Hello Cpp Node!!!"); // First print

      timer_ = this->create_wall_timer(std::chrono::seconds(1), // wtf is this ???
          std::bind(&MyNode::timerCallback, this));
    }
  private:
    int counter_{};
    void timerCallback() {
      counter_++;
      RCLCPP_INFO(this->get_logger(), "Hello world %d", counter_); // Loop output
    }

    rclcpp::TimerBase::SharedPtr timer_; // = std:shared_ptr<rclcpp::TimerBase>
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MyNode>(); // Using OOP
  rclcpp::spin(node); // Hold the program
  rclcpp::shutdown();
  return 0;
}