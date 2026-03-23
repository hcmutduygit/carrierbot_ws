#include "rclcpp/rclcpp.hpp"
#include "my_robot_base/can_node.hpp"
#include <thread>
#include <chrono>
#include <atomic>

WaveshareCAN can("/dev/usbcan", 2000000, 2.0);
std::atomic<bool> should_stop(false);

void send_vel(WaveshareCAN &can) //0x030
{
    try
    {
        // Get integer velocities
        // int right_vel = right_wheel_velocity;
        // int left_vel = left_wheel_velocity;
        int right_vel = 3000;
        int left_vel = -3000;
     
        // Create 8-byte data array: first 4 bytes for left wheel, last 4 bytes for right wheel
        uint8_t data[8];

        // Convert left velocity to bytes (first 4 bytes)
        std::memcpy(data, &left_vel, sizeof(int));

        // Convert right velocity to bytes (last 4 bytes)
        std::memcpy(data + 4, &right_vel, sizeof(int));

        // Create data vector
        std::vector<uint8_t> velocity_data(data, data + 8);

        // Send velocities to CAN ID 0x011
        can.send(0x011, velocity_data);
        // cnt_send++;
        // std::cout << "Sent left velocity " << left_vel << " and right velocity " << right_vel << " to ID 0x013" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error in send_vel: " << e.what() << std::endl;
    }
}

// Convert two bytes to a signed 16-bit integer
int16_t hex_to_signed(const std::vector<uint8_t> &data, size_t start_idx, size_t bits = 16)
{
    uint16_t value = (data[start_idx] << 8) | data[start_idx + 1];
    // Convert unsigned to signed using proper casting
    int16_t signed_value = static_cast<int16_t>(value);
    return signed_value;
}

// Convert two bytes to an unsigned 16-bit integer for angles
uint16_t hex_to_unsigned(const std::vector<uint8_t> &data, size_t start_idx)
{
    // Combine two bytes into a 16-bit unsigned integer (big-endian)
    return static_cast<uint16_t>((data[start_idx] << 8) | data[start_idx + 1]);
}


int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("can_node");
  auto logger = node->get_logger();

  // Đăng ký hàm callback khi shutdown
  auto shutdown_callback = [&]() {
    RCLCPP_INFO(logger, "Shutdown requested. Stopping loops...");
    should_stop = true;
  };
  rclcpp::on_shutdown(shutdown_callback);

  try {
    // Mở CAN interface
    RCLCPP_INFO(logger, "Opening CAN interface...");
    can.open();
    
    // Bắt đầu nhận CAN frames trong thread riêng
    RCLCPP_INFO(logger, "Starting CAN receive loop...");
    can.start_receive_loop([logger](uint16_t can_id, const std::vector<uint8_t>& data) {
        if (should_stop) {
            return;
        }
        RCLCPP_INFO(logger, "Received CAN ID: 0x%X, Data Length: %zu", can_id, data.size());
    });
    
    // Gửi vận tốc liên tục với tần số 1Hz (cho đến khi dừng)
    RCLCPP_INFO(logger, "Sending velocity commands continuously at 1Hz (Press Ctrl+C to stop)...");
    int count = 0;
    rclcpp::WallRate loop_rate(1.0); // 1 Hz
    
    while (rclcpp::ok() && !should_stop) {
        // Gửi vận tốc
        std::cout << "\n📍 Iteration " << (++count) << std::endl;
        send_vel(can);
        
        rclcpp::spin_some(node);
        loop_rate.sleep();
    }
    
    RCLCPP_INFO(logger, "Shutting down CAN Node...");
    
    try {
      // can.close() sẽ xử lý việc join thread và đóng kết nối
      can.close();
    }
    catch (const std::exception &e) {
      RCLCPP_ERROR(logger, "Error closing CAN: %s", e.what());
    }
    
    RCLCPP_INFO(logger, "CAN Node stopped successfully!");
  }
  catch (const std::exception &e) {
    RCLCPP_ERROR(logger, "Error: %s", e.what());
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}