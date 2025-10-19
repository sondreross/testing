# Makefile for Linux bubblesort benchmark
# Using compiler flags matching IncludeOS build system (LLVM/Clang)
# Note: If clang++ doesn't work, you can change CXX to g++

CXX = clang++

# Compiler flags matching IncludeOS (which uses LLVM/Clang):
# -std=c++20: C++20 standard (matching IncludeOS CMAKE_CXX_STANDARD 20)
# -stdlib=libc++: Use LLVM's C++ standard library (native to clang)
# -O2: Optimization level 2 (matching IncludeOS default optimization)
# -Wall -Wextra: Enable all warnings (matching IncludeOS WARNS)
# -fstack-protector-strong: Stack protection (matching IncludeOS CAPABS)
# -fno-omit-frame-pointer: Keep frame pointer (matching IncludeOS flags)
# -fno-threadsafe-statics: Disable thread-safe static initialization (matching IncludeOS)
# -g: Include debug symbols (matching IncludeOS CAPABS)
CXXFLAGS = -std=c++20 -stdlib=libc++ -O2 -Wall -Wextra -fstack-protector-strong -fno-omit-frame-pointer -fno-threadsafe-statics -g

# Need to link against libc++ and libc++abi
LDFLAGS = -stdlib=libc++ -lc++abi

TARGET = bubblesort_bench_linux
SOURCE = bubblesort_bench_linux.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	@echo "Running benchmark (requires root for MSR access)..."
	@echo "If this fails, run: sudo modprobe msr"
	sudo ./$(TARGET)

.PHONY: all clean run

