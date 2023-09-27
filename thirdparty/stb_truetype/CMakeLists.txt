cmake_minimum_required(VERSION 3.1)
project(stb_truetype)

add_library(stb_truetype)
target_sources(stb_truetype PRIVATE "src/stb_truetype")
target_include_directories(stb_truetype PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

