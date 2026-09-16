cmake_minimum_required(VERSION 3.24)

project(backtester LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BT_ENABLE_ASAN "ASAN" ON)
option(BT_ENABLE_UBSAN "UBSAN" ON)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.6
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(
    pybind11
    googletest
)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(apps/backtester_cli)
add_subdirectory(tests)
add_subdirectory(python)
