cmake_minimum_required(VERSION 3.13)
project(profilerLib)
set(CMAKE_CXX_STANDARD 17)


add_library(profilerLib INTERFACE)
target_include_directories(profilerLib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")


