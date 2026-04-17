import os
import subprocess
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory, get_package_share_path
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.substitutions import LaunchConfiguration, Command
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():

    map_file = os.path.join(
        get_package_share_directory('carrierbot_bringup'),
        'maps',
        'map_b1.yaml'
    )

    use_sim_time = LaunchConfiguration("use_sim_time")

    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false"
    )

    slam_config_arg = DeclareLaunchArgument(
        "slam_config",
        default_value=os.path.join(
            get_package_share_directory("carrierbot_slam"), "config", "mapper_params_localization_mode.yaml"
        ),
        description="Full path to slam yaml file to load"
    )

    slam_config = LaunchConfiguration("slam_config")

    slam_toolbox = TimerAction(
        period=5.0,
        actions=[
            Node(
                package="slam_toolbox",
                # executable="sync_slam_toolbox_node", # Mapping
                executable="localization_slam_toolbox_node", # Localization
                name="slam_toolbox",
                output="screen",
                parameters=[
                    slam_config, 
                    {
                        "map_file_name": map_file,
                        "use_sim_time": use_sim_time
                    }
                ],
                remappings=[
                    ("scan", "/scan_filtered"),
                ]
            )
        ]
    )

    rviz = TimerAction(
        period=7.0,
        actions=[
            Node(
                package="rviz2",
                executable="rviz2",
                name="rviz2",
                arguments=[
                    "-d",
                    os.path.join(
                        get_package_share_directory("carrierbot_slam"),
                        "rviz",
                        "slam_toolbox.rviz"
                    )
                ],
                output="screen",
            )
        ]
    )

    return LaunchDescription([
        use_sim_time_arg,
        slam_config_arg,
        slam_toolbox,
        rviz,
    ])