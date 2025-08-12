cmake_minimum_required(VERSION 3.10.0)
project(logger_test VERSION 0.1.0 LANGUAGES C CXX)

# reauire c++ 23   
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(logger_test main.cpp)

