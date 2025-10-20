#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

// MSR addresses
#define MSR_RAPL_POWER_UNIT    0x606
#define MSR_PKG_ENERGY_STATUS  0x611

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

// Function to generate random data
std::vector<int> generate_random_array(size_t size) {
    std::vector<int> arr;
    arr.reserve(size);
    
    // Simple pseudo-random number generation
    unsigned int seed = 12345;
    for (size_t i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        arr.push_back(static_cast<int>(seed % 10000));
    }
    
    return arr;
}

// Bubblesort implementation
void bubblesort(std::vector<int>& arr) {
    size_t n = arr.size();
    bool swapped;
    
    for (size_t i = 0; i < n - 1; i++) {
        swapped = false;
        for (size_t j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // Swap elements
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }
        // If no swaps were made, array is sorted
        if (!swapped) break;
    }
}

int main() {
    const int cpu = 0; // Use CPU 0
    
    // Get energy unit
    uint64_t power_unit_raw = read_msr(cpu, MSR_RAPL_POWER_UNIT);
    double energy_unit = 1.0 / (1 << ((power_unit_raw >> 8) & 0x1F));
    
    // Print CSV header
    printf("array_size,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");
    
    const size_t sizes[] = {2000, 50000, 100000};
    const int repetitions = 30;
    
    for (size_t size : sizes) {
        for (int rep = 0; rep < repetitions; rep++) {
            auto data = generate_random_array(size);
        
        // Read energy and timestamp before
        uint64_t energy_before = read_msr(cpu, MSR_PKG_ENERGY_STATUS);
        uint64_t cycles_start = rdtsc_begin();
        
        // Run bubblesort
        bubblesort(data);
        
        // Read energy and timestamp after
        uint64_t cycles_end = rdtsc_end();
        uint64_t energy_after = read_msr(cpu, MSR_PKG_ENERGY_STATUS);
        
        // Calculate results
        uint64_t cycles_elapsed = cycles_end - cycles_start;
        
        // Handle energy counter wraparound (64-bit counter)
        uint64_t energy_delta;
        if (energy_after >= energy_before) {
            energy_delta = energy_after - energy_before;
        } else {
            // Wraparound case (though unlikely with 64-bit counter)
            energy_delta = (UINT64_MAX - energy_before) + energy_after + 1;
        }
        
        double pkg_joules = energy_delta * energy_unit;
        
        // Read actual CPU frequency from /proc/cpuinfo
        double cpu_freq_mhz = 3000.0; // Default fallback
        FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
        if (cpuinfo) {
            char line[256];
            while (fgets(line, sizeof(line), cpuinfo)) {
                if (strncmp(line, "cpu MHz", 7) == 0) {
                    double mhz = 0.0;
                    if (sscanf(line, "cpu MHz\t: %lf", &mhz) == 1 && mhz > 0.0) {
                        cpu_freq_mhz = mhz;
                        break;
                    }
                }
            }
            fclose(cpuinfo);
        }
        double time_ns = (cycles_elapsed * 1000.0) / cpu_freq_mhz;
        double time_ms = time_ns / 1000000.0;
        
        // Print CSV row (PKG only, no DRAM)
        printf("%zu,%llu,%llu,%llu,%.3f,%.6f,%.6f,%.3f,,,%.6f,%.3f\n",
            size,
            (unsigned long long)cycles_elapsed,
            (unsigned long long)cycles_start,
            (unsigned long long)cycles_end,
            time_ns,
            time_ms,
            pkg_joules,
            pkg_joules * 1000,
            pkg_joules,  // total = pkg only
            pkg_joules * 1000
        );
        }
    }
    
    return 0;
}
