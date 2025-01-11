#!/usr/bin/python3
import argparse
import csv
import matplotlib.pyplot as plt

# Set up the command-line argument parser
parser = argparse.ArgumentParser(description="Plot throughput, delay, and packet loss from CSV files.")
parser.add_argument("--afile", "-a", help="The CSV file containing the flow statistics for A (e.g., a_metrics.csv)", default="a_metrics.csv")
parser.add_argument("--bfile", "-b", help="The CSV file containing the flow statistics for B (e.g., b_metrics.csv)", default="b_metrics.csv")
parser.add_argument("--option", "-o", choices=['a', 'b', 'ab'], help="Option to process data for 'a' or 'b'", default='ab', required=True)

# Parse command-line arguments
args = parser.parse_args()
filename_a = args.afile
filename_b = args.bfile
option = args.option

# Lists to store data for plotting
loads = []
throughputs = []
delays = []
packet_loss_list = []

# Read data from 'a_metrics.csv'
if option in ['a', 'ab']:
    with open(filename_a, mode='r') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)
        for row in csv_reader:
            load = float(row[1])
            throughput = float(row[2])
            delay = float(row[3])

            loads.append(load)
            throughputs.append(throughput)
            delays.append(delay)

# Plot Throughput vs Load and Delay vs Load if option 'a' or 'both' is chosen
if option in ['a', 'ab']:
    plt.figure(figsize=(10, 6))

    # Plot throughput (no connecting line)
    plt.subplot(211)
    plt.plot(loads, throughputs, 'bo-', label='Throughput')  # 'bo' means blue color with circle markers
    plt.xlabel('Load (Mbps)')
    plt.ylabel('Throughput (Mbps)')
    plt.title('Throughput vs Load')
    plt.grid(True)

    # Plot delay (no connecting line)
    plt.subplot(212)
    plt.plot(loads, delays, 'ro-', label='Delay')  # 'rx' means red color with x markers
    plt.xlabel('Load (Mbps)')
    plt.ylabel('Delay (s)')
    plt.title('Delay vs Load')
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('part_a_graph.png')
    plt.show()


# Read data from 'b_metrics.csv'
if option in ['b', 'ab']:
    loads.clear()
    throughputs.clear()
    delays.clear()
    with open(filename_b, mode='r') as file:
        csv_reader = csv.reader(file)
        next(csv_reader)
        # Extract packet loss values for 'b' data
        for row in csv_reader:
            load = float(row[1])
            throughput = float(row[2])
            delay = float(row[3])

            loads.append(load)
            throughputs.append(throughput)
            delays.append(delay)
            packet_loss = float(row[4])  # Extracting the packet loss percentage
            packet_loss_list.append(packet_loss)

# Plot Packet Loss if option 'b' or 'both' is chosen
if option in ['b', 'ab']:
    # Print packet loss values
    # print("Packet Loss Values from CSV (B):")
    # for packet_loss in packet_loss_list:
        # print(f"{packet_loss:.2f}%")

    # first half: new reno, 2nd hafl: vegas
    mid_point = len(packet_loss_list) // 2
    plt.figure(figsize=(11, 6))

    # Plot packet loss with labeled segments for newReno and TCP Vegas
    plt.subplot(311)
    plt.plot(loads[:mid_point], packet_loss_list[:mid_point], 'ro-', label='newReno')  # First half: newReno
    plt.plot(loads[mid_point:], packet_loss_list[mid_point:], 'bo-', label='TCP Vegas')  # Second half: TCP Vegas
    plt.title("Packet Loss vs Load")
    plt.xlabel("Load (Mbps)")
    plt.ylabel("Packet Loss (%)")
    plt.legend()
    plt.grid(True)

    # Plot throughput (no connecting line)
    plt.subplot(312)
    plt.plot(loads[:mid_point], throughputs[:mid_point], 'ro-', label='newReno')  # First half: newReno
    plt.plot(loads[mid_point:], throughputs[mid_point:], 'bo-', label='TCP Vegas')  # Second half: TCP Vegas
    plt.xlabel('Load (Mbps)')
    plt.ylabel('Throughput (Mbps)')
    plt.title('Throughput vs Load')
    plt.legend()
    plt.grid(True)

    # Plot delay (no connecting line)
    plt.subplot(313)
    plt.plot(loads[:mid_point], delays[:mid_point], 'ro-', label='newReno')  # First half: newReno
    plt.plot(loads[mid_point:], delays[mid_point:], 'bo-', label='TCP Vegas')  # Second half: TCP Vegas
    plt.xlabel('Load (Mbps)')
    plt.ylabel('Delay (s)')
    plt.title('Delay vs Load')
    plt.legend()
    plt.grid(True)

    plt.tight_layout(pad=2.0)
    plt.savefig('part_b_graph.png')
    plt.show()