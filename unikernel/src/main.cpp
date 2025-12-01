#include <os>
#include <service>
#include <energy_bench>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <arch/x86/cpu.hpp>
#include "../../support.h"

// Include all benchmark headers
extern "C" {
#include "../../benchmarks/crc32/crc32.h"
#include "../../benchmarks/cubic/cubic.h"
#include "../../benchmarks/dijkstra/dijkstra.h"
#include "../../benchmarks/fdct/fdct.h"
#include "../../benchmarks/fir/fir.h"
#include "../../benchmarks/matmult/matmult.h"
#include "../../benchmarks/nettle-sha256/nettle_sha256.h"
#include "../../benchmarks/rijndael/rijndael.h"
}

// Array of benchmark functions
typedef int (*benchmark_func_t)(void);
typedef void (*init_func_t)(void);
typedef void (*void_func_t)(void);

// Wrapper functions to convert int-returning benchmarks to void
void crc32_wrapper() { (void)crc32(); }
void cubic_wrapper() { (void)cubic(); }
void dijkstra_wrapper() { (void)dijkstra_bench(); }
void fdct_wrapper() { (void)fdct_bench(); }
void fir_wrapper() { (void)fir(); }
void matmult_wrapper() { (void)matmult(); }
void nettle_sha256_wrapper() { (void)nettle_sha256_bench(); }
void rijndael_wrapper() { (void)rijndael(); }

struct Benchmark {
    const char* name;
    init_func_t init;
    void_func_t func;
};

Benchmark benchmarks[] = {
    {"crc32", initialise_benchmark_crc32, crc32_wrapper},
    {"cubic", initialise_benchmark_cubic, cubic_wrapper},
    {"dijkstra", initialise_benchmark_dijkstra, dijkstra_wrapper},
    {"fdct", initialise_benchmark_fdct, fdct_wrapper},
    {"fir", initialise_benchmark_fir, fir_wrapper},
    {"matmult", initialise_benchmark_matmult, matmult_wrapper},
    {"nettle-sha256", initialise_benchmark_nettle_sha256, nettle_sha256_wrapper},
    {"rijndael", initialise_benchmark_rijndael, rijndael_wrapper}
};

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

  const int repetitions = 1;
  
  // Run each benchmark
  for (size_t b = 0; b < sizeof(benchmarks) / sizeof(benchmarks[0]); b++) {
    for (int i = 0; i < repetitions; ++i) {
      benchmarks[b].init();
      auto result = energy_bench::bench_function(
        benchmarks[b].func,
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
        benchmarks[b].name,
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
  }
  os::shutdown();
}
