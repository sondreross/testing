BUILD_DIR = benchmarks-build
BENCHMARK_DIRS = crc32 cubic dijkstra fdct fir matmult matmult-float matmult-int nettle-sha256 rijndael

.PHONY: all clean $(BENCHMARK_DIRS)

all: $(BUILD_DIR) $(BENCHMARK_DIRS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BENCHMARK_DIRS): $(BUILD_DIR)
	@echo "Building $@..."
	$(MAKE) -C $@
	cp $@/*.o $(BUILD_DIR)/

clean:
	rm -rf $(BUILD_DIR)
	@for dir in $(BENCHMARK_DIRS); do \
		echo "Cleaning $$dir..."; \
		$(MAKE) -C $$dir clean; \
	done
