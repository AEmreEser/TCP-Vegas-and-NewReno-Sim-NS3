#!/usr/bin/python3

import sys
import os
import numpy as np
import matplotlib.pyplot as plt
import re
from collections import defaultdict

class NetworkAnalyzer:
    def __init__(self):
        self.part_a_results = defaultdict(lambda: {'throughput': [], 'delay': [], 'bytes': []})
        self.part_b_results = defaultdict(lambda: defaultdict(lambda: {'throughput': [], 'delay': [], 'bytes': []}))

    def parse_part_a(self, filename):
        """Parse Part A simulation results"""
        current_load = None

        with open(filename, 'r') as f:
            lines = f.readlines()

        for line in lines:
            if "Load:" in line:
                match = re.search(r"Load: (\d+\.?\d*)", line)
                if match:
                    current_load = float(match.group(1))
            elif "Throughput:" in line:
                throughput = float(re.search(r"Throughput: (\d+\.?\d*)", line).group(1))
                self.part_a_results[current_load]['throughput'].append(throughput)
            elif "Average Delay:" in line:
                delay = float(re.search(r"Average Delay: (\d+\.?\d*)", line).group(1))
                self.part_a_results[current_load]['delay'].append(delay)
            elif "Total Bytes Received:" in line:
                bytes_received = int(re.search(r"Total Bytes Received: (\d+)", line).group(1))
                self.part_a_results[current_load]['bytes'].append(bytes_received)

    def parse_part_b(self, filename):
        """Parse Part B simulation results"""
        current_load = None
        current_variant = None

        with open(filename, 'r') as f:
            lines = f.readlines()

        for line in lines:
            if "TCP Variant:" in line:
                current_variant = re.search(r"TCP Variant: (\w+)", line).group(1)
            elif "Load:" in line:
                match = re.search(r"Load: (\d+\.?\d*)", line)
                if match:
                    current_load = float(match.group(1))
            elif "Throughput:" in line:
                throughput = float(re.search(r"Throughput: (\d+\.?\d*)", line).group(1))
                self.part_b_results[current_variant][current_load]['throughput'].append(throughput)
            elif "Average Delay:" in line:
                delay = float(re.search(r"Average Delay: (\d+\.?\d*)", line).group(1))
                self.part_b_results[current_variant][current_load]['delay'].append(delay)
            elif "Total Bytes Received:" in line:
                bytes_received = int(re.search(r"Total Bytes Received: (\d+)", line).group(1))
                self.part_b_results[current_variant][current_load]['bytes'].append(bytes_received)

    def generate_graphs(self, output_dir):
        """Generate all required graphs"""
        os.makedirs(output_dir, exist_ok=True)

        # Part A Graphs
        self._plot_part_a_graphs(output_dir)

        # Part B Graphs
        self._plot_part_b_graphs(output_dir)

        # Comparison Graphs
        # self._plot_comparison_graphs(output_dir)

    def _plot_part_a_graphs(self, output_dir):
        """Generate graphs for Part A"""
        loads = sorted(self.part_a_results.keys())
        if not loads:
            print("No data available for Part A graphs.")
            return

        metrics = {
            'throughput': ('Throughput (Mbps)', 'Load vs Throughput'),
            'delay': ('Delay (ms)', 'Load vs Delay')
        }

        for metric, (ylabel, title) in metrics.items():
            plt.figure(figsize=(10, 6))
            values = [
                np.mean(self.part_a_results[load][metric])
                if self.part_a_results[load][metric]
                else None
                for load in loads
            ]

            # Filter out None values
            valid_data = [(load, value) for load, value in zip(loads, values) if value is not None]
            if not valid_data:
                print(f"No valid data for Part A: {metric}.")
                continue

            valid_loads, valid_values = zip(*valid_data)
            plt.plot(valid_loads, valid_values, 'bo-', label='Part A')
            plt.xlabel('Load (Mbps)')
            plt.ylabel(ylabel)
            plt.title(f'Part A: {title}')
            plt.grid(True)
            plt.savefig(os.path.join(output_dir, f'part_a_{metric}.png'))
            plt.close()

    def _plot_part_b_graphs(self, output_dir):
        """Generate graphs for Part B"""
        variants = list(self.part_b_results.keys())
        if not variants:
            return

        loads = sorted(self.part_b_results[variants[0]].keys())
        metrics = {
            'throughput': ('Throughput (Mbps)', 'Load vs Throughput'),
            'delay': ('Delay (ms)', 'Load vs Delay')
        }

        for metric, (ylabel, title) in metrics.items():
            plt.figure(figsize=(10, 6))
            for variant in variants:
                values = [np.mean(self.part_b_results[variant][load][metric]) for load in loads]
                plt.plot(loads, values, 'o-', label=variant)
            plt.xlabel('Load (Mbps)')
            plt.ylabel(ylabel)
            plt.title(f'Part B: {title}')
            plt.legend()
            plt.grid(True)
            plt.savefig(os.path.join(output_dir, f'part_b_{metric}.png'))
            plt.close()

    def _plot_comparison_graphs(self, output_dir):
        """Generate comparison graphs between parts"""
        metrics = {
            'throughput': ('Throughput (Mbps)', 'Load vs Throughput'),
            'delay': ('Delay (ms)', 'Load vs Delay')
        }

        for metric, (ylabel, title) in metrics.items():
            plt.figure(figsize=(12, 6))

            # Plot Part A
            loads_a = sorted(self.part_a_results.keys())
            values_a = [np.mean(self.part_a_results[load][metric]) for load in loads_a]
            plt.plot(loads_a, values_a, 'bo-', label='Part A')

            # Plot Part B (both variants)
            for variant in self.part_b_results.keys():
                loads_b = sorted(self.part_b_results[variant].keys())
                values_b = [np.mean(self.part_b_results[variant][load][metric])
                           for load in loads_b]
                plt.plot(loads_b, values_b, 'o-', label=f'Part B ({variant})')

            plt.xlabel('Load (Mbps)')
            plt.ylabel(ylabel)
            plt.title(f'Comparison: {title}')
            plt.legend()
            plt.grid(True)
            plt.savefig(os.path.join(output_dir, f'comparison_{metric}.png'))
            plt.close()

def main():
    if len(sys.argv) != 4:
        print("Usage: ./analyze_results.py <part_a_results.txt> <part_b_results.txt> <output_directory>")
        sys.exit(1)

    part_a_file = sys.argv[1]
    part_b_file = sys.argv[2]
    output_directory = sys.argv[3]

    analyzer = NetworkAnalyzer()

    # Parse both result files
    if os.path.exists(part_a_file):
        analyzer.parse_part_a(part_a_file)
    else:
        print(f"Warning: Part A results file {part_a_file} not found")

    if os.path.exists(part_b_file):
        analyzer.parse_part_b(part_b_file)
    else:
        print(f"Warning: Part B results file {part_b_file} not found")

    # Generate graphs
    analyzer.generate_graphs(output_directory)
    print(f"Graphs have been generated in {output_directory}")

if __name__ == "__main__":
    main()
