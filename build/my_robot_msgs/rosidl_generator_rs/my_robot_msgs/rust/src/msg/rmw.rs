#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "my_robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__my_robot_msgs__msg__CanFrame() -> *const std::ffi::c_void;
}

#[link(name = "my_robot_msgs__rosidl_generator_c")]
extern "C" {
    fn my_robot_msgs__msg__CanFrame__init(msg: *mut CanFrame) -> bool;
    fn my_robot_msgs__msg__CanFrame__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<CanFrame>, size: usize) -> bool;
    fn my_robot_msgs__msg__CanFrame__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<CanFrame>);
    fn my_robot_msgs__msg__CanFrame__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<CanFrame>, out_seq: *mut rosidl_runtime_rs::Sequence<CanFrame>) -> bool;
}

// Corresponds to my_robot_msgs__msg__CanFrame
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// CAN Frame Message
/// Định nghĩa cấu trúc message để gửi dữ liệu CAN

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CanFrame {
    /// CAN ID (ví dụ: 0x123)
    pub can_id: u32,

    /// Dữ liệu CAN (tối đa 8 bytes)
    pub data: rosidl_runtime_rs::Sequence<u8>,

}



impl Default for CanFrame {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !my_robot_msgs__msg__CanFrame__init(&mut msg as *mut _) {
        panic!("Call to my_robot_msgs__msg__CanFrame__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for CanFrame {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_robot_msgs__msg__CanFrame__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_robot_msgs__msg__CanFrame__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { my_robot_msgs__msg__CanFrame__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for CanFrame {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for CanFrame where Self: Sized {
  const TYPE_NAME: &'static str = "my_robot_msgs/msg/CanFrame";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__my_robot_msgs__msg__CanFrame() }
  }
}


