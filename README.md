# Simulation and Analysis Workflow

This repository contains the necessary setup to build, run, and analyze simulation results for Part A and Part B using NS-3. It uses a Makefile to streamline the workflow.

## Prerequisites

Ensure the following tools are installed:
1. [NS-3](https://www.nsnam.org/)
2. Python 3 (for running the analysis script)
3. Required Python packages:
   - `numpy`
   - `matplotlib`

Install the Python dependencies with:
```bash
pip install numpy matplotlib
```

---

## Files and Directories

### Source Files
- `partA.cc`: Source code for Part A simulation.
- `partB.cc`: Source code for Part B simulation.

### Output Files
- `results_a.txt`: Output results of Part A.
- `results_b.txt`: Output results of Part B.

### Directories
- `pcaps`: Stores `.pcap` files generated during simulations.
- `graphs`: Stores generated graphs from the analysis.

### Scripts
- `analyze.py`: Python script for analyzing simulation results and generating graphs.

---

## Usage

The workflow is managed using the Makefile. Below are the main commands:

### 1. **Build and Run Part A**
This command runs the Part A simulation, generates results, and moves `.pcap` files to the `pcaps` directory:
```bash
make partA
```

**Example**:
```bash
$ make partA
./ns3 run partA > results_a.txt
mv *.pcap pcaps/
```
- Output: `results_a.txt` in the current directory.
- `.pcap` files moved to the `pcaps` directory.

---

### 2. **Build and Run Part B**
This command runs the Part B simulation, generates results, and moves `.pcap` files to the `pcaps` directory:
```bash
make partB
```

**Example**:
```bash
$ make partB
./ns3 run partB > results_b.txt
mv *.pcap pcaps/
```
- Output: `results_b.txt` in the current directory.
- `.pcap` files moved to the `pcaps` directory.

---

### 3. **Run Analysis**
This command processes the results from Part A and Part B, and generates graphs:
```bash
make analysis
```

**Example**:
```bash
$ make analysis
python3 analyze.py results_a.txt results_b.txt pcaps
mv pcaps/*.png graphs/
```
- Output: Graph images moved to the `graphs` directory.

---

### 4. **Clean All Generated Files**
This command removes all generated files, including results, `.pcap` files, and graphs:
```bash
make clean
```

**Example**:
```bash
$ make clean
rm -f partA partB
rm -rf pcaps
rm *.pcap
```

---

## Workflow Summary

1. Run Part A simulation:
   ```bash
   make partA
   ```
2. Run Part B simulation:
   ```bash
   make partB
   ```
3. Analyze the results and generate graphs:
   ```bash
   make analysis
   ```
4. Clean up generated files (optional):
   ```bash
   make clean
   ```

---

## Notes

- Modify the `Makefile` as needed to adapt to your directory structure or NS-3 environment.
- Ensure the NS-3 `waf` script is configured and that you are running the commands in the root directory of your NS-3 installation.

