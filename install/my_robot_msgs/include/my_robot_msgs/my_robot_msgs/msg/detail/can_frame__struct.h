// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from my_robot_msgs:msg/CanFrame.idl
// generated code does not contain a copyright notice

#ifndef MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_H_
#define MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'data'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/CanFrame in the package my_robot_msgs.
/**
  * CAN Frame Message
  * Định nghĩa cấu trúc message để gửi dữ liệu CAN
 */
typedef struct my_robot_msgs__msg__CanFrame
{
  /// CAN ID (ví dụ: 0x123)
  uint32_t can_id;
  /// Dữ liệu CAN (tối đa 8 bytes)
  rosidl_runtime_c__uint8__Sequence data;
} my_robot_msgs__msg__CanFrame;

// Struct for a sequence of my_robot_msgs__msg__CanFrame.
typedef struct my_robot_msgs__msg__CanFrame__Sequence
{
  my_robot_msgs__msg__CanFrame * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} my_robot_msgs__msg__CanFrame__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // MY_ROBOT_MSGS__MSG__DETAIL__CAN_FRAME__STRUCT_H_
