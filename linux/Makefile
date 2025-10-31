CC = clang++
CXXFLAGS = -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -fno-threadsafe-statics -ffunction-sections -fdata-sections -Wall -Wextra -Wno-frame-address
TARGET = bubblesort_bench_linux
SOURCE = bubblesort_bench_linux.cpp
BUBBLESORT_OBJ = ../benchmarks/bubblesort/libbubblesort.o
BUBBLESORT_HEADER = ../benchmarks/bubblesort/bubblesort.h

all: $(TARGET)

$(TARGET): $(SOURCE) $(BUBBLESORT_OBJ) $(BUBBLESORT_HEADER)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(BUBBLESORT_OBJ) $(BUBBLESORT_HEADER)

clean:
	rm -f $(TARGET)

.PHONY: all clean