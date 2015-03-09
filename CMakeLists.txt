cmake_minimum_required(VERSION 2.8.4)
project(esc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Og -fpermissive -fPIC")
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(modules)

add_executable(esc
    main
    loader
    compiler
    symtab
    misc)

add_library(module STATIC
    module
    registry)

target_link_libraries(esc module dl m)
