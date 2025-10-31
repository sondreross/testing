#ifndef SUPPORT_H
#define SUPPORT_H

#ifdef __cplusplus
extern "C" {
#endif


// Benchmark function declaration
int benchmark(void) __attribute__((noinline));

// Get benchmark name
char* get_benchmark_name(void);


#ifdef __cplusplus
}
#endif

#endif // SUPPORT_H