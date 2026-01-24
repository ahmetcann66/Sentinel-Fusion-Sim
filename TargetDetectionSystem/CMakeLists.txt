cmake_minimum_required(VERSION 3.20)

project(RadarDetection 
    VERSION 2.0.0 
    LANGUAGES CXX 
    DESCRIPTION "Advanced Radar Target Detection System"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    src/TargetDetector.cpp
    src/main.cpp
)

add_executable(radar_detection ${SOURCES})

target_include_directories(radar_detection PRIVATE ${INCLUDE_DIR})

target_compile_features(radar_detection PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(radar_detection PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus
        $<$<CONFIG:Release>:/O2 /DNDEBUG /GL>
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
    )
else()
    # HATA DUZELTME: -fsanitize bayrakları kaldırıldı
    target_compile_options(radar_detection PRIVATE
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Release>:-O3 -march=native -flto -DNDEBUG>
        $<$<CONFIG:Debug>:-g -O0> 
    )

    target_link_options(radar_detection PRIVATE
        $<$<CONFIG:Release>:-flto>
    )

    target_link_libraries(radar_detection PRIVATE m)
endif()

install(TARGETS radar_detection
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

include(CPack)
set(CPACK_PACKAGE_NAME "RadarDetection")
set(CPACK_PACKAGE_VENDOR "Defense Technology")