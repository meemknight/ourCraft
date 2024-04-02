cmake_minimum_required(VERSION 3.1)
project(gl2d)

add_library(gl2d)
target_sources(gl2d PRIVATE "src/gl2d.cpp" "src/gl2dParticleSystem.cpp")
target_include_directories(gl2d PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(gl2d PUBLIC glm glad stb_image stb_truetype)