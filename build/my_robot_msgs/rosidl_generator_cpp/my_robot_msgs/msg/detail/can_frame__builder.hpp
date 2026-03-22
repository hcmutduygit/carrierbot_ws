// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from my_robot_msgs:msg/CanFrame.idl
// generated code does not contain a copyright notice

#ifndef MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__BUILDER_HPP_
#define MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "my_robot_msgs/msg/detail/can_frame__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace my_robot_msgs
{

namespace msg
{

namespace builder
{

class Init_CanFrame_data
{
public:
  explicit Init_CanFrame_data(::my_robot_msgs::msg::CanFrame & msg)
  : msg_(msg)
  {}
  ::my_robot_msgs::msg::CanFrame data(::my_robot_msgs::msg::CanFrame::_data_type arg)
  {
    msg_.data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::my_robot_msgs::msg::CanFrame msg_;
};

class Init_CanFrame_can_id
{
public:
  Init_CanFrame_can_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_CanFrame_data can_id(::my_robot_msgs::msg::CanFrame::_can_id_type arg)
  {
    msg_.can_id = std::move(arg);
    return Init_CanFrame_data(msg_);
  }

private:
  ::my_robot_msgs::msg::CanFrame msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::my_robot_msgs::msg::CanFrame>()
{
  return my_robot_msgs::msg::builder::Init_CanFrame_can_id();
}

}  // namespace my_robot_msgs

#endif  // MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__BUILDER_HPP_
