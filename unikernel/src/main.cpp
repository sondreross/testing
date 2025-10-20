#include <os>
#include <service>
#include <energy_bench>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <arch/x86/cpu.hpp>

// Check if RAPL is available
bool check_rapl_support() {
  // Try reading RAPL power unit MSR
  // If this causes an exception, RAPL is not supported
  try {
    uint64_t test = x86::CPU::read_msr(MSR_RAPL_POWER_UNIT);
    (void)test; // Suppress unused warning
    return true;
  } catch (...) {
    return false;
  }
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

void Service::start(const std::string&){
  // Print CSV header (same as Linux version)
  printf("array_size,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");

  const size_t sizes[] = {2000, 50000, 100000};
  for (size_t size : sizes) {
    auto data = generate_random_array(size);
    auto result = energy_bench::bench_function(
      [&data]() {
        bubblesort(data);
      },
      energy_bench::PKG | energy_bench::DRAM // Try to measure both if available
    );

    double time_ns = result.time_ns(os::cpu_freq().count() / 1000);
    double time_ms = time_ns / 1'000'000.0;
    double pkg_joules = result.pkg_joules();
    double dram_joules = (result.measured_domains & energy_bench::DRAM) ? result.dram_joules() : 0.0;
    double total_joules = result.total_joules();

    // Print CSV row
    printf("%zu,%llu,%llu,%llu,%.3f,%.6f,%.6f,%.3f,%.6f,%.3f,%.6f,%.3f\n",
      size,
      (unsigned long long)result.cycles_elapsed,
      (unsigned long long)result.cycles_start,
      (unsigned long long)result.cycles_end,
      time_ns,
      time_ms,
      pkg_joules,
      pkg_joules * 1000,
      dram_joules,
      dram_joules * 1000,
      total_joules,
      total_joules * 1000
    );
  }
  os::shutdown();
}
