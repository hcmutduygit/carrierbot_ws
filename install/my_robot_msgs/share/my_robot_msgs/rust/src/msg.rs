#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to my_robot_msgs__msg__CanFrame
/// CAN Frame Message
/// Định nghĩa cấu trúc message để gửi dữ liệu CAN

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct CanFrame {
    /// CAN ID (ví dụ: 0x123)
    pub can_id: u32,

    /// Dữ liệu CAN (tối đa 8 bytes)
    pub data: Vec<u8>,

}



impl Default for CanFrame {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::CanFrame::default())
  }
}

impl rosidl_runtime_rs::Message for CanFrame {
  type RmwMsg = super::msg::rmw::CanFrame;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        can_id: msg.can_id,
        data: msg.data.into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
      can_id: msg.can_id,
        data: msg.data.as_slice().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      can_id: msg.can_id,
      data: msg.data
          .into_iter()
          .collect(),
    }
  }
}


