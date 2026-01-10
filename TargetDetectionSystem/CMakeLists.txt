cmake_minimum_required(VERSION 3.10)
project(TargetDetectionSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(target_detection ${SOURCES})

# Libraries
find_package(OpenCV REQUIRED)
target_link_libraries(target_detection ${OpenCV_LIBS})

# Math library
target_link_libraries(target_detection m)