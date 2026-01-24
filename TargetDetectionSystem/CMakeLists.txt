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

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Debug configuration
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic -fsanitize=address -fsanitize=undefined")
    
    # Release configuration with aggressive optimizations
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -mtune=native -flto -Wall -Wextra")
    
    # Release with debug info
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -Wall -Wextra")
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC configurations
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi /W4 /EHsc /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL /W3 /EHsc")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /W3 /EHsc")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/TargetDetector.cpp
    src/main.cpp
)

# Create executable with better organization
add_executable(radar_detection ${SOURCES})

# Target properties
set_target_properties(radar_detection PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME "radar_detection"
)

# Compiler-specific link options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(radar_detection PRIVATE 
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
    )
endif()

# Link math library for Unix-like systems
if(UNIX AND NOT APPLE)
    target_link_libraries(radar_detection PRIVATE m)
elseif(UNIX AND APPLE)
    target_link_libraries(radar_detection PRIVATE m)
endif()

# Installation
install(TARGETS radar_detection
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Package configuration
include(CPack)
set(CPACK_PACKAGE_NAME "RadarDetection")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Defense Technology")