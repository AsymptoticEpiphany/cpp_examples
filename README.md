# C++ Example Code
This project currently prints the elements of a tuple nicely.

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

## Debugging in VS Code

- Press **F5** to build and launch the program in the debugger.
- The launch configuration is set to run the executable from `build/main`.
- Ensure you have the C++ extension for VS Code installed.

## Project Structure
- `src/main.cpp`: Entry point of the application.
- `src/utils/print_tuple.h`: Utility header for printing tuples.
- `CMakeLists.txt`: CMake build configuration.
- `Makefile`: Delegates build/run/clean to CMake.
- `.vscode/`: VS Code configuration files for build and debug.
