cmake_minimum_required(VERSION 3.5)
project(Conceptum)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -O0")
set(dir ./)
set(SOURCE_FILES src/main.c src/memman.c)
add_executable(Conceptum ${SOURCE_FILES})
SET(EXECUTABLE_OUTPUT_PATH ${dir})