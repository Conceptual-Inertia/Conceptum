cmake_minimum_required(VERSION 3.5)
project(Conceptum)

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(SOURCE_FILES src/main.c)
add_executable(Conceptum ${SOURCE_FILES})