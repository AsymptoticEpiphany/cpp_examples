cmake_minimum_required(VERSION 3.14)
project(cpp_examples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(main src/main.cpp)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(main PRIVATE /W4 /WX)
endif()

# GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(tests tests/test_print_tuple.cpp)
target_link_libraries(tests gtest_main)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR})

include(GoogleTest)
gtest_discover_tests(tests)
