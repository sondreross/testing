CC = gcc
CFLAGS = -O3 -DNDEBUG -g -fstack-protector-strong -fno-omit-frame-pointer -ffunction-sections -fdata-sections -Wall -Wextra

# Auto-discover all benchmark directories
BENCHMARK_DIRS = $(shell find . -maxdepth 1 -type d ! -name '.' ! -name '..' | sed 's|^\./||' | sort)

.PHONY: all clean list

all:
	@for dir in $(BENCHMARK_DIRS); do \
		if [ -f "$$dir/Makefile" ] && [ ! -f "$$dir/Makefile.am" ]; then \
			echo "Building $$dir using its Makefile..."; \
			$(MAKE) -C $$dir || exit 1; \
		else \
			for src in $$dir/lib*.c $$dir/*.c; do \
				if [ -f "$$src" ] && grep -q "int benchmark()" "$$src" 2>/dev/null; then \
					obj=$${src%.c}.o; \
					echo "Compiling $$src -> $$obj"; \
					if [ "$$dir" = "sglib-arraysort" ] && [ "$$(basename $$src)" = "arraysort.c" ]; then \
						$(CC) $(CFLAGS) -DQUICK_SORT -c -o $$obj $$src || exit 1; \
					elif [ "$$dir" = "sglib-arrayheapsort" ] && [ "$$(basename $$src)" = "arraysort.c" ]; then \
						$(CC) $(CFLAGS) -DHEAP_SORT -c -o $$obj $$src || exit 1; \
					elif [ "$$dir" = "sglib-arrayquicksort" ] && [ "$$(basename $$src)" = "arraysort.c" ]; then \
						$(CC) $(CFLAGS) -DQUICK_SORT -c -o $$obj $$src || exit 1; \
					elif [ "$$dir" = "matmult" ] && [ "$$(basename $$src)" = "matmult.c" ]; then \
						$(CC) $(CFLAGS) -DMATMULT_INT -c -o $$obj $$src || exit 1; \
					elif [ "$$dir" = "matmult-float" ] && [ "$$(basename $$src)" = "matmult.c" ]; then \
						$(CC) $(CFLAGS) -DMATMULT_FLOAT -c -o $$obj $$src || exit 1; \
					elif [ "$$dir" = "matmult-int" ] && [ "$$(basename $$src)" = "matmult.c" ]; then \
						$(CC) $(CFLAGS) -DMATMULT_INT -c -o $$obj $$src || exit 1; \
					else \
						$(CC) $(CFLAGS) -c -o $$obj $$src || exit 1; \
					fi; \
				fi; \
			done; \
		fi; \
	done

clean:
	@for dir in $(BENCHMARK_DIRS); do \
		if [ -f "$$dir/Makefile" ] && [ ! -f "$$dir/Makefile.am" ]; then \
			echo "Cleaning $$dir using its Makefile..."; \
			$(MAKE) -C $$dir clean 2>/dev/null || true; \
		fi; \
		find $$dir -name '*.o' -delete 2>/dev/null || true; \
	done

list:
	@echo "Benchmark directories:"
	@for dir in $(BENCHMARK_DIRS); do echo "  $$dir"; done
