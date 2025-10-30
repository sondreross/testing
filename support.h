#ifndef SUPPORT_H
#define SUPPORT_H

// Benchmark function declaration
int run_benchmark(void) __attribute__((noinline));

char* get_benchmark_name(void);

#endif // SUPPORT_H