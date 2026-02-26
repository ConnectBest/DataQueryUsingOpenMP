# ====== CONFIG ======

BUILD_DIR := build
TARGET    := tlc
DATA      ?= /Users/mkennedy/MiniData/tlc_2023/fhvhv_2023_milesge4.csv
THREADS   ?= 1
LIMIT     ?= 0

# ====== DEFAULT TARGET ======

.PHONY: all
all: build

# ====== CONFIGURE ======

.PHONY: configure
configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

# ====== BUILD ======

.PHONY: build
build: configure
	cmake --build $(BUILD_DIR) -j

# ====== RUN ======

.PHONY: run
run: build
	OMP_NUM_THREADS=$(THREADS) ./$(BUILD_DIR)/$(TARGET) -p $(DATA) --limit $(LIMIT)

# ====== BENCH (WITH RSS) ======

.PHONY: bench
bench: build
	OMP_NUM_THREADS=$(THREADS) /usr/bin/time -l ./$(BUILD_DIR)/$(TARGET) -p $(DATA) --limit 0

# ====== CLEAN ======

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# ====== REBUILD ======

.PHONY: rebuild
rebuild: clean build