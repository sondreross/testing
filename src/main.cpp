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
  printf("=== Bubblesort Energy Benchmark ===\n");
  printf("Platform: %s\n", os::arch());
  printf("NOTE: RAPL energy measurement requires:\n");
  printf("  - Intel CPU (Sandy Bridge or newer)\n");
  printf("  - Physical hardware (not VM)\n");
  printf("  - Proper MSR access\n\n");
  
  // Test with different array sizes
  const size_t sizes[] = {2000, 50000, 100000};
  
  for (size_t size : sizes) {
    printf("Testing bubblesort with array size: %zu\n", size);
    
    // Generate random data
    auto data = generate_random_array(size);
    
    // Benchmark the bubblesort - use only PKG domain which is most widely supported
    auto result = energy_bench::bench_function(
      [&data]() {
        bubblesort(data);
      },
      energy_bench::PKG  // Measure only CPU package (most compatible)
    );
    
    // Print results
    printf("  Array size: %zu elements\n", size);
    printf("  CPU Cycles: %llu (start: %llu, end: %llu)\n",
           (unsigned long long)result.cycles_elapsed,
           (unsigned long long)result.cycles_start,
           (unsigned long long)result.cycles_end);
    printf("  Time: %.3f ns (%.6f ms)\n", 
           result.time_ns(os::cpu_freq().count() / 1000),
           result.time_ns(os::cpu_freq().count() / 1000) / 1'000'000.0);
    printf("  PKG Energy:  %.6f J (%.3f mJ)\n", 
           result.pkg_joules(), result.pkg_joules() * 1000);
    
    if (result.measured_domains & energy_bench::DRAM) {
      printf("  DRAM Energy: %.6f J (%.3f mJ)\n", 
             result.dram_joules(), result.dram_joules() * 1000);
    }
    printf("  Total Energy: %.6f J (%.3f mJ)\n", 
           result.total_joules(), result.total_joules() * 1000);
    
    // Verify first few elements are sorted
    printf("  First 5 sorted elements: ");
    for (size_t i = 0; i < 5 && i < data.size(); i++) {
      printf("%d ", data[i]);
    }
    printf("\n\n");
  }
  
  printf("Benchmark complete. Shutting down...\n");
  os::shutdown();
}
