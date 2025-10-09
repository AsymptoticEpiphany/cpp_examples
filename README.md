# C++ Example Code
This project currently has an example of Multi Producer Multi Consumer queue.
It also has a utility function that prints the elements of a tuple nicely and includes Google Test. 
Continuous Integration (CI) is set up using GitHub Actions.

General usage after running 'make build' as explained below.

- Open 4 terminal windows.
- Run 'python3 fake_trace_generator.py --tcp --port 5556'
  - Run on ports 5555, 5556, and 5557 in each terminal window.
- Run 'make run' in the main terminal window
- The python scripts will generate fake random data and send on a pipe to their respective ports, and the main cpp file will listen on those ports and begin ingesting that data and putting it on a multi producer multi consumer queue for processing.
- For this to work correctly, there also needs to be a Postgesql DB running with correct permissions. Currently that is hardcoded in main and will come from command line args in the future.

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
## Continuous Integration / Continuous Deployment (CI/CD)
```
This project uses GitHub Actions to automatically build and test all code on every push and pull request.
The workflow is defined in .github/workflows/run_unit_tests.yml and performs the following steps:

- Checks out the code
- Installs dependencies (cmake, g++, ninja-build)
- Configures and builds the project in Debug mode with sanitizers enabled
- Runs all unit tests via CTest, with AddressSanitizer/UBSan/Leak checks enabled in Debug builds
- Optionally builds a Release version optimized for production (without sanitizers)
- Keeps Debug and Release builds separate to avoid conflicts
- You can view the CI status and logs on the GitHub Actions tab.

- Checks out the code from the repository
- Installs dependencies (cmake, g++, ninja-build) on Ubuntu
- Debug build with sanitizers:
  - Configures and builds the project in Debug mode with AddressSanitizer (ASan), UndefinedBehaviorSanitizer (UBSan), and LeakSanitizer enabled
  - Runs all unit tests via CTest, reporting any memory or undefined behavior issues\

- Release build for production:
  - Builds a separate Release version optimized for performance, without sanitizers
  - This Release binary is kept separate from Debug builds to avoid conflicts
  - The binary is optionally uploaded as a CI artifact for inspection or debugging purposes

- Tag-based releases (i.e. git tag v0.0.1 && git push origin v0.0.1):
  - When a commit is tagged (e.g., v1.0.0), the workflow automatically:
    - Rebuilds the Release binary to ensure it’s fresh and reproducible
    - Renames the binary to a versioned format, e.g., my_app_v1.0.0_linux_x64
    - Uploads the versioned binary to GitHub Releases for download and distribution

CI visibility:
- You can view the build and test status, logs, and uploaded artifacts in the GitHub Actions tab
- Tagged releases will appear in the GitHub Releases tab once published.
```
![Build Status](https://github.com/AsymptoticEpiphany/cpp_examples/actions/workflows/run_unit_tests.yml/badge.svg)

## Project Structure
```
cpp_examples/
├── .github/
│   └── workflows/
│       └── run_unit_tests.yml
├── .vscode/
├── include/
│   └── nlohmann
├── src/
├── tests/
├── utils/
├── CMakeLists.txt
├── Makefile
├── README.md
└── .gitignore
└── fake_trace_generator.py

```

