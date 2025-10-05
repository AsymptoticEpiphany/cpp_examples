# Makefile with automatic sanitizer detection for Debug builds
.PHONY: all build clean run test debug release release_sanitize

BUILD_DIR := build
CMAKE_TYPE ?= Debug
FORCE_SANITIZERS ?= 0   # 0=off, 1=on (optional override)

# Determine if sanitizers should be enabled
SANITIZERS_FLAG =
ifeq ($(CMAKE_TYPE),Debug)
    SANITIZERS_FLAG := -DENABLE_SANITIZERS=ON
else ifeq ($(FORCE_SANITIZERS),1)
    SANITIZERS_FLAG := -DENABLE_SANITIZERS=ON
endif

all: build

# General build target
build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CMAKE_TYPE) $(SANITIZERS_FLAG)
	cmake --build $(BUILD_DIR)

# Debug build (automatic sanitizers)
debug:
	$(MAKE) CMAKE_TYPE=Debug build

# Release build (sanitizers off)
release:
	$(MAKE) CMAKE_TYPE=Release build

# Release build with sanitizers forced
release_sanitize:
	$(MAKE) CMAKE_TYPE=Release FORCE_SANITIZERS=1 build

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run main executable
run: build
	./$(BUILD_DIR)/main

# Run unit tests with ASan memory/leak detection
test: build
	ASAN_OPTIONS=detect_leaks=1:abort_on_error=1:color=always cmake --build $(BUILD_DIR) --target test

