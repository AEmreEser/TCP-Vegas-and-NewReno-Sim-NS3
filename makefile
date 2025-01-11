PARTA_SRC := partA.cc
PARTB_SRC := partB.cc

RESULTS_DIR := pcaps
GRAPHS_DIR := graphs
ANALYSIS_SCRIPT := analyze.py

# Default target
all: partA partB analysis

# Build PartA
partA: $(RESULTS_DIR)
	./ns3 run $(basename $(PARTA_SRC)) 
	@mv *.pcap $(RESULTS_DIR)


# Build PartB
partB: $(RESULTS_DIR)
	./ns3 run $(basename $(PARTB_SRC))
	@mv *.pcap $(RESULTS_DIR)

# Analysis: Generate graphs
analysis: $(GRAPHS_DIR)
	python3 $(ANALYSIS_SCRIPT) -o ab
	@mv *.png $(GRAPHS_DIR)
	@echo "Graphs saved to ./$(GRAPHS_DIR)"

# Clean generated files
clean:
	@- rm *.pcap
	@- rm -f $(PARTA_BIN) $(PARTB_BIN)
	@- rm -rf $(RESULTS_DIR)
	@- rm -rf $(GRAPHS_DIR)
	@- rm *.png

$(RESULTS_DIR):
	@mkdir -p $(RESULTS_DIR)

$(GRAPHS_DIR):
	@mkdir -p $(GRAPHS_DIR)

# Phony targets
.PHONY: all partA partB analysis clean
