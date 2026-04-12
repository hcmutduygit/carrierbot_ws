import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory, get_package_share_path
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, Command, PathJoinSubstitution
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    urdf_path = os.path.join(get_package_share_path('carrierbot_description'), 
                             'urdf', 'my_robot.urdf.xacro')
    robot_description = ParameterValue(Command(['xacro ', urdf_path]), value_type=str)

    controller_config_arg = DeclareLaunchArgument(
        "controller_config",
        default_value=os.path.join(
            get_package_share_directory("carrierbot_controller"), "config", "controller.yaml"
        ),
        description="Full path to controller yaml file to load"
    )

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

    controller_config = LaunchConfiguration("controller_config")
    amcl_config = LaunchConfiguration("amcl_config")
    lifecycle_nodes = ["map_server", "amcl"]

    imu = Node(
        package="ros_imu_bno055",
        executable="bno055_i2c_node",
        name="imu_node",
        output="screen",
        respawn="true",
        respawn_delay=2,
        parameters=[
            {"device": "/dev/i2c-1"},
            {"address": 40},
            {"frame_id": "imu"}
        ]
    )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("rplidar_ros"),
                "launch",
                "rplidar_s2_launch.py"
            )
        )
    )

    robot_localization = Node(
        package="robot_localization",
        executable="ekf_node",
        name="ekf_filter_node",
        output="screen",
        parameters=[os.path.join(get_package_share_directory("carrierbot_mapping"), "config", "ekf.yaml")],
    )

    controller = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            {"robot_description": robot_description},
            controller_config
        ],
        output="screen"
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner.py",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
        ],
    )

    carrierbot_controller_spawner = Node(
        package="controller_manager",
        executable="spawner.py",
        arguments=["carrierbot_controller", 
                   "--controller-manager", 
                   "/controller_manager"
        ],
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}],
        output="screen"
    )
    
    nav2_map_server = Node(
        package="nav2_map_server",
        executable="map_server",
        name="map_server",
        output="screen",
        parameters=[
            {"yaml_filename": map_path},
            {"use_sim_time": False}
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
            {"use_sim_time": False}
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
        arguments=["-d", os.path.join(get_package_share_directory("carrierbot_mapping"), "rviz", "localization.rviz")],
        output="screen",
    )

    return LaunchDescription([
        controller_config_arg,
        amcl_config_arg,
        imu,
        lidar,
        robot_localization,
        controller,
        joint_state_broadcaster_spawner,
        carrierbot_controller_spawner,
        robot_state_publisher,
        nav2_map_server,
        nav2_amcl,
        nav2_lifecycle_manager,
        rviz,
    ])