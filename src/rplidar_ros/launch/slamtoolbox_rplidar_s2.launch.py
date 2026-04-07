#!/usr/bin/env python3

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # Launch configuration variables
    use_sim_time = LaunchConfiguration('use_sim_time')
    
    # Get the package directories
    rplidar_ros_prefix = get_package_share_directory('rplidar_ros')
    
    # SLAM Toolbox configuration file
    slam_params_file = os.path.join(
        rplidar_ros_prefix,
        'config',
        'mapper_params_online_async.yaml'
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            'use_sim_time',
            default_value='false',
            description='Use simulation (Gazebo) clock if true'
        ),

        # SLAM Toolbox node - Online Async
        Node(
            package='slam_toolbox',
            executable='async_slam_toolbox_node',
            name='slam_toolbox',
            output='screen',
            parameters=[
                slam_params_file,
                {'use_sim_time': use_sim_time}
            ]
        ),

        # RPLidar node
        Node(
            package='rplidar_ros',
            executable='rplidar_node',
            name='rplidar_node',
            parameters=[{
                'channel_type': 'serial',
                'serial_port': '/dev/ttyUSB0',
                'serial_baudrate': 1000000,
                'frame_id': 'laser',
                'inverted': False,
                'angle_compensate': True,
                'scan_mode': 'DenseBoost'
            }],
            output='screen'
        ),

        # Static transform: base_link -> laser (identity transform since we only have lidar)
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='base_to_laser_broadcaster',
            arguments=['0', '0', '0', '0', '0', '0', 'base_link', 'laser']
        ),

        # Static transform: odom -> base_link (will be updated by SLAM)
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            name='odom_to_base_broadcaster',
            arguments=['0', '0', '0', '0', '0', '0', 'odom', 'base_link']
        ),
    ])
