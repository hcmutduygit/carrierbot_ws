#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist


class CmdVelAngularInverter(Node):
    def __init__(self):
        super().__init__('cmd_vel_angular_inverter')

        self.declare_parameter('input_topic', '/nav2/cmd_vel_raw')
        self.declare_parameter('output_topic', '/cmd_vel')
        self.declare_parameter('invert_linear', False)
        self.declare_parameter('invert_angular', True)

        input_topic = self.get_parameter('input_topic').value
        output_topic = self.get_parameter('output_topic').value
        self.invert_linear = self.get_parameter('invert_linear').value
        self.invert_angular = self.get_parameter('invert_angular').value

        self.subscription = self.create_subscription(Twist, input_topic, self.on_twist, 10)
        self.publisher = self.create_publisher(Twist, output_topic, 10)

        self.get_logger().info(
            f'Nav2 cmd_vel inverter: {input_topic} -> {output_topic} '
            f'(invert_linear={self.invert_linear}, invert_angular={self.invert_angular})'
        )

    def on_twist(self, msg: Twist):
        output = Twist()
        output.linear.x = -msg.linear.x if self.invert_linear else msg.linear.x
        output.linear.y = msg.linear.y
        output.linear.z = msg.linear.z
        output.angular.x = msg.angular.x
        output.angular.y = msg.angular.y
        output.angular.z = -msg.angular.z if self.invert_angular else msg.angular.z

        self.publisher.publish(output)


def main(args=None):
    rclpy.init(args=args)
    node = CmdVelAngularInverter()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()