cmake_minimum_required(VERSION 3.1)
project(raudio)

add_library(raudio)
target_sources(raudio PRIVATE src/raudio.c)
target_include_directories(raudio PUBLIC include)