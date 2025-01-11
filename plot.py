import matplotlib.pyplot as plt
import csv

# Read the CSV file
loads = []
throughputs = []
delays = []

with open('flow_metrics.csv', mode='r') as file:
    csv_reader = csv.reader(file)
    next(csv_reader)  # Skip header
    for row in csv_reader:
        load = float(row[1])
        throughput = float(row[2])
        delay = float(row[3])

        loads.append(load)
        throughputs.append(throughput)
        delays.append(delay)

# Plot Throughput vs Load
plt.figure(figsize=(10, 6))

# Plot throughput (no connecting line)
plt.subplot(211)
plt.plot(loads, throughputs, 'bo', label='Throughput')  # 'bo' means blue color with circle markers
plt.xlabel('Load (Mbps)')
plt.ylabel('Throughput (Mbps)')
plt.title('Throughput vs Load')
plt.grid(True)

# Plot delay (no connecting line)
plt.subplot(212)
plt.plot(loads, delays, 'rx', label='Delay')  # 'rx' means red color with x markers
plt.xlabel('Load (Mbps)')
plt.ylabel('Delay (s)')
plt.title('Delay vs Load')
plt.grid(True)

plt.tight_layout()
plt.savefig('part_a_graphs.png')
plt.show()
