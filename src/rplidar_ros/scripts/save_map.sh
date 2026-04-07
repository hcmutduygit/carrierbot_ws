#!/bin/bash

# Script to save the map created by Cartographer

# Default values
MAP_NAME="my_map"
OUTPUT_DIR="$HOME/maps"

# Parse command line arguments
while getopts "n:o:h" opt; do
  case $opt in
    n)
      MAP_NAME="$OPTARG"
      ;;
    o)
      OUTPUT_DIR="$OPTARG"
      ;;
    h)
      echo "Usage: $0 [-n map_name] [-o output_directory]"
      echo "  -n: Name of the map (default: my_map)"
      echo "  -o: Output directory (default: ~/maps)"
      exit 0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Full path for the map
MAP_PATH="$OUTPUT_DIR/$MAP_NAME"

echo "======================================"
echo "Cartographer Map Saver"
echo "======================================"
echo "Map name: $MAP_NAME"
echo "Output directory: $OUTPUT_DIR"
echo "======================================"

# Finish the trajectory (this tells Cartographer to stop optimizing and finalize the map)
echo "Finishing trajectory..."
ros2 service call /finish_trajectory cartographer_ros_msgs/srv/FinishTrajectory "{trajectory_id: 0}"

sleep 2

# Save the state (pbstream file)
echo "Saving Cartographer state..."
ros2 service call /write_state cartographer_ros_msgs/srv/WriteState "{filename: '$MAP_PATH.pbstream'}"

sleep 2

# Convert to occupancy grid and save as PGM/YAML
echo "Saving occupancy grid map..."
ros2 run nav2_map_server map_saver_cli -f "$MAP_PATH"

echo "======================================"
echo "Map saved successfully!"
echo "Files created:"
echo "  - $MAP_PATH.pbstream (Cartographer state)"
echo "  - $MAP_PATH.pgm (Occupancy grid image)"
echo "  - $MAP_PATH.yaml (Map metadata)"
echo "======================================"
