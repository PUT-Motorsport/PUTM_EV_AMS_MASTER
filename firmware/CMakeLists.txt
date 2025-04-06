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

# Find python packages and include them in cmake 
find_package(PythonInterp 3.12 REQUIRED)
find_package(PythonLibs 3.12 REQUIRED)

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

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
# Add Components
add_subdirectory(Components)
# Add Tasks
add_subdirectory(Threads)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

file(GLOB UX_DCD_STM32_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/usbx/common/usbx_stm32_device_controllers/ux_dcd_stm32_*.c")
# message(UX_DCD_STM32_SRC=${UX_DCD_STM32_SRC})

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${UX_DCD_STM32_SRC}
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/ST/usbx/common/usbx_stm32_device_controllers/
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    PUTM_USE_CAN_FD
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries    
)

# Modify HAL drivers this will not work without find package
add_custom_target(
    CUSTOM_COMMANDS ALL
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/custom_spi.py ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/custom_uart.py ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Modifing HAL drivers"  
)

add_dependencies(${CMAKE_PROJECT_NAME} CUSTOM_COMMANDS)

# Add user compile flags
set(GCC_USER_COMPILE_FLAGS
    # g++ throws ((void*)0) conversion errors in threadx libs -> allow it
    "-fpermissive"    
)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_USER_COMPILE_FLAGS}")