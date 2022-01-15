cmake_minimum_required(VERSION 3.1)
project(fastNoiseSIMD)

# Define MY_SOURCES to be a list of all the source files for my game 
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/FastNoiseSIMD/*.cpp")

add_library(fastNoiseSIMD)
target_sources(fastNoiseSIMD PRIVATE ${MY_SOURCES})
target_include_directories(fastNoiseSIMD PUBLIC "FastNoiseSIMD")