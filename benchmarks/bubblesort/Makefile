CC = gcc
CFLAGS = -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -ffunction-sections -fdata-sections -Wall -Wextra
TARGET = libbubblesort.o
SOURCE = libbubblesort.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -c -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
	
.PHONY: all clean