import sys
if sys.prefix == '/usr':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/home/duybuntu/carrierbot_ws/install/my_robot_communicate'
