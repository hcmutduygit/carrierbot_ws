import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    bringup_pkg = get_package_share_directory("carrierbot_bringup")
    slam_pkg = get_package_share_directory("carrierbot_slam")

    use_sim_time = LaunchConfiguration("use_sim_time")
    map_yaml = LaunchConfiguration("map")
    amcl_config = LaunchConfiguration("amcl_config")
    gz_args = LaunchConfiguration("gz_args")

    use_sim_time_arg = DeclareLaunchArgument("use_sim_time", default_value="true")
    map_arg = DeclareLaunchArgument(
        "map",
        default_value=os.path.join(bringup_pkg, "maps", "lab_map.yaml"),
    )
    amcl_config_arg = DeclareLaunchArgument(
        "amcl_config",
        default_value=os.path.join(slam_pkg, "config", "amcl.yaml"),
    )
    gz_args_arg = DeclareLaunchArgument(
        "gz_args",
        default_value="empty.sdf -r",
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(bringup_pkg, "launch", "my_robot_gazebo.launch.py")
        ),
        launch_arguments={"gz_args": gz_args}.items(),
    )

    localization = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(slam_pkg, "launch", "slam.launch.py")
        ),
        launch_arguments={
            "use_sim_time": use_sim_time,
            "map": map_yaml,
            "amcl_config": amcl_config,
        }.items(),
    )

    navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("carrierbot_navigation"),
                "launch",
                "navigation.launch.py",
            )
        ),
        launch_arguments={"use_sim_time": use_sim_time}.items(),
    )

    delayed_localization = TimerAction(period=4.0, actions=[localization])
    delayed_navigation = TimerAction(period=6.0, actions=[navigation])

    return LaunchDescription(
        [
            use_sim_time_arg,
            map_arg,
            amcl_config_arg,
            gz_args_arg,
            gazebo,
            delayed_localization,
            delayed_navigation,
        ]
    )
