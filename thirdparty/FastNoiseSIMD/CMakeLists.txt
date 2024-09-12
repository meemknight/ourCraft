cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(fastNoiseSIMD)


if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -msse4.1")
endif (UNIX)

# Define MY_SOURCES to be a list of all the source files for my game 
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/FastNoiseSIMD/*.cpp")

add_library(fastNoiseSIMD)
target_sources(fastNoiseSIMD PRIVATE ${MY_SOURCES})
target_include_directories(fastNoiseSIMD PUBLIC "FastNoiseSIMD")