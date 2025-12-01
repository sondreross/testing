CC = clang++
CXXFLAGS = -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -fno-threadsafe-statics -ffunction-sections -fdata-sections -Wall -Wextra -Wno-frame-address
TARGET = bench_linux
SOURCE = bench_linux.cpp
BENCHMARK_OBJS = ../benchmarks/benchmarks-build/crc_32.o \
                 ../benchmarks/benchmarks-build/basicmath_small.o \
                 ../benchmarks/benchmarks-build/dijkstra_small.o \
                 ../benchmarks/benchmarks-build/libfdct.o \
                 ../benchmarks/benchmarks-build/libfir.o \
                 ../benchmarks/benchmarks-build/matmult_float.o \
                 ../benchmarks/benchmarks-build/matmult_int.o \
                 ../benchmarks/benchmarks-build/nettle-sha256.o \
                 ../benchmarks/benchmarks-build/aesxam.o \
                 ../benchmarks/benchmarks-build/aes.o
SUPPORT_HEADER = ../support.h

all: $(TARGET)

$(TARGET): $(SOURCE) $(BENCHMARK_OBJS) $(SUPPORT_HEADER)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(BENCHMARK_OBJS) -lm

clean:
	rm -f $(TARGET)

.PHONY: all clean