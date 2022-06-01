cmake_minimum_required(VERSION 3.1)
project(glui)

add_library(glui)
target_sources(glui PRIVATE "src/glui.cpp")
target_include_directories(glui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(glui PUBLIC glm glad stb_image stb_truetype gl2d)