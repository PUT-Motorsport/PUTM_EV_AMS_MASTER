cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# find_package(PythonInterp 2.7 REQUIRED)
# find_package(PythonLibs 2.7 REQUIRED)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PUTM_EV_AMS_MASTER_2025)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM CXX)

# Core project settings
project(${CMAKE_PROJECT_NAME})
# message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB CXX_SRC "${PROJECT_SOURCE_DIR}/Core/Src/*.cpp")
# message(CXX_SRC=${CXX_SRC})

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CXX_SRC}
    # Add user sources here
)

# file(GLOB CXX_INC "${PROJECT_SOURCE_DIR}/Core/Inc/*.hpp")
# message(CXX_INC=${CXX_INC})

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # ${CXX_INC}
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Modify SPI HAL drivers
add_custom_target(
    CUSTOM_COMMANDS ALL
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/custom_spi.py ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/custom_uart.py ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Modifing HAL drivers"
)

add_dependencies(${CMAKE_PROJECT_NAME} CUSTOM_COMMANDS)