cmake_minimum_required(VERSION 2.8)

project(Zilog)
include_directories(include)

set (PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(Zilog src/Main.cpp src/Disassembler.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra")

