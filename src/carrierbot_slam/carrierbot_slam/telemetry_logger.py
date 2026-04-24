#!/usr/bin/env python3

import csv
import datetime
import math
import os
from typing import Optional

import rclpy
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node
from sensor_msgs.msg import Imu
from sensor_msgs.msg import JointState
from sensor_msgs.msg import LaserScan


DEFAULT_CSV_DIR = '/home/nvidia/carrierbot_ws/src/carrierbot_bringup/data_log'


def make_default_csv_path():
    timestamp = datetime.datetime.now().strftime('%Y%m%d_%H%M%S')
    return os.path.join(DEFAULT_CSV_DIR, f'telemetry_log_{timestamp}.csv')


def quaternion_to_yaw(x, y, z, w):
    siny_cosp = 2.0 * (w * z + x * y)
    cosy_cosp = 1.0 - 2.0 * (y * y + z * z)
    return math.atan2(siny_cosp, cosy_cosp)


def quaternion_to_rpy(x, y, z, w):
    sinr_cosp = 2.0 * (w * x + y * z)
    cosr_cosp = 1.0 - 2.0 * (x * x + y * y)
    roll = math.atan2(sinr_cosp, cosr_cosp)

    sinp = 2.0 * (w * y - z * x)
    if abs(sinp) >= 1:
        pitch = math.copysign(math.pi / 2, sinp)
    else:
        pitch = math.asin(sinp)

    yaw = quaternion_to_yaw(x, y, z, w)
    return roll, pitch, yaw


def valid_quaternion(q):
    values = (q.x, q.y, q.z, q.w)
    return all(math.isfinite(value) for value in values) and any(abs(value) > 1e-12 for value in values)


def format_value(value):
    if value is None:
        return ''
    if isinstance(value, float) and not math.isfinite(value):
        return 'nan'
    return f'{value:.2f}'


def format_list(values):
    if not values:
        return ''
    return '|'.join(format_value(value) for value in values)


class TelemetryLogger(Node):
    def __init__(self):
        super().__init__('telemetry_logger')

        self.declare_parameter('odom_topic', '/odom')
        self.declare_parameter('filtered_odom_topic', '/odometry/filtered')
        self.declare_parameter('imu_topic', '/imu/data')
        self.declare_parameter('cmd_vel_topic', '/cmd_vel')
        self.declare_parameter('joint_state_topic', '/joint_states')
        self.declare_parameter('laser_topic', '/scan_filtered')
        self.declare_parameter('csv_path', make_default_csv_path())
        self.declare_parameter('publish_rate', 10.0)
        self.declare_parameter('left_wheel_joint', 'base_left_wheel_joint')
        self.declare_parameter('right_wheel_joint', 'base_right_wheel_joint')

        self.odom_x: Optional[float] = None
        self.odom_y: Optional[float] = None
        self.odom_yaw: Optional[float] = None
        self.odom_vx: Optional[float] = None
        self.odom_vy: Optional[float] = None
        self.odom_wz: Optional[float] = None

        self.filtered_x: Optional[float] = None
        self.filtered_y: Optional[float] = None
        self.filtered_yaw: Optional[float] = None
        self.filtered_vx: Optional[float] = None
        self.filtered_vy: Optional[float] = None
        self.filtered_wz: Optional[float] = None

        self.imu_qx: Optional[float] = None
        self.imu_qy: Optional[float] = None
        self.imu_qz: Optional[float] = None
        self.imu_qw: Optional[float] = None
        self.imu_roll: Optional[float] = None
        self.imu_pitch: Optional[float] = None
        self.imu_yaw: Optional[float] = None

        self.cmd_lin_x: Optional[float] = None
        self.cmd_lin_y: Optional[float] = None
        self.cmd_lin_z: Optional[float] = None
        self.cmd_ang_x: Optional[float] = None
        self.cmd_ang_y: Optional[float] = None
        self.cmd_ang_z: Optional[float] = None

        self.left_wheel_position: Optional[float] = None
        self.right_wheel_position: Optional[float] = None
        self.left_wheel_velocity: Optional[float] = None
        self.right_wheel_velocity: Optional[float] = None

        self.scan_stamp_s: Optional[float] = None
        self.scan_frame: Optional[str] = None
        self.scan_angle_min_deg: Optional[float] = None
        self.scan_angle_max_deg: Optional[float] = None
        self.scan_angle_increment_deg: Optional[float] = None
        self.scan_range_min: Optional[float] = None
        self.scan_range_max: Optional[float] = None
        self.scan_time: Optional[float] = None
        self.scan_time_increment: Optional[float] = None
        self.scan_ranges = []
        self.scan_intensities = []

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
                'filtered_x',
                'filtered_y',
                'filtered_yaw_deg',
                'filtered_vx',
                'filtered_vy',
                'filtered_wz',
                'imu_qx',
                'imu_qy',
                'imu_qz',
                'imu_qw',
                'imu_roll_deg',
                'imu_pitch_deg',
                'imu_yaw_deg',
                'cmd_lin_x',
                'cmd_lin_y',
                'cmd_lin_z',
                'cmd_ang_x',
                'cmd_ang_y',
                'cmd_ang_z',
                'left_wheel_position',
                'right_wheel_position',
                'left_wheel_velocity',
                'right_wheel_velocity',
                'scan_stamp_s',
                'scan_frame',
                'scan_angle_min_deg',
                'scan_angle_max_deg',
                'scan_angle_increment_deg',
                'scan_range_min',
                'scan_range_max',
                'scan_time',
                'scan_time_increment',
                'scan_ranges',
                'scan_intensities',
            ])
            self.csv_file.flush()

        self.create_subscription(Odometry, self.get_parameter('odom_topic').value, self.odom_callback, 10)
        self.create_subscription(Odometry, self.get_parameter('filtered_odom_topic').value, self.filtered_odom_callback, 10)
        self.create_subscription(Imu, self.get_parameter('imu_topic').value, self.imu_callback, 10)
        self.create_subscription(Twist, self.get_parameter('cmd_vel_topic').value, self.cmd_vel_callback, 10)
        self.create_subscription(JointState, self.get_parameter('joint_state_topic').value, self.joint_state_callback, 10)
        self.create_subscription(LaserScan, self.get_parameter('laser_topic').value, self.laser_callback, 10)

        publish_rate = float(self.get_parameter('publish_rate').value)
        period = 1.0 / publish_rate if publish_rate > 0.0 else 0.2
        self.create_timer(period, self.log_csv)

        self.get_logger().info(f'Telemetry logger started: {self.csv_path}')

    def odom_callback(self, msg):
        pose = msg.pose.pose
        self.odom_x = pose.position.x
        self.odom_y = pose.position.y
        q = pose.orientation
        if valid_quaternion(q):
            self.odom_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

        twist = msg.twist.twist
        self.odom_vx = twist.linear.x
        self.odom_vy = twist.linear.y
        self.odom_wz = twist.angular.z

    def filtered_odom_callback(self, msg):
        pose = msg.pose.pose
        self.filtered_x = pose.position.x
        self.filtered_y = pose.position.y
        q = pose.orientation
        if valid_quaternion(q):
            self.filtered_yaw = quaternion_to_yaw(q.x, q.y, q.z, q.w)

        twist = msg.twist.twist
        self.filtered_vx = twist.linear.x
        self.filtered_vy = twist.linear.y
        self.filtered_wz = twist.angular.z

    def imu_callback(self, msg):
        q = msg.orientation
        if valid_quaternion(q):
            self.imu_qx = q.x
            self.imu_qy = q.y
            self.imu_qz = q.z
            self.imu_qw = q.w
            self.imu_roll, self.imu_pitch, self.imu_yaw = quaternion_to_rpy(q.x, q.y, q.z, q.w)

    def cmd_vel_callback(self, msg):
        self.cmd_lin_x = msg.linear.x
        self.cmd_lin_y = msg.linear.y
        self.cmd_lin_z = msg.linear.z
        self.cmd_ang_x = msg.angular.x
        self.cmd_ang_y = msg.angular.y
        self.cmd_ang_z = msg.angular.z

    def joint_state_callback(self, msg):
        left_joint = self.get_parameter('left_wheel_joint').value
        right_joint = self.get_parameter('right_wheel_joint').value
        joint_map = {name: index for index, name in enumerate(msg.name)}
        left_index = joint_map.get(left_joint)
        right_index = joint_map.get(right_joint)

        def get_value(values, index):
            if index is None or index >= len(values):
                return None
            return values[index]

        self.left_wheel_position = get_value(msg.position, left_index)
        self.right_wheel_position = get_value(msg.position, right_index)
        self.left_wheel_velocity = get_value(msg.velocity, left_index)
        self.right_wheel_velocity = get_value(msg.velocity, right_index)

    def laser_callback(self, msg):
        self.scan_stamp_s = msg.header.stamp.sec + msg.header.stamp.nanosec / 1e9
        self.scan_frame = msg.header.frame_id
        self.scan_angle_min_deg = math.degrees(msg.angle_min)
        self.scan_angle_max_deg = math.degrees(msg.angle_max)
        self.scan_angle_increment_deg = math.degrees(msg.angle_increment)
        self.scan_range_min = msg.range_min
        self.scan_range_max = msg.range_max
        self.scan_time = msg.scan_time
        self.scan_time_increment = msg.time_increment
        self.scan_ranges = list(msg.ranges)
        self.scan_intensities = list(msg.intensities)

    def log_csv(self):
        current_time = self.get_clock().now().nanoseconds / 1e9
        self.csv_writer.writerow([
            f'{current_time:.2f}',
            format_value(self.odom_x),
            format_value(self.odom_y),
            f'{math.degrees(self.odom_yaw):.2f}' if self.odom_yaw is not None else '',
            format_value(self.odom_vx),
            format_value(self.odom_vy),
            format_value(self.odom_wz),
            format_value(self.filtered_x),
            format_value(self.filtered_y),
            f'{math.degrees(self.filtered_yaw):.2f}' if self.filtered_yaw is not None else '',
            format_value(self.filtered_vx),
            format_value(self.filtered_vy),
            format_value(self.filtered_wz),
            format_value(self.imu_qx),
            format_value(self.imu_qy),
            format_value(self.imu_qz),
            format_value(self.imu_qw),
            f'{math.degrees(self.imu_roll):.2f}' if self.imu_roll is not None else '',
            f'{math.degrees(self.imu_pitch):.2f}' if self.imu_pitch is not None else '',
            f'{math.degrees(self.imu_yaw):.2f}' if self.imu_yaw is not None else '',
            format_value(self.cmd_lin_x),
            format_value(self.cmd_lin_y),
            format_value(self.cmd_lin_z),
            format_value(self.cmd_ang_x),
            format_value(self.cmd_ang_y),
            format_value(self.cmd_ang_z),
            format_value(self.left_wheel_position),
            format_value(self.right_wheel_position),
            format_value(self.left_wheel_velocity),
            format_value(self.right_wheel_velocity),
            format_value(self.scan_stamp_s),
            self.scan_frame or '',
            format_value(self.scan_angle_min_deg),
            format_value(self.scan_angle_max_deg),
            format_value(self.scan_angle_increment_deg),
            format_value(self.scan_range_min),
            format_value(self.scan_range_max),
            format_value(self.scan_time),
            format_value(self.scan_time_increment),
            format_list(self.scan_ranges),
            format_list(self.scan_intensities),
        ])
        self.csv_file.flush()


def main(args=None):
    rclpy.init(args=args)
    node = TelemetryLogger()
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