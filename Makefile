CXX = g++

SRCS = main.cpp debug_utils.cpp # src files

# for debugging
debug:
	$(CXX) $(SRCS) -o reconstruction_debug_hitaesh -DDEBUG

# build for release
release:
	$(CXX) $(SRCS) -o reconstruction_hitaesh -DNDEBUG

# just "make" defaults to release
all: release

run_debug: debug
	./reconstruction_debug_hitaesh mbo.csv

run_release: release
	./reconstruction_hitaesh mbo.csv

# run executable
run: run_release

clean:
	@echo "Cleaning build artifacts..."
	rm -f reconstruction_debug_hitaesh reconstruction_hitaesh
	@echo "Clean complete."

.PHONY: all debug release run_debug run_release run clean