// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from my_robot_msgs:msg/CanFrame.idl
// generated code does not contain a copyright notice

#ifndef MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_HPP_
#define MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__my_robot_msgs__msg__CanFrame __attribute__((deprecated))
#else
# define DEPRECATED__my_robot_msgs__msg__CanFrame __declspec(deprecated)
#endif

namespace my_robot_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct CanFrame_
{
  using Type = CanFrame_<ContainerAllocator>;

  explicit CanFrame_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->can_id = 0ul;
    }
  }

  explicit CanFrame_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->can_id = 0ul;
    }
  }

  // field types and members
  using _can_id_type =
    uint32_t;
  _can_id_type can_id;
  using _data_type =
    std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>>;
  _data_type data;

  // setters for named parameter idiom
  Type & set__can_id(
    const uint32_t & _arg)
  {
    this->can_id = _arg;
    return *this;
  }
  Type & set__data(
    const std::vector<uint8_t, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<uint8_t>> & _arg)
  {
    this->data = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    my_robot_msgs::msg::CanFrame_<ContainerAllocator> *;
  using ConstRawPtr =
    const my_robot_msgs::msg::CanFrame_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      my_robot_msgs::msg::CanFrame_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      my_robot_msgs::msg::CanFrame_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__my_robot_msgs__msg__CanFrame
    std::shared_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__my_robot_msgs__msg__CanFrame
    std::shared_ptr<my_robot_msgs::msg::CanFrame_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const CanFrame_ & other) const
  {
    if (this->can_id != other.can_id) {
      return false;
    }
    if (this->data != other.data) {
      return false;
    }
    return true;
  }
  bool operator!=(const CanFrame_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct CanFrame_

// alias to use template instance with default allocator
using CanFrame =
  my_robot_msgs::msg::CanFrame_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace my_robot_msgs

#endif  // MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_HPP_
