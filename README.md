# C++ Example Code
This project currently has an example of Multi Producer Multi Consumer queue.
It also has a utility function that prints the elements of a tuple nicely and includes Google Test. 
Continuous Integration (CI) is set up using GitHub Actions.

## Build and Run with Make

From the project root directory, run the following commands (which delegates to CMake):

```bash
make build             # Configure and build the project (default: Debug with memory sanitizers)
make debug             # Build Debug version with memory sanitizers enabled
make release           # Build Release version without memory sanitizers (optimized for production)
make release_sanitize  # Build Release version with memory sanitizers enabled
make run               # Build (if needed) and run the main executable
make test              # Build (if needed) and run all unit tests (memory sanitizers enabled in Debug)
make clean             # Remove the build directory and all generated files

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
- In Visual Studio Code you can use the CMake Tools extension or Test Explorer UI to run and debug tests interactively.

## Debugging in VS Code

- Press **F5** to build and launch the program in the debugger.
- The launch configuration is set to run the executable from `build/main`.
- Ensure you have the C++ and CMake Tools extensions for VS Code installed.

## Continuous Integration (CI)

This project uses GitHub Actions to automatically build and test all code on every push and pull request.
The workflow is defined in .github/workflows/run_unit_tests.yml and performs the following steps:

- Checks out the code
- Installs dependencies (cmake, g++, ninja-build)
- Configures and builds the project in Debug mode with sanitizers enabled
- Runs all unit tests via CTest, with AddressSanitizer/UBSan/Leak checks enabled in Debug builds
- Optionally builds a Release version optimized for production (without sanitizers)
- Keeps Debug and Release builds separate to avoid conflicts
- You can view the CI status and logs on the GitHub Actions tab.

![Build Status](https://github.com/AsymptoticEpiphany/cpp_examples/actions/workflows/run_unit_tests.yml/badge.svg)

## Project Structure
- `src/main.cpp`: Entry point of the application.
- `utils/print_tuple.h`: Utility header for printing tuples.
- `tests/test_print_tuple.cpp`: Unit tests for tuple printing utilities.

- `CMakeLists.txt`: CMake build configuration (with Google Test integration).
- `Makefile`: Delegates build/run/test/clean to CMake.

- `.vscode/`: VS Code configuration files for build, test, and debug.
- `.github/workflows/run_unit_tests.yml`: GitHub Actions workflow for CI.

