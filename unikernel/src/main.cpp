#include <os>
#include <service>
#include <energy_bench>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <arch/x86/cpu.hpp>
#include "../../support.h"

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

void Service::start(const std::string&){
  // Print CSV header matching Linux format
  printf("benchmark,cpu_cycles,cycles_start,cycles_end,time_ns,time_ms,temp_before,temp_after,pkg_temp_before,pkg_temp_after,pkg_joules,pkg_mJ,dram_joules,dram_mJ,total_joules,total_mJ\n");

  const int repetitions = 30;
  
  for (int i = 0; i < repetitions; ++i) {
    initialise_benchmark();
    auto result = energy_bench::bench_function(
      (void (*)(void))benchmark,
      energy_bench::PKG
    );

    double temp_before = result.therm_tcc - result.therm_start;
    double temp_after = result.therm_tcc - result.therm_end;
    double pkg_temp_before = result.therm_tcc - result.pkg_therm_start;
    double pkg_temp_after = result.therm_tcc - result.pkg_therm_end;
    double time_ns = result.nanos_elapsed;
    double time_ms = time_ns / 1e6;
    double pkg_joules = result.pkg_joules();
    double dram_joules = (result.measured_domains & energy_bench::DRAM) ? result.dram_joules() : 0.0;
    double total_joules = result.total_joules();

    // Print CSV row matching Linux format
    printf("%s,%llu,%llu,%llu,%.3f,%.6f,%.2f,%.2f,%.2f,%.2f,%.6f,%.3f,%.6f,%.3f,%.6f,%.3f\n",
      get_benchmark_name(),
      (unsigned long long)result.cycles_elapsed,
      (unsigned long long)result.cycles_start,
      (unsigned long long)result.cycles_end,
      time_ns,
      time_ms,
      temp_before,
      temp_after,
      pkg_temp_before,
      pkg_temp_after,
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
