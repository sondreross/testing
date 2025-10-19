# Makefile for Linux bubblesort benchmark

CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall
TARGET = bubblesort_bench_linux
SOURCE = bubblesort_bench_linux.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	@echo "Running benchmark (requires root for MSR access)..."
	@echo "If this fails, run: sudo modprobe msr"
	sudo ./$(TARGET)

.PHONY: all clean run
