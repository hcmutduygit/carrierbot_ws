#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import sys
import os

def plot_csv(csv_file):
    """
    Plot data from CSV file
    """
    if not os.path.exists(csv_file):
        print(f"Error: File '{csv_file}' not found!")
        sys.exit(1)
    
    # Read CSV file
    df = pd.read_csv(csv_file)
    
    # Convert timestamp from ms to seconds
    df['time_s'] = df['timestamp_ms'] / 1000.0
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 1, figsize=(12, 8))
    fig.suptitle(f'Carrierbot Data Analysis - {os.path.basename(csv_file)}', fontsize=14, fontweight='bold')
    
    # Convert to numpy arrays to avoid pandas indexing issues
    time_data = df['time_s'].values
    left_encoder = df['left_encoder_rps'].values
    left_cmd = df['left_cmd_rps'].values
    right_encoder = df['right_encoder_rps'].values
    right_cmd = df['right_cmd_rps'].values
    
    # Plot 1: Left wheel
    axes[0].plot(time_data, left_encoder, 'b-', label='Encoder (RPS)', linewidth=2)
    axes[0].plot(time_data, left_cmd, 'r--', label='Command (RPS)', linewidth=2)
    axes[0].set_xlabel('Time (s)')
    axes[0].set_ylabel('Velocity (RPS)')
    axes[0].set_title('Left Wheel')
    axes[0].legend()
    axes[0].grid(True, alpha=0.3)
    
    # Plot 2: Right wheel
    axes[1].plot(time_data, right_encoder, 'g-', label='Encoder (RPS)', linewidth=2)
    axes[1].plot(time_data, right_cmd, 'r--', label='Command (RPS)', linewidth=2)
    axes[1].set_xlabel('Time (s)')
    axes[1].set_ylabel('Velocity (RPS)')
    axes[1].set_title('Right Wheel')
    axes[1].legend()
    axes[1].grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # Save figure
    output_file = csv_file.replace('.csv', '.png')
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"✓ Plot saved to: {output_file}")
    
    # Show plot
    plt.show()
    
    # Print statistics
    print("\n" + "="*50)
    print("Statistics:")
    print("="*50)
    print(f"\nLeft Wheel:")
    print(f"  Encoder - Mean: {df['left_encoder_rps'].mean():.4f}, Max: {df['left_encoder_rps'].max():.4f}, Min: {df['left_encoder_rps'].min():.4f}")
    print(f"  Command - Mean: {df['left_cmd_rps'].mean():.4f}, Max: {df['left_cmd_rps'].max():.4f}, Min: {df['left_cmd_rps'].min():.4f}")
    
    print(f"\nRight Wheel:")
    print(f"  Encoder - Mean: {df['right_encoder_rps'].mean():.4f}, Max: {df['right_encoder_rps'].max():.4f}, Min: {df['right_encoder_rps'].min():.4f}")
    print(f"  Command - Mean: {df['right_cmd_rps'].mean():.4f}, Max: {df['right_cmd_rps'].max():.4f}, Min: {df['right_cmd_rps'].min():.4f}")
    
    print(f"\nTotal samples: {len(df)}")
    print(f"Duration: {df['time_s'].max():.2f} seconds")
    print("="*50)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        # Auto-find latest CSV file in data folder
        data_dir = "/home/duybuntu/carrierbot_ws/src/carrierbot_firmware/data"
        if not os.path.exists(data_dir):
            print(f"Error: Data directory '{data_dir}' not found!")
            sys.exit(1)
        
        csv_files = sorted([f for f in os.listdir(data_dir) if f.endswith('.csv')])
        if not csv_files:
            print("No CSV files found!")
            sys.exit(1)
        
        csv_file = os.path.join(data_dir, csv_files[-1])  # Latest file
        print(f"Using latest file: {csv_file}\n")
    else:
        csv_file = sys.argv[1]
    
    plot_csv(csv_file)
