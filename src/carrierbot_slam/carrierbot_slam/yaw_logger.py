#!/usr/bin/env python3

import csv
import datetime
import math
import os

import rclpy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node
from sensor_msgs.msg import Imu


DEFAULT_CSV_DIR = '/home/nvidia/carrierbot_ws/src/carrierbot_bringup/data_log'


def make_default_csv_path():
    timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    return os.path.join(DEFAULT_CSV_DIR, f'yaw_log_{timestamp}.csv')


def quaternion_to_yaw(x, y, z, w):
    siny_cosp = 2.0 * (w * z + x * y)
    cosy_cosp = 1.0 - 2.0 * (y * y + z * z)
    return math.atan2(siny_cosp, cosy_cosp)


def format_yaw(yaw_rad):
    if yaw_rad is None:
        return 'n/a'
    return f'{math.degrees(yaw_rad):.2f} deg'


def valid_quaternion(q):
    values = (q.x, q.y, q.z, q.w)
    return all(math.isfinite(value) for value in values) and any(abs(value) > 1e-12 for value in values)


def format_value(value):
    if value is None:
        return ''
    if isinstance(value, float) and not math.isfinite(value):
        return 'nan'
    return f'{value:.2f}'


class YawLogger(Node):
    def __init__(self):
        super().__init__('yaw_logger')

        self.declare_parameter('odom_topic', '/odom')
        self.declare_parameter('imu_topic', '/imu/data')
        self.declare_parameter('ekf_topic', '/odometry/filtered')
        self.declare_parameter('cmd_vel_topic', '/cmd_vel')
        self.declare_parameter('csv_path', make_default_csv_path())
        self.declare_parameter('publish_rate', 10.0)

        self.odom_x = None
        self.odom_y = None
        self.odom_yaw = None
        self.odom_vx = None
        self.odom_vy = None
        self.odom_wz = None
        self.imu_qx = None
        self.imu_qy = None
        self.imu_qz = None
        self.imu_qw = None
        self.imu_yaw = None
        self.ekf_x = None
        self.ekf_y = None
        self.ekf_yaw = None
        self.ekf_vx = None
        self.ekf_vy = None
        self.ekf_wz = None
        self.cmd_lin_x = None
        self.cmd_lin_y = None
        self.cmd_lin_z = None
        self.cmd_ang_x = None
        self.cmd_ang_y = None
        self.cmd_ang_z = None

        self.csv_path = os.path.expanduser(str(self.get_parameter('csv_path').value))
        csv_dir = os.path.dirname(self.csv_path)
        if csv_dir:
            os.makedirs(csv_dir, exist_ok=True)

        self.csv_file = open(self.csv_path, 'a', newline='')
        self.csv_writer = csv.writer(self.csv_file)
        if self.csv_file.tell() == 0:
            self.csv_writer.writerow([
                'time_s',
                'odom_x',
                'odom_y',
                'odom_yaw_deg',
                'odom_vx',
                'odom_vy',
                'odom_wz',
                'imu_qx',
                'imu_qy',
                'imu_qz',
                'imu_qw',
                'imu_yaw_deg',
                'ekf_x',
                'ekf_y',
                'ekf_yaw_deg',
                'ekf_vx',
                'ekf_vy',
                'ekf_wz',
                'cmd_lin_x',
                'cmd_lin_y',
                'cmd_lin_z',
                'cmd_ang_x',
                'cmd_ang_y',
                'cmd_ang_z',
            ])
            self.csv_file.flush()

        self.create_subscription(Odometry, self.get_parameter('odom_topic').value, self.odom_callback, 10)
        self.create_subscription(Imu, self.get_parameter('imu_topic').value, self.imu_callback, 10)
        self.create_subscription(Odometry, self.get_parameter('ekf_topic').value, self.ekf_callback, 10)
        self.create_subscription(Twist, self.get_parameter('cmd_vel_topic').value, self.cmd_vel_callback, 10)

        publish_rate = float(self.get_parameter('publish_rate').value)
        period = 1.0 / publish_rate if publish_rate > 0.0 else 0.2
        self.create_timer(period, self.log_yaw)

        self.get_logger().info('Yaw logger started')

    def odom_callback(self, msg):
        pose = msg.pose.pose
        self.odom_x = pose.position.x
        self.odom_y = pose.position.y
        q = msg.pose.pose.orientation
        if valid_quaternion(q):
            self.odom_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

        twist = msg.twist.twist
        self.odom_vx = twist.linear.x
        self.odom_vy = twist.linear.y
        self.odom_wz = twist.angular.z

    def imu_callback(self, msg):
        q = msg.orientation
        if valid_quaternion(q):
            self.imu_qx = q.x
            self.imu_qy = q.y
            self.imu_qz = q.z
            self.imu_qw = q.w
            self.imu_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

    def ekf_callback(self, msg):
        pose = msg.pose.pose
        self.ekf_x = pose.position.x
        self.ekf_y = pose.position.y
        q = msg.pose.pose.orientation
        if valid_quaternion(q):
            self.ekf_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

        twist = msg.twist.twist
        self.ekf_vx = twist.linear.x
        self.ekf_vy = twist.linear.y
        self.ekf_wz = twist.angular.z

    def cmd_vel_callback(self, msg):
        self.cmd_lin_x = msg.linear.x
        self.cmd_lin_y = msg.linear.y
        self.cmd_lin_z = msg.linear.z
        self.cmd_ang_x = msg.angular.x
        self.cmd_ang_y = msg.angular.y
        self.cmd_ang_z = msg.angular.z

    def log_yaw(self):
        current_time = self.get_clock().now().nanoseconds / 1e9
        self.get_logger().info(
            f'odom yaw: {format_yaw(self.odom_yaw)} | '
            f'imu q: ({self.imu_qx:.2f}, {self.imu_qy:.2f}, {self.imu_qz:.2f}, {self.imu_qw:.2f}) | '
            f'imu yaw: {format_yaw(self.imu_yaw)} | '
            f'ekf yaw: {format_yaw(self.ekf_yaw)}'
            if self.imu_qx is not None and self.imu_qy is not None and self.imu_qz is not None and self.imu_qw is not None
            else f'odom yaw: {format_yaw(self.odom_yaw)} | imu q: (n/a) | imu yaw: {format_yaw(self.imu_yaw)} | ekf yaw: {format_yaw(self.ekf_yaw)}'
        )

        self.csv_writer.writerow([
            f'{current_time:.2f}',
            format_value(self.odom_x),
            format_value(self.odom_y),
            f'{math.degrees(self.odom_yaw):.2f}' if self.odom_yaw is not None else '',
            format_value(self.odom_vx),
            format_value(self.odom_vy),
            format_value(self.odom_wz),
            f'{self.imu_qx:.2f}' if self.imu_qx is not None else '',
            f'{self.imu_qy:.2f}' if self.imu_qy is not None else '',
            f'{self.imu_qz:.2f}' if self.imu_qz is not None else '',
            f'{self.imu_qw:.2f}' if self.imu_qw is not None else '',
            f'{math.degrees(self.imu_yaw):.2f}' if self.imu_yaw is not None else '',
            format_value(self.ekf_x),
            format_value(self.ekf_y),
            f'{math.degrees(self.ekf_yaw):.2f}' if self.ekf_yaw is not None else '',
            format_value(self.ekf_vx),
            format_value(self.ekf_vy),
            format_value(self.ekf_wz),
            format_value(self.cmd_lin_x),
            format_value(self.cmd_lin_y),
            format_value(self.cmd_lin_z),
            format_value(self.cmd_ang_x),
            format_value(self.cmd_ang_y),
            format_value(self.cmd_ang_z),
        ])
        self.csv_file.flush()


def main(args=None):
    rclpy.init(args=args)
    node = YawLogger()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        if getattr(node, 'csv_file', None) is not None:
            node.csv_file.close()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()