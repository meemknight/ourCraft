cmake_minimum_required(VERSION 3.1)
project(stb_image)

add_library(stb_image)
target_sources(stb_image PRIVATE "src/stb_image.cpp" "src/stb_image_write.cpp" )
target_include_directories(stb_image PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")



