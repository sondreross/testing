#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <time.h>
#include "../support.h"

// Include all benchmark headers
extern "C" {
#include "../benchmarks/crc32/crc32.h"
#include "../benchmarks/cubic/cubic.h"
#include "../benchmarks/dijkstra/dijkstra.h"
#include "../benchmarks/fdct/fdct.h"
#include "../benchmarks/fir/fir.h"
#include "../benchmarks/matmult-float/matmult_float.h"
#include "../benchmarks/matmult-int/matmult_int.h"
#include "../benchmarks/nettle-sha256/nettle_sha256.h"
#include "../benchmarks/rijndael/rijndael.h"
}

// MSR addresses
#define MSR_RAPL_POWER_UNIT    0x606
#define MSR_PKG_ENERGY_STATUS  0x611
#define MSR_TEMPERATURE_TARGET 0x1A2
#define IA32_THERM_STATUS      0x19C
#define IA32_PACKAGE_THERM_STATUS 0x1B1

// Read MSR value
uint64_t read_msr(int cpu, uint32_t msr) {
    char msr_path[64];
    snprintf(msr_path, sizeof(msr_path), "/dev/cpu/%d/msr", cpu);
    
    int fd = open(msr_path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening MSR device");
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. You are running as root (sudo)\n");
        fprintf(stderr, "  2. msr module is loaded (sudo modprobe msr)\n");
        exit(1);
    }
    
    uint64_t data;
    if (pread(fd, &data, sizeof(data), msr) != sizeof(data)) {
        perror("Error reading MSR");
        close(fd);
        exit(1);
    }
    
    close(fd);
    return data;
}

// Read CPU timestamp counter with full serialization (start of measurement)
static inline uint64_t rdtsc_begin() {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "cpuid\n\t"          // Serialize - wait for all prior instructions
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        : "a"(0)             // cpuid leaf 0
        : "rbx", "rcx"       // cpuid clobbers these
    );
    return ((uint64_t)hi << 32) | lo;
}

// Read CPU timestamp counter with serialization (end of measurement)
static inline uint64_t rdtsc_end() {
    uint32_t lo, hi;
    __asm__ __volatile__ (
        "rdtscp\n\t"         // Read TSC with implicit serialization
        "mov %%edx, %0\n\t"
        "mov %%eax, %1\n\t"
        "cpuid\n\t"          // Serialize - prevent later instructions from starting
        : "=r"(hi), "=r"(lo)
        : 
        : "rax", "rbx", "rcx", "rdx"
    );
    return ((uint64_t)hi << 32) | lo;
}

// Read CPU temperature
double read_cpu_temp(int cpu) {
    // Read temperature target (TjMax)
    uint64_t temp_target = read_msr(cpu, MSR_TEMPERATURE_TARGET);
    uint32_t tj_max = (temp_target >> 16) & 0xFF;
    
    // Read thermal status
    uint64_t therm_status = read_msr(cpu, IA32_THERM_STATUS);
    uint32_t digital_readout = (therm_status >> 16) & 0x7F;
    
    // Calculate temperature: TjMax - Digital Readout
    double temp = tj_max - digital_readout;
    return temp;
}

// Read package temperature
double read_pkg_temp(int cpu) {
    // Read temperature target (TjMax)
    uint64_t temp_target = read_msr(cpu, MSR_TEMPERATURE_TARGET);
    uint32_t tj_max = (temp_target >> 16) & 0xFF;
    
    // Read package thermal status
    uint64_t pkg_therm_status = read_msr(cpu, IA32_PACKAGE_THERM_STATUS);
    uint32_t digital_readout = (pkg_therm_status >> 16) & 0x7F;
    
    // Calculate temperature: TjMax - Digital Readout
    double temp = tj_max - digital_readout;
    return temp;
}

// Array of benchmark functions
typedef int (*benchmark_func_t)(void);
typedef void (*init_func_t)(void);
typedef char* (*name_func_t)(void);

struct Benchmark {
    const char* name;
    init_func_t init;
    benchmark_func_t func;
    name_func_t get_name;
};

