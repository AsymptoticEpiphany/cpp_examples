cmake_minimum_required(VERSION 3.14)
project(cpp_examples)

# ------------------------------------------------------------------------------
# Core compiler settings
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------------------------
# Sanitizer configuration
# ------------------------------------------------------------------------------
option(ENABLE_SANITIZERS "Enable Address/UB/Leak sanitizers (Debug only by default)" OFF)

# Convert ON/OFF to 1/0 for generator expressions
if(ENABLE_SANITIZERS)
    set(ENABLE_SANITIZERS_BOOL 1)
else()
    set(ENABLE_SANITIZERS_BOOL 0)
endif()

# ------------------------------------------------------------------------------
# Main executable
# ------------------------------------------------------------------------------
add_executable(main src/main.cpp)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

# Compile and link options for ASan/UBSan/LSan
target_compile_options(main PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_SANITIZERS_BOOL}>>:-fsanitize=address -fsanitize=undefined -fsanitize=leak -g>
)
target_link_options(main PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_SANITIZERS_BOOL}>>:-fsanitize=address -fsanitize=undefined -fsanitize=leak>
)

# ------------------------------------------------------------------------------
# Compiler warnings
# ------------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(main PRIVATE /W4 /WX)
endif()

# ------------------------------------------------------------------------------
# GoogleTest setup
# ------------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(UNIT_TEST_FILES
    tests/test_print_tuple.cpp
    tests/test_mpmc_queue.cpp
)

add_executable(unit_tests ${UNIT_TEST_FILES})
target_link_libraries(unit_tests PRIVATE gtest gtest_main)
target_include_directories(unit_tests PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/utils
)

# Compile and link options for unit tests
target_compile_options(unit_tests PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_SANITIZERS_BOOL}>>:-fsanitize=address -fsanitize=undefined -fsanitize=leak -g>
)
target_link_options(unit_tests PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<BOOL:${ENABLE_SANITIZERS_BOOL}>>:-fsanitize=address -fsanitize=undefined -fsanitize=leak>
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

