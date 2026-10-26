cmake_minimum_required(VERSION 3.14)
project(NetworkRoutingSimulator VERSION 1.0 LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # Use -std=c++17, not -std=gnu++17

option(BUILD_TESTS          "Build unit tests (fetches GoogleTest)" ON)
option(WARNINGS_AS_ERRORS   "Treat compiler warnings as errors"     OFF)

# Compiler warnings (GCC and Clang), applied to our targets only
function(set_project_warnings target)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if (WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# Core logic: no console I/O, linked by both the app and the tests
add_library(routing_core STATIC
    src/Router.cpp
    src/Graph.cpp
    src/DijkstraRouting.cpp
    src/BFSRouting.cpp
    src/DisjointSet.cpp
    src/NetworkSimulator.cpp
)
target_include_directories(routing_core PUBLIC include)
set_project_warnings(routing_core)

# Interactive console application
add_executable(NetworkRoutingSimulator
    src/main.cpp
    src/ConsoleUI.cpp
)
target_link_libraries(NetworkRoutingSimulator PRIVATE routing_core)
set_project_warnings(NetworkRoutingSimulator)

# Optional: copy sample data to build directory so the binary can find it easily
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

if (BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(unit_tests
        tests/unit/GraphTest.cpp
        tests/unit/RoutingTest.cpp
        tests/unit/DisjointSetTest.cpp
        tests/unit/NetworkSimulatorTest.cpp
    )
    target_link_libraries(unit_tests PRIVATE routing_core GTest::gtest_main)
    target_compile_definitions(unit_tests PRIVATE
        SAMPLE_NETWORK_PATH="${CMAKE_SOURCE_DIR}/data/sample_network.txt")
    set_project_warnings(unit_tests)

    include(GoogleTest)
    gtest_discover_tests(unit_tests)

    # End-to-end tests: drive the real binary through scripted menu input
    add_test(NAME cli_end_to_end
        COMMAND ${CMAKE_COMMAND} -E env BIN=$<TARGET_FILE:NetworkRoutingSimulator>
                bash ${CMAKE_SOURCE_DIR}/tests/run_tests.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
