cmake_minimum_required(VERSION 3.20)

project(RadarDetection 
    VERSION 2.0.0 
    LANGUAGES CXX 
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)


set(SOURCES
    src/TargetDetector.cpp
    src/main.cpp
)

add_executable(radar_detection ${SOURCES})

target_include_directories(radar_detection PRIVATE ${INCLUDE_DIR})

if(MSVC)
    target_compile_options(radar_detection PRIVATE /W4 /permissive- /Zc:__cplusplus /NOMINMAX)
else()
    target_compile_options(radar_detection PRIVATE -Wall -Wextra -Wpedantic)
endif()