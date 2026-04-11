import os
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory, get_package_share_path
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.substitutions import LaunchConfiguration, Command
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

    slam_config_arg = DeclareLaunchArgument(
        "slam_config",
        default_value=os.path.join(
            get_package_share_directory("carrierbot_mapping"), "config", "slam_toolbox.yaml"
        ),
        description="Full path to slam yaml file to load"
    )

    controller_config = LaunchConfiguration("controller_config")
    slam_config = LaunchConfiguration("slam_config")

    # imu = IncludeLaunchDescription(
    #     os.path.join(
    #         get_package_share_directory("ros-imu-bno055"), "launch", "imu.launch"
    #     ),
    # )

    lidar = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("rplidar_ros"),
                "launch",
                "rplidar_s2_launch.py"
            )
        )
    )

    # robot_localization = Node(
    #     package="robot_localization",
    #     executable="ekf_node",
    #     name="ekf_filter_node",
    #     output="screen",
    #     parameters=[os.path.join(get_package_share_directory("carrierbot_mapping"), "config", "ekf.yaml")],
    # )


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
    
    nav2_map_saver = Node(
        package="nav2_map_server",
        executable="map_saver_server",
        name="map_saver_server",
        output="screen",
        parameters=[
            {"save_map_timeout": 5.0},
            {"free_thresh_default": 0.196},
            {"occupied_thresh_default": 0.65},
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

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(get_package_share_directory("carrierbot_mapping"), "rviz", "slam.rviz")],
        output="screen",
    )

    return LaunchDescription([
        slam_config_arg,
        controller_config_arg,
        # imu,
        lidar,
        # robot_localization,
        controller,
        joint_state_broadcaster_spawner,
        carrierbot_controller_spawner,
        robot_state_publisher,
        nav2_map_saver,
        slam_toolbox,
        rviz,
    ])