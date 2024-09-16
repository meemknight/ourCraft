cmake_minimum_required(VERSION 3.1)
project(stb_image_write)

add_library(stb_image_write)
target_sources(stb_image_write PRIVATE "src/stb_image_write.cpp")
target_include_directories(stb_image_write PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



