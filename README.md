# C++ Hello World Project

This is a simple C++ project with a `main.cpp` file that prints "Greetings Universe!!!" to the console.

## Build and Run with CMake

From the project root directory, run the following commands:

```bash
# Configure the project (run once or when CMake files change)
cmake -S . -B build

# Build the project
cmake --build build

# Run the executable
./build/main
```

Alternatively, you can use the provided Makefile (which delegates to CMake):

```bash
make build   # Configure and build using CMake
make run     # Build (if needed) and run the executable
make clean   # Remove the build directory
```

## Project Structure
- `src/main.cpp`: Entry point of the application.
- `CMakeLists.txt`: CMake build configuration.
- `Makefile`: Delegates build/run/clean to CMake.
