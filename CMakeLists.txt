# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.22)

# Define the project name
project(Low_latency_DPDK)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Adjust compiler flags for CPU architecture optimization
# This enables SIMD instructions and maximizes performance
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native")

# Find and configure DPDK
find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

# Set up include and link directories for DPDK
include_directories(${DPDK_INCLUDE_DIRS})
link_directories(${DPDK_LIBRARY_DIRS})
add_definitions(${DPDK_CFLAGS_OTHER})

# Define the executable and its source files
add_executable(Low_latency_DPDK
        main.cpp
        DPDKSetup.cpp
        MarketDataHandler.cpp
        OrderBook.cpp
        TCPIPStack.h
        TCPIPStack.cpp
        OrderProtocol.cpp
        OrderProtocol.h
)

# Link the executable with DPDK libraries
target_link_libraries(Low_latency_DPDK ${DPDK_LIBRARIES})