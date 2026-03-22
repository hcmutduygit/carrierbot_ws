// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from my_robot_msgs:msg/CanFrame.idl
// generated code does not contain a copyright notice

#ifndef MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__TRAITS_HPP_
#define MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "my_robot_msgs/msg/detail/can_frame__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace my_robot_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const CanFrame & msg,
  std::ostream & out)
{
  out << "{";
  // member: can_id
  {
    out << "can_id: ";
    rosidl_generator_traits::value_to_yaml(msg.can_id, out);
    out << ", ";
  }

  // member: data
  {
    if (msg.data.size() == 0) {
      out << "data: []";
    } else {
      out << "data: [";
      size_t pending_items = msg.data.size();
      for (auto item : msg.data) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const CanFrame & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: can_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "can_id: ";
    rosidl_generator_traits::value_to_yaml(msg.can_id, out);
    out << "\n";
  }

  // member: data
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.data.size() == 0) {
      out << "data: []\n";
    } else {
      out << "data:\n";
      for (auto item : msg.data) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const CanFrame & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace my_robot_msgs

namespace rosidl_generator_traits
{

[[deprecated("use my_robot_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const my_robot_msgs::msg::CanFrame & msg,
  std::ostream & out, size_t indentation = 0)
{
  my_robot_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use my_robot_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const my_robot_msgs::msg::CanFrame & msg)
{
  return my_robot_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<my_robot_msgs::msg::CanFrame>()
{
  return "my_robot_msgs::msg::CanFrame";
}

template<>
inline const char * name<my_robot_msgs::msg::CanFrame>()
{
  return "my_robot_msgs/msg/CanFrame";
}

template<>
struct has_fixed_size<my_robot_msgs::msg::CanFrame>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<my_robot_msgs::msg::CanFrame>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<my_robot_msgs::msg::CanFrame>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__TRAITS_HPP_
