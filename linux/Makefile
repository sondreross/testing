# Makefile for Linux bubblesort benchmark

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET = bubblesort_bench_linux
SOURCE = bubblesort_bench_linux.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	@echo "Note: This program requires root privileges to read MSRs"
	@echo "Running with sudo..."
	sudo ./$(TARGET)

.PHONY: all clean run
