PARTA_SRC := partA.cc
PARTB_SRC := partB.cc

PARTA_RES := results_a.txt
PARTB_RES := results_b.txt

RESULTS_DIR := pcaps
GRAPHS_DIR := graphs
ANALYSIS_SCRIPT := analyze.py

# Default target
all: partA partB analysis

# Build PartA
partA: $(RESULTS_DIR)
	./ns3 run partA > $(PARTA_RES)
	mv *.pcap $(RESULTS_DIR)

# Build PartB
partB: $(RESULTS_DIR)
	./ns3 run partB > $(PARTB_RES)
	mv *.pcap $(RESULTS_DIR)

# Analysis: Generate graphs
analysis: $(RESULTS_DIR)/results_a.txt $(RESULTS_DIR)/results_b.txt $(GRAPHS_DIR)
	python3 $(ANALYSIS_SCRIPT) $(PARTA_RES) $(PARTB_RES) $(RESULTS_DIR)
	mv $(RESULTS_DIR)/*.png $(GRAPHS_DIR)

# Clean generated files
clean:
	@- rm -f $(PARTA_BIN) $(PARTB_BIN)
	@- rm -rf $(RESULTS_DIR)
	@- rm *.pcap

$(RESULTS_DIR):
	mkdir -p $(RESULTS_DIR)

$(GRAPHS_DIR):
	mkdir -p $(GRAPHS_DIR)

$(RESULTS_DIR)/results_a.txt: | $(RESULTS_DIR)
$(RESULTS_DIR)/results_b.txt: | $(RESULTS_DIR)

# Phony targets
.PHONY: all partA partB analysis clean
