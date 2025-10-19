#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

// RAPL MSR addresses
#define MSR_RAPL_POWER_UNIT      0x606
#define MSR_PKG_ENERGY_STATUS    0x611

// Function to read MSR (requires root privileges)
uint64_t read_msr(int cpu, uint32_t reg) {
    uint64_t data;
    std::ostringstream path;
    path << "/dev/cpu/" << cpu << "/msr";
    
    std::ifstream msr_file(path.str(), std::ios::binary);
    if (!msr_file.is_open()) {
        std::cerr << "Error: Cannot open " << path.str() << std::endl;
        std::cerr << "Make sure msr kernel module is loaded (sudo modprobe msr)" << std::endl;
        std::cerr << "And run this program with sudo" << std::endl;
        return 0;
    }
    
    msr_file.seekg(reg, std::ios::beg);
    msr_file.read(reinterpret_cast<char*>(&data), sizeof(data));
    msr_file.close();
    
    return data;
}

// Get RAPL energy unit
double get_rapl_units() {
    uint64_t power_unit_msr = read_msr(0, MSR_RAPL_POWER_UNIT);
    uint8_t energy_unit_bits = (power_unit_msr >> 8) & 0x1F;
    double energy_unit_joules = 1.0 / (1 << energy_unit_bits);
    return energy_unit_joules;
}

// Read TSC using inline assembly
static inline uint64_t rdtsc_start() {
    uint32_t lo, hi;
    asm volatile (
        "cpuid\n\t"
        "rdtsc\n\t"
        : "=a"(lo), "=d"(hi)
        :: "%rbx", "%rcx"
    );
    return ((uint64_t)hi << 32) | lo;
}

static inline uint64_t rdtsc_end() {
    uint32_t lo, hi;
    asm volatile (
        "rdtsc\n\t"
        "lfence\n\t"
        : "=a"(lo), "=d"(hi)
        :
        : "memory"
    );
    return ((uint64_t)hi << 32) | lo;
}

// Get CPU frequency from /proc/cpuinfo
double get_cpu_freq_mhz() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    
    while (std::getline(cpuinfo, line)) {
        if (line.find("cpu MHz") != std::string::npos) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                return std::stod(line.substr(pos + 1));
            }
        }
    }
    return 2400.0; // Default fallback
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

struct BenchResult {
    uint64_t cycles_start;
    uint64_t cycles_end;
    uint64_t cycles_elapsed;
    double energy_joules;
    double time_ns;
    double time_ms;
};

BenchResult benchmark_function(std::function<void()> func, double cpu_freq_mhz) {
    BenchResult result = {0};
    
    // Get energy unit
    double energy_unit = get_rapl_units();
    
    // Read energy before
    uint64_t energy_before = read_msr(0, MSR_PKG_ENERGY_STATUS);
    
    // Capture start time
    result.cycles_start = rdtsc_start();
    
    // Run the function
    func();
    
    // Capture end time
    result.cycles_end = rdtsc_end();
    result.cycles_elapsed = result.cycles_end - result.cycles_start;
    
    // Read energy after
    uint64_t energy_after = read_msr(0, MSR_PKG_ENERGY_STATUS);
    
    // Calculate energy consumption
    uint64_t energy_diff = energy_after - energy_before;
    result.energy_joules = energy_diff * energy_unit;
    
    // Calculate time
    result.time_ns = (result.cycles_elapsed * 1000.0) / cpu_freq_mhz;
    result.time_ms = result.time_ns / 1'000'000.0;
    
    return result;
}

int main() {
    std::cout << "=== Bubblesort Energy Benchmark (Linux) ===" << std::endl;
    std::cout << std::endl;
    
    // Get CPU frequency
    double cpu_freq_mhz = get_cpu_freq_mhz();
    std::cout << "CPU Frequency: " << cpu_freq_mhz << " MHz" << std::endl;
    
    // Check if we can read MSRs
    uint64_t test_msr = read_msr(0, MSR_RAPL_POWER_UNIT);
    if (test_msr == 0) {
        std::cout << "\nWARNING: Cannot read MSRs. Energy measurement will not work." << std::endl;
        std::cout << "Run: sudo modprobe msr" << std::endl;
        std::cout << "Then: sudo ./bubblesort_bench_linux" << std::endl;
        std::cout << "\nContinuing with timing only...\n" << std::endl;
    }
    
    std::cout << "\nNOTE: RAPL energy measurement requires:" << std::endl;
    std::cout << "  - Intel CPU (Sandy Bridge or newer)" << std::endl;
    std::cout << "  - Physical hardware (not VM)" << std::endl;
    std::cout << "  - Root privileges (sudo)" << std::endl;
    std::cout << "  - MSR kernel module loaded" << std::endl;
    std::cout << std::endl;
    
    // Test with different array sizes
    const size_t sizes[] = {2000, 50000, 100000};
    
    for (size_t size : sizes) {
        std::cout << "Testing bubblesort with array size: " << size << std::endl;
        
        // Generate random data
        auto data = generate_random_array(size);
        
        // Benchmark the bubblesort
        auto result = benchmark_function([&data]() {
            bubblesort(data);
        }, cpu_freq_mhz);
        
        // Print results
        std::cout << "  Array size: " << size << " elements" << std::endl;
        std::cout << "  CPU Cycles: " << result.cycles_elapsed 
                  << " (start: " << result.cycles_start 
                  << ", end: " << result.cycles_end << ")" << std::endl;
        std::cout << "  Time: " << std::fixed << std::setprecision(3) 
                  << result.time_ns << " ns (" 
                  << std::setprecision(6) << result.time_ms << " ms)" << std::endl;
        std::cout << "  PKG Energy: " << std::setprecision(6) 
                  << result.energy_joules << " J (" 
                  << std::setprecision(3) << result.energy_joules * 1000 << " mJ)" << std::endl;
        
        // Verify first few elements are sorted
        std::cout << "  First 5 sorted elements: ";
        for (size_t i = 0; i < 5 && i < data.size(); i++) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl << std::endl;
    }
    
    std::cout << "Benchmark complete." << std::endl;
    
    return 0;
}
