# Use CMake to build and run the project
.PHONY: all build clean run test

all: build

build:
	cmake -S . -B build
	cmake --build build

clean:
	rm -rf build	

run: build
	./build/main

test: build
	cd build && ctest


