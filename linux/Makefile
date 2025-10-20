CC = clang++
CXXFLAGS = -std=c++20 -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -fno-threadsafe-statics -ffunction-sections -fdata-sections -Wall -Wextra -Wno-frame-address
TARGET = bubblesort_bench_linux
SOURCE = bubblesort_bench_linux.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean

