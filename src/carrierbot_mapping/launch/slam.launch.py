import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    slam_config = LaunchConfiguration("slam_config")
    lifecycle_nodes = ["map_saver_server"]

    slam_config_arg = DeclareLaunchArgument(
        "slam_config",
        default_value=os.path.join(
            get_package_share_directory("carrierbot_mapping"), "config", "slam_toolbox.yaml"
        ),
        description="Full path to slam yaml file to load"
    )

    imu = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("ros-imu-bno055"), "launch", "imu.launch"
        ),
    )

    lidar = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("rplidar_ros"), "launch", "rplidar_s2.launch"
        ),
    )

    robot_localization = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[os.path.join(get_package_share_directory("carrierbot_mapping"), "config", "ekf.yaml")],
    )
    
    nav2_map_saver = Node(
        package="nav2_map_server",
        executable="map_saver_server",
        name="map_saver_server",
        output="screen",
        parameters=[
            {"save_map_timeout": 5.0},
            {"free_thresh_default", "0.196"},
            {"occupied_thresh_default", "0.65"},
        ],
    )

    slam_toolbox = Node(
        package="slam_toolbox",
        executable="sync_slam_toolbox_node",
        name="slam_toolbox",
        output="screen",
        parameters=[
            slam_config,
        ],
    )

    nav2_lifecycle_manager = Node(
        package="nav2_lifecycle_manager",
        executable="lifecycle_manager",
        name="lifecycle_manager_slam",
        output="screen",
        parameters=[
            {"node_names": lifecycle_nodes},
            {"autostart": True}
        ],
    )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(get_package_share_directory("carrierbot_mapping"), "rviz", "slam.rviz")],
        output="screen",
    )

    return LaunchDescription([
        slam_config_arg,
        imu,
        lidar,
        robot_localization,
        nav2_map_saver,
        slam_toolbox,
        nav2_lifecycle_manager,
        rviz,
    ])