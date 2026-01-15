cmake_minimum_required(VERSION 3.16)
project(TargetDetectionSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native -flto -ffast-math -funroll-loops")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG -march=native")
endif()

# MSVC optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG /GL")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob1 /DNDEBUG /Zi")
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/TargetDetector.cpp
    src/SensorSimulator.cpp
    src/main.cpp
)

# Benchmark executable
set(BENCHMARK_SOURCES
    src/TargetDetector.cpp
    src/benchmark.cpp
)

# Create executable
add_executable(target_detection ${SOURCES})

# Create benchmark executable
add_executable(benchmark ${BENCHMARK_SOURCES})

# Find required packages
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV development libraries.")
endif()

# Link libraries
target_link_libraries(target_detection PRIVATE ${OpenCV_LIBS})
target_link_libraries(benchmark PRIVATE ${OpenCV_LIBS})

# Performance optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(target_detection PRIVATE -flto)
    target_link_options(target_detection PRIVATE -flto)
    target_compile_options(benchmark PRIVATE -flto)
    target_link_options(benchmark PRIVATE -flto)
endif()

# Math library for Unix systems
if(UNIX)
    target_link_libraries(target_detection PRIVATE m)
endif()

# Install targets
install(TARGETS target_detection
    RUNTIME DESTINATION bin
)

# Add custom target for cleaning build artifacts
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
    COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
)

# Testing support (optional)
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()