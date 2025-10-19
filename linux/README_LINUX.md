# Bubblesort Energy Benchmark for Linux

This is a standalone C++ program that benchmarks bubblesort with energy measurement using Intel RAPL (Running Average Power Limit) on regular Linux.

## Features

- Measures **Package (PKG) energy only** using RAPL MSR
- Outputs results in **CSV format**
- Measures CPU cycles and execution time
- Compatible with Intel CPUs (Sandy Bridge or newer)

## Requirements

- Intel CPU with RAPL support (Sandy Bridge or newer)
- Linux with MSR module
- Root access (for reading MSR registers)
- g++ compiler with C++11 support

## Building

```bash
make
```

Or manually:
```bash
g++ -std=c++11 -O2 -Wall -o bubblesort_bench_linux bubblesort_bench_linux.cpp
```

## Running

```bash
# Load MSR module (if not already loaded)
sudo modprobe msr

# Run the benchmark
sudo ./bubblesort_bench_linux
```

Or use the Makefile:
```bash
make run
```

## Output Format

The program outputs CSV data with the following columns:

- `array_size`: Number of elements in the array
- `cpu_cycles`: Total CPU cycles elapsed
- `cycles_start`: Starting CPU cycle count
- `cycles_end`: Ending CPU cycle count
- `time_ns`: Execution time in nanoseconds
- `time_ms`: Execution time in milliseconds
- `pkg_joules`: Package energy in joules
- `pkg_mJ`: Package energy in millijoules
- `dram_joules`: (empty - not measured)
- `dram_mJ`: (empty - not measured)
- `total_joules`: Total energy (same as pkg_joules)
- `total_mJ`: Total energy in millijoules

## Example Output

```
array_size,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ
2000,5234567,1234567890,1239802457,1744.856,1.744856,0.012345,12.345,,,0.012345,12.345
50000,328456789,1239802457,1568259246,109485.596,109.485596,0.156789,156.789,,,0.156789,156.789
100000,1315678234,1568259246,2883937480,438559.411,438.559411,0.623456,623.456,,,0.623456,623.456
```

## Troubleshooting

### Permission Denied
Make sure you run with `sudo` or as root.

### MSR Module Not Loaded
```bash
sudo modprobe msr
```

### CPU Frequency Adjustment
The program assumes a 3 GHz CPU for time calculations. You can modify the `cpu_freq_mhz` variable in the code to match your CPU frequency, or read it dynamically from `/proc/cpuinfo`.

## Notes

- Only PKG (Package) domain is measured, DRAM is not included
- Energy counter is 32-bit and may wrap around for very long measurements
- Timing is based on RDTSC which counts CPU cycles
