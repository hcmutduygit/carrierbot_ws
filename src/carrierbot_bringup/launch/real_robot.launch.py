import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import IncludeLaunchDescription, TimerAction, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    use_sim_time = LaunchConfiguration("use_sim_time")
    channel_type =  LaunchConfiguration('channel_type', default='serial')
    serial_port = LaunchConfiguration('serial_port', default='/dev/rplidar')
    serial_baudrate = LaunchConfiguration('serial_baudrate', default='1000000') #for s2 is 1000000
    frame_id = LaunchConfiguration('frame_id', default='laser')
    inverted = LaunchConfiguration('inverted', default='false')
    angle_compensate = LaunchConfiguration('angle_compensate', default='true')
    scan_mode = LaunchConfiguration('scan_mode', default='DenseBoost')

    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="false"
    )

    channel_type_arg = DeclareLaunchArgument(
            'channel_type',
            default_value=channel_type,
            description='Specifying channel type of lidar')

    serial_port_arg = DeclareLaunchArgument(
            'serial_port',
            default_value=serial_port,
            description='Specifying usb port to connected lidar')

    serial_baudrate_arg = DeclareLaunchArgument(
            'serial_baudrate',
            default_value=serial_baudrate,
            description='Specifying usb port baudrate to connected lidar')

    frame_id_arg = DeclareLaunchArgument(
            'frame_id',
            default_value=frame_id,
            description='Specifying frame_id of lidar')

    inverted_arg = DeclareLaunchArgument(
            'inverted',
            default_value=inverted,
            description='Specifying whether or not to invert scan data')

    angle_compensate_arg = DeclareLaunchArgument(
            'angle_compensate',
            default_value=angle_compensate,
            description='Specifying whether or not to enable angle_compensate of scan data')

    scan_mode_arg = DeclareLaunchArgument(
            'scan_mode',
            default_value=scan_mode,
            description='Specifying scan mode of lidar')

    lidar = Node(
        package='rplidar_ros',
        executable='rplidar_node',
        name='rplidar_node',
        parameters=[{
            'channel_type':channel_type,
            'serial_port': serial_port, 
            'serial_baudrate': serial_baudrate, 
            'frame_id': frame_id,
            'inverted': inverted, 
            'angle_compensate': angle_compensate,
            'scan_mode': scan_mode,
            'use_sim_time': use_sim_time
        }],
        output='screen'
    )

    laser_filter = Node(
        package="carrierbot_slam",
        executable="laser_filter.py",
        name="laser_filter",
        output="screen",
        parameters=[{
            "min_range": 0.5,
            "max_range": 12.0,
        }],
        remappings=[
            ("scan", "/scan"),
            ("scan_filtered", "/scan_filtered"),
        ]
    )

    hardware_interface = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("carrierbot_firmware"),
                "launch",
                "hardware_interface.launch.py"
            )
        ),
        launch_arguments={"use_sim_time": use_sim_time}.items()
    )

    controller = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("carrierbot_controller"),
                "launch",
                "controller.launch.py"
            )
        ),
        launch_arguments={"use_sim_time": use_sim_time}.items()
    )

    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("carrierbot_slam"),
                "launch",
                "slam.launch.py"
            )
        ),
        launch_arguments={"use_sim_time": use_sim_time}.items()
    )

    navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory("carrierbot_navigation"),
                "launch",
                "navigation.launch.py"
            )
        ),
        launch_arguments={"use_sim_time": use_sim_time}.items()
    )

    return LaunchDescription([
        use_sim_time_arg,
        channel_type_arg,
        serial_port_arg,
        serial_baudrate_arg,
        frame_id_arg,
        inverted_arg,
        angle_compensate_arg,
        scan_mode_arg,        
                
        hardware_interface,
        lidar,
        laser_filter,
        TimerAction(period=3.0, actions=[controller]),
        TimerAction(period=10.0, actions=[slam]),
        TimerAction(period=15.0, actions=[navigation]),
    ])