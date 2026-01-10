cmake_minimum_required(VERSION 3.18)
project(FourierAnimation LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags (conditional for MSVC vs GCC/Clang)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# Jetson AGX Orin specific settings
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    message(STATUS "Building for ARM64 (Jetson)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8.2-a+crypto+fp16+simd")
    
    # CUDA architecture for Jetson AGX Orin (Ampere)
    set(CMAKE_CUDA_ARCHITECTURES 87)
endif()

# =============================================================================
# Find Dependencies
# =============================================================================

# OpenCV (required)
find_package(OpenCV REQUIRED COMPONENTS 
    core 
    imgproc 
    imgcodecs 
    videoio
    highgui
)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")

# spdlog (required)
find_package(spdlog CONFIG REQUIRED)
message(STATUS "spdlog found")

# indicators (required, for progress bar)
find_package(indicators CONFIG REQUIRED)
message(STATUS "indicators found")

# CUDA (optional, for hardware acceleration)
find_package(CUDAToolkit)
if(CUDAToolkit_FOUND)
    message(STATUS "CUDA version: ${CUDAToolkit_VERSION}")
    add_definitions(-DUSE_CUDA)
endif()

# GStreamer (optional, for hardware video encoding on Jetson)
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(GSTREAMER gstreamer-1.0)
    if(GSTREAMER_FOUND)
        message(STATUS "GStreamer found: ${GSTREAMER_VERSION}")
        add_definitions(-DUSE_GSTREAMER)
    endif()
    
    # Cairo (optional, for high-quality rendering)
    pkg_check_modules(CAIRO cairo)
    if(CAIRO_FOUND)
        message(STATUS "Cairo found: ${CAIRO_VERSION}")
        add_definitions(-DUSE_CAIRO)
    endif()
endif()

# Try finding Cairo via vcpkg/CMake if pkg-config didn't find it
if(NOT CAIRO_FOUND)
    find_package(unofficial-cairo CONFIG QUIET)
    if(unofficial-cairo_FOUND)
        set(CAIRO_FOUND TRUE)
        set(CAIRO_LIBRARIES unofficial::cairo::cairo)
        message(STATUS "Cairo found via vcpkg")
        add_definitions(-DUSE_CAIRO)
    endif()
endif()

# =============================================================================
# Source Files
# =============================================================================

set(HEADERS
    include/fourier.hpp
    include/contour_extractor.hpp
    include/animation.hpp
    include/video_writer.hpp
)

set(SOURCES
    src/fourier.cpp
    src/contour_extractor.cpp
    src/animation.cpp
    src/video_writer.cpp
    src/main.cpp
)

# =============================================================================
# Executable
# =============================================================================

add_executable(fourier_animation ${SOURCES} ${HEADERS})

target_include_directories(fourier_animation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/color/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/kissfft
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(fourier_animation PRIVATE
    ${OpenCV_LIBS}
    spdlog::spdlog_header_only
    indicators::indicators
)

# CUDA linking if available
if(CUDAToolkit_FOUND)
    target_link_libraries(fourier_animation PRIVATE
        CUDA::cudart
    )
endif()

# GStreamer linking if available
if(GSTREAMER_FOUND)
    target_include_directories(fourier_animation PRIVATE ${GSTREAMER_INCLUDE_DIRS})
    target_link_libraries(fourier_animation PRIVATE ${GSTREAMER_LIBRARIES})
endif()

# Cairo linking if available
if(CAIRO_FOUND)
    if(CAIRO_INCLUDE_DIRS)
        target_include_directories(fourier_animation PRIVATE ${CAIRO_INCLUDE_DIRS})
    endif()
    target_link_libraries(fourier_animation PRIVATE ${CAIRO_LIBRARIES})
endif()

# =============================================================================
# Installation
# =============================================================================

install(TARGETS fourier_animation
    RUNTIME DESTINATION bin
)

# =============================================================================
# Summary
# =============================================================================

message(STATUS "")
message(STATUS "=== Fourier Animation Build Configuration ===")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenCV:         ${OpenCV_VERSION}")
message(STATUS "CUDA:           ${CUDAToolkit_FOUND}")
message(STATUS "GStreamer:      ${GSTREAMER_FOUND}")
message(STATUS "Cairo:          ${CAIRO_FOUND}")
message(STATUS "==============================================")
message(STATUS "")
