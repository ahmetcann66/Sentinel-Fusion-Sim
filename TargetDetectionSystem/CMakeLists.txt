cmake_minimum_required(VERSION 3.20)
project(RadarDetection VERSION 2.0.0 LANGUAGES CXX DESCRIPTION "Advanced Radar Target Detection System")

# C++20 Configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif()

# Windows için temiz Compiler Flags (Sanitizer'lar kaldırıldı)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Debug: Sadece temel hata ayıklama bilgisi
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
    # Release: Optimizasyon açık
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall -Wextra")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /W4 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL /W3 /EHsc")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/TargetDetector.cpp
    src/MonitorInterface.cpp
    src/main.cpp
)

# Create executable
add_executable(radar_detection ${SOURCES})

# Target properties
set_target_properties(radar_detection PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)