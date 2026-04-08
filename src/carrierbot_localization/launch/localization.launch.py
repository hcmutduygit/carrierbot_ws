import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


def generate_launch_description():

    amcl_config = LaunchConfiguration("amcl_config")
    lifecycle_nodes = ["map_server", "amcl"]

    amcl_config_arg = DeclareLaunchArgument(
        "amcl_config",
        default_value=os.path.join(
            get_package_share_directory("carrierbot_localization"), "config", "amcl.yaml"
        ),
        description="Full path to amcl yaml file to load"
    )

    map_path = PathJoinSubstitution([
        get_package_share_directory("carrierbot_mapping"), "maps", "map.yaml"
    ])

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
        parameters=[os.path.join(get_package_share_directory("carrierbot_localization"), "config", "ekf.yaml")],
    )
    
    nav2_map_server = Node(
        package="nav2_map_server",
        executable="map_server",
        name="map_server",
        output="screen",
        parameters=[
            {"yaml_filename": map_path},
        ],
    )

    nav2_amcl = Node(
        package="nav2_amcl",
        executable="amcl",
        name="amcl",
        output="screen",
        emulate_tty=True,
        parameters=[
            amcl_config,
        ],
    )

    nav2_lifecycle_manager = Node(
        package="nav2_lifecycle_manager",
        executable="lifecycle_manager",
        name="lifecycle_manager_localization",
        output="screen",
        parameters=[
            {"node_names": lifecycle_nodes},
            {"autostart": True}
        ],
    )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(get_package_share_directory("carrierbot_localization"), "rviz", "localization.rviz")],
        output="screen",
    )

    return LaunchDescription([
        amcl_config_arg,
        imu,
        lidar,
        robot_localization,
        nav2_map_server,
        nav2_amcl,
        nav2_lifecycle_manager,
        rviz,
    ])