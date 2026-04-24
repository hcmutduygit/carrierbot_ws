from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        Node(
            package='carrierbot_slam',
            executable='telemetry_logger.py',
            name='telemetry_logger',
            output='screen',
            parameters=[{
                'odom_topic': '/odom',
                'filtered_odom_topic': '/odometry/filtered',
                'imu_topic': '/imu/data',
                'cmd_vel_topic': '/cmd_vel',
                'joint_state_topic': '/joint_states',
                'laser_topic': '/scan_filtered',
                'left_wheel_joint': 'base_left_wheel_joint',
                'right_wheel_joint': 'base_right_wheel_joint',
                'publish_rate': 10.0,
            }]
        )
    ])