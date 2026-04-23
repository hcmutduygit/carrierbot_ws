#!/usr/bin/env python3

import math

import rclpy
from nav_msgs.msg import Odometry
from rclpy.node import Node
from sensor_msgs.msg import Imu


def quaternion_to_yaw(x, y, z, w):
    siny_cosp = 2.0 * (w * z + x * y)
    cosy_cosp = 1.0 - 2.0 * (y * y + z * z)
    return math.atan2(siny_cosp, cosy_cosp)


def format_yaw(yaw_rad):
    if yaw_rad is None:
        return 'n/a'
    return f'{math.degrees(yaw_rad):.2f} deg ({yaw_rad:.3f} rad)'


def valid_quaternion(q):
    values = (q.x, q.y, q.z, q.w)
    return all(math.isfinite(value) for value in values) and any(abs(value) > 1e-12 for value in values)


class YawLogger(Node):
    def __init__(self):
        super().__init__('yaw_logger')

        self.declare_parameter('odom_topic', '/odom')
        self.declare_parameter('imu_topic', '/imu/data')
        self.declare_parameter('ekf_topic', '/odometry/filtered')
        self.declare_parameter('publish_rate', 10.0)

        self.odom_yaw = None
        self.imu_yaw = None
        self.ekf_yaw = None

        self.create_subscription(Odometry, self.get_parameter('odom_topic').value, self.odom_callback, 10)
        self.create_subscription(Imu, self.get_parameter('imu_topic').value, self.imu_callback, 10)
        self.create_subscription(Odometry, self.get_parameter('ekf_topic').value, self.ekf_callback, 10)

        publish_rate = float(self.get_parameter('publish_rate').value)
        period = 1.0 / publish_rate if publish_rate > 0.0 else 0.2
        self.create_timer(period, self.log_yaw)

        self.get_logger().info('Yaw logger started')

    def odom_callback(self, msg):
        q = msg.pose.pose.orientation
        if valid_quaternion(q):
            self.odom_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

    def imu_callback(self, msg):
        q = msg.orientation
        if valid_quaternion(q):
            self.imu_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

    def ekf_callback(self, msg):
        q = msg.pose.pose.orientation
        if valid_quaternion(q):
            self.ekf_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

    def log_yaw(self):
        self.get_logger().info(
            f'odom yaw: {format_yaw(self.odom_yaw)} | '
            f'imu yaw: {format_yaw(self.imu_yaw)} | '
            f'ekf yaw: {format_yaw(self.ekf_yaw)}'
        )


def main(args=None):
    rclpy.init(args=args)
    node = YawLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()