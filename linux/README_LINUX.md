# Bubblesort Energy Benchmark for Linux

This is a standalone C++ program that benchmarks bubblesort on Linux, measuring both execution time and energy consumption using Intel RAPL.

## Requirements

- Linux system with Intel CPU (Sandy Bridge or newer for RAPL support)
- GCC or Clang with C++17 support
- Root privileges (for reading MSRs)
- MSR kernel module

## Building

```bash
cd /home/sondrsro/git/sondrIncludeOS/example/src
make
```

## Running

1. Load the MSR kernel module (if not already loaded):
```bash
sudo modprobe msr
```

2. Run the benchmark with sudo:
```bash
sudo ./bubblesort_bench_linux
```

Or use the Makefile:
```bash
make run
```

## What it does

The benchmark:
- Tests bubblesort with arrays of size 2,000, 50,000, and 100,000 elements
- Measures CPU cycles using RDTSC
- Measures energy consumption using Intel RAPL (Reading via MSRs)
- Reports timing in nanoseconds and milliseconds
- Reports energy in Joules and millijoules

## Output Example

```
=== Bubblesort Energy Benchmark (Linux) ===

CPU Frequency: 2400.0 MHz

NOTE: RAPL energy measurement requires:
  - Intel CPU (Sandy Bridge or newer)
  - Physical hardware (not VM)
  - Root privileges (sudo)
  - MSR kernel module loaded

Testing bubblesort with array size: 2000
  Array size: 2000 elements
  CPU Cycles: 15234567 (start: 123456789, end: 138691356)
  Time: 5078123.456 ns (5.078123 ms)
  PKG Energy: 0.030700 J (30.700 mJ)
  First 5 sorted elements: 0 1 3 4 11

...
```

## Troubleshooting

**Cannot read MSRs:**
- Make sure you're running with sudo
- Load the MSR module: `sudo modprobe msr`
- Check if `/dev/cpu/0/msr` exists

**Energy always shows 0:**
- Your CPU might not support RAPL
- You might be in a VM (RAPL usually doesn't work in VMs)
- Try running on bare metal Intel hardware

**Cycles are 0 or identical:**
- The TSC might be disabled or not working properly
- Try on different hardware

## Cleaning

```bash
make clean
```
