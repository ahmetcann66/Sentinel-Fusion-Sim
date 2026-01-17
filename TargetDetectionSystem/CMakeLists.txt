cmake_minimum_required(VERSION 3.20)
project(TargetDetectionSystem 
    VERSION 2.0.0 
    DESCRIPTION "Advanced Multi-Sensor Target Detection System"
    LANGUAGES CXX)

# C++20 standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Modern CMake configuration
include(GNUInstallDirs)
include(FeatureSummary)

# Enhanced compiler options with maximum performance optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast")
    # Maximum release optimizations with aggressive flags
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native -flto=auto -ffast-math -funroll-loops -fno-omit-frame-pointer -fomit-frame-pointer -finline-functions -finline-functions-called-once -fipa-pta -fgraphite-identity -floop-nest-optimize -ftree-loop-vectorize -ftree-slp-vectorize -funsafe-loop-optimizations -fpredictive-commoning -frename-registers -fsched2-use-superblocks -fsched2-use-traces -fsee -fno-semantic-interposition -fvisibility-inlines-hidden")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -fsanitize=address,undefined")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG -march=native -flto=auto -ffast-math")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG -flto=auto -fdata-sections -ffunction-sections -Wl,--gc-sections")
endif()

# Enhanced MSVC optimizations with maximum performance
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
    # Aggressive MSVC optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Ot /GL /DNDEBUG /arch:AVX2 /fp:fast /Qpar /Qipo /Qstd=c++20 /favor:INTEL64 /Qunroll")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Ob1 /GL /DNDEBUG /Zi /arch:AVX2 /fp:fast")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /DDEBUG /Zi")
endif()

# Platform-specific optimizations
if(APPLE)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mcpu=apple-m1")
elseif(UNIX AND NOT APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mavx2 -mfma")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mcpu=cortex-a72")
    endif()
endif()

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# Include directories with proper scope
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files with modern organization
set(COMMON_SOURCES
    src/TargetDetector.cpp
    src/SensorSimulator.cpp
)

# Main application sources
set(MAIN_SOURCES
    ${COMMON_SOURCES}
    src/main.cpp
)

# Benchmark executable sources
set(BENCHMARK_SOURCES
    ${COMMON_SOURCES}
    src/benchmark.cpp
)

# Testing sources (optional)
set(TEST_SOURCES
    ${COMMON_SOURCES}
    tests/test_main.cpp
    tests/test_target_detector.cpp
    tests/test_sensor_simulator.cpp
)

# Find required packages with modern CMake
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV development libraries.")
endif()

# Optional threading library for better performance
find_package(Threads REQUIRED)

# Create main executable with modern target
add_executable(target_detection ${MAIN_SOURCES})

# Create benchmark executable
add_executable(benchmark ${BENCHMARK_SOURCES})

# Set target properties
set_target_properties(target_detection PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "target_detection"
    VERSION ${PROJECT_VERSION}
)

set_target_properties(benchmark PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "benchmark"
    VERSION ${PROJECT_VERSION}
)

# Modern library linking
target_link_libraries(target_detection PRIVATE
    OpenCV::core
    OpenCV::imgproc
    Threads::Threads
)

target_link_libraries(benchmark PRIVATE
    OpenCV::core
    OpenCV::imgproc
    Threads::Threads
)

# Enhanced performance optimizations with aggressive LTO
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(target_detection PRIVATE -flto=auto -fwhole-program-vtables)
    target_link_options(target_detection PRIVATE -flto=auto -fwhole-program-vtables)
    target_compile_options(benchmark PRIVATE -flto=auto -fwhole-program-vtables)
    target_link_options(benchmark PRIVATE -flto=auto -fwhole-program-vtables)
    
    # Enable link-time optimization and profile-guided optimization
    set_property(TARGET target_detection PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    
    # Additional optimizations for GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(target_detection PRIVATE -fgnu89-inline -fipa-sra -fpartial-inlining)
        target_compile_options(benchmark PRIVATE -fgnu89-inline -fipa-sra -fpartial-inlining)
    endif()
endif()

# MSVC LTO support
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET target_detection PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET benchmark PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Math library for Unix systems
if(UNIX)
    target_link_libraries(target_detection PRIVATE m)
    target_link_libraries(benchmark PRIVATE m)
endif()

# Precompile headers for faster compilation
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
    target_precompile_headers(target_detection PRIVATE
        <vector>
        <memory>
        <algorithm>
        <chrono>
        <mutex>
        <span>
        <optional>
    )
    
    target_precompile_headers(benchmark PRIVATE
        <vector>
        <memory>
        <algorithm>
        <chrono>
        <mutex>
        <span>
        <optional>
    )
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

# Modern testing support with Catch2
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" ON)
option(ENABLE_PROFILING "Enable profiling support" OFF)

if(BUILD_TESTING)
    enable_testing()
    
    # Find Catch2 or download it
    find_package(Catch2 QUIET)
    if(NOT Catch2_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.5.0
        )
        FetchContent_MakeAvailable(Catch2)
    endif()
    
    # Create test executable
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests PRIVATE
        ${COMMON_SOURCES}
        OpenCV::core
        OpenCV::imgproc
        Threads::Threads
        Catch2::Catch2WithMain
    )
    
    add_test(NAME UnitTests COMMAND unit_tests)
    
    # Enable CTest integration
    include(CTest)
endif()

# Benchmark configuration
if(BUILD_BENCHMARKS)
    # Find Google Benchmark or download it
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
        )
        FetchContent_MakeAvailable(googlebenchmark)
    endif()
    
    # Re-create benchmark with proper linking
    target_link_libraries(benchmark PRIVATE
        ${COMMON_SOURCES}
        OpenCV::core
        OpenCV::imgproc
        Threads::Threads
        benchmark::benchmark
        benchmark::benchmark_main
    )
endif()

# Profiling support
if(ENABLE_PROFILING AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(target_detection PRIVATE -pg)
        target_link_options(target_detection PRIVATE -pg)
    endif()
endif()

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Defense Technology Solutions")

# Feature summary
feature_summary(WHAT ALL)