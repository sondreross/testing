CC = clang++
CXXFLAGS = -std=c++20 -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -fno-threadsafe-statics -ffunction-sections -fdata-sections -Wall -Wextra -Wno-frame-address
TARGET = hello_linux
OBJS = main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(OBJS) -o $(TARGET)

main.o: main.cpp
	$(CC) $(CXXFLAGS) -c main.cpp -o main.o

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

