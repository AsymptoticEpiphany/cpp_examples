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
make test    # Build and run all unit tests
make clean   # Remove the build directory
```

## Running Unit Tests

- To run all unit tests from the command line:
  ```bash
  make test
  # or
  cd build && ctest
  ```
- To run the test executable directly:
  ```bash
  ./build/tests
  ```
- In Visual Studio Code, you can use the CMake Tools extension or Test Explorer UI to run and debug tests interactively.

## Debugging in VS Code

- Press **F5** to build and launch the program in the debugger.
- The launch configuration is set to run the executable from `build/main`.
- Ensure you have the C++ and CMake Tools extensions for VS Code installed.

## Project Structure
- `src/main.cpp`: Entry point of the application.
- `utils/print_tuple.h`: Utility header for printing tuples.
- `tests/test_print_tuple.cpp`: Unit tests for tuple printing utilities.
- `CMakeLists.txt`: CMake build configuration (with Google Test integration).
- `Makefile`: Delegates build/run/test/clean to CMake.
- `.vscode/`: VS Code configuration files for build, test, and debug.
