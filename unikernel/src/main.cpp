#include <os>
#include <service>
#include <energy_bench>
#include <cstdlib>
#include <ctime>
#include <arch/x86/cpu.hpp>
#include "../../bubblesort.h"

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

// Function to generate random data (malloced array)
int* generate_random_array(size_t size) {
  int* arr = (int*)malloc(size * sizeof(int));
  if (!arr) return nullptr;
  unsigned int seed = 12345;
  for (size_t i = 0; i < size; i++) {
    seed = seed * 1103515245 + 12345;
    arr[i] = static_cast<int>(seed % 10000);
  }
  return arr;
}

void Service::start(const std::string&){
  // Print CSV header (same as Linux version)
  printf("array_size,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");

  const size_t sizes[] = {100000};
  const int repetitions = 30;
  
  for (size_t size : sizes) {
    for (int rep = 0; rep < repetitions; rep++) {
      int* data = generate_random_array(size);
      if (!data) continue;
      auto result = energy_bench::bench_function(
        [&data, size]() {
          bubblesort(data, size);
        },
        energy_bench::PKG
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
      free(data);
    }
  }
  os::shutdown();
}
