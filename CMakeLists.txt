cmake_minimum_required(VERSION 3.14)
project(NetworkRoutingSimulator VERSION 1.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Use -std=c++17, not -std=gnu++17

# Compiler warnings (GCC and Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Collect all source files
set(SOURCES
    src/main.cpp
    src/Router.cpp
    src/Graph.cpp
    src/DijkstraRouting.cpp
    src/BFSRouting.cpp
    src/DisjointSet.cpp
    src/NetworkSimulator.cpp
)

# Create executable
add_executable(NetworkRoutingSimulator ${SOURCES})

# Add include directory
target_include_directories(NetworkRoutingSimulator PRIVATE include)

# Optional: copy sample data to build directory so the binary can find it easily
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
