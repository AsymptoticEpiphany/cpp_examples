# Use CMake to build and run the project
.PHONY: all build run clean

all: build

build:
	cmake -S . -B build
	cmake --build build

run: build
	./build/main

clean:
	rm -rf build