Benchmark benchmarks[] = {
    {"crc32", initialise_benchmark_crc32, crc32, get_benchmark_name_crc32},
    {"cubic", initialise_benchmark_cubic, cubic, get_benchmark_name_cubic},
    {"dijkstra", initialise_benchmark_dijkstra, dijkstra_bench, get_benchmark_name_dijkstra},
    {"fdct", initialise_benchmark_fdct, fdct_bench, get_benchmark_name_fdct},
    {"fir", initialise_benchmark_fir, fir, get_benchmark_name_fir},
    {"matmult-float", initialise_benchmark_matmult_float, matmult_float, get_benchmark_name_matmult_float},
    {"matmult-int", initialise_benchmark_matmult_int, matmult_int, get_benchmark_name_matmult_int},
    {"nettle-sha256", initialise_benchmark_nettle_sha256, nettle_sha256_bench, NULL},
    {"rijndael", initialise_benchmark_rijndael, rijndael, get_benchmark_name_rijndael}
};

int main() {
    const int cpu = 0; // Use CPU 0
    
    // Get energy unit
    uint64_t power_unit_raw = read_msr(cpu, MSR_RAPL_POWER_UNIT);
    double energy_unit = 1.0 / (1 << ((power_unit_raw >> 8) & 0x1F));
    
    // Print CSV header (no wall-clock time columns)
    printf("benchmark,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,temp_before,temp_after,pkg_temp_before,pkg_temp_after,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");

    const int repetitions = 30;
    
    // Run each benchmark
    for (size_t b = 0; b < sizeof(benchmarks) / sizeof(benchmarks[0]); b++) {
        for (int rep = 0; rep < repetitions; rep++) {
            benchmarks[b].init();

        // Read temperature, energy and timestamp before
            struct timespec time_start, time_end;
            clock_gettime(CLOCK_MONOTONIC, &time_start);
            double temp_before = read_cpu_temp(cpu);
            double pkg_temp_before = read_pkg_temp(cpu);
            uint64_t energy_before = read_msr(cpu, MSR_PKG_ENERGY_STATUS);
            uint64_t cycles_start = rdtsc_begin();
            
            // Run benchmark
            benchmarks[b].func();
            
        // Read energy, timestamp and temperature after
            uint64_t cycles_end = rdtsc_end();
            uint64_t energy_after = read_msr(cpu, MSR_PKG_ENERGY_STATUS);
            double temp_after = read_cpu_temp(cpu);
            double pkg_temp_after = read_pkg_temp(cpu);
            clock_gettime(CLOCK_MONOTONIC, &time_end);
        
    // Calculate results
    uint64_t cycles_elapsed = cycles_end - cycles_start;
        
        // Calculate wall clock time
        double time_ns = (time_end.tv_sec - time_start.tv_sec) * 1e9 + 
                        (time_end.tv_nsec - time_start.tv_nsec);
        double time_ms = time_ns / 1e6;

        // Handle energy counter wraparound (64-bit counter)
        uint64_t energy_delta;
        if (energy_after >= energy_before) {
            energy_delta = energy_after - energy_before;
        } else {
            // Wraparound case (though unlikely with 64-bit counter)
            energy_delta = (UINT64_MAX - energy_before) + energy_after + 1;
        }
        
        double pkg_joules = energy_delta * energy_unit;
        
            // Print CSV row (PKG only, no DRAM)
            printf("%s,%llu,%llu,%llu,%.3f,%.6f,%.2f,%.2f,%.2f,%.2f,%.6f,%.3f,,,%.6f,%.3f\n",
                benchmarks[b].name,
                (unsigned long long)cycles_elapsed,
                (unsigned long long)cycles_start,
                (unsigned long long)cycles_end,
                time_ns,
                time_ms,
                temp_before,
                temp_after,
                pkg_temp_before,
                pkg_temp_after,
                pkg_joules,
                pkg_joules * 1000,
                pkg_joules,  // total = pkg only
                pkg_joules * 1000
            );
        }
    }
    
    return 0;
}