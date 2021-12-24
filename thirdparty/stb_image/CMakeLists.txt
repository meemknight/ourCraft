cmake_minimum_required(VERSION 3.1)
project(stb_image)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")