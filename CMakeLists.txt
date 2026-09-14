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

FetchContent_MakeAvailable(pybind11)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(apps/backtester_cli)
add_subdirectory(external/googletest EXCLUDE_FROM_ALL)
add_subdirectory(tests)
add_subdirectory(python)
