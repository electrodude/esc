cmake_minimum_required(VERSION 2.8.4)
project(esc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpermissive -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(modules)

include(ragel.cmake)

ragel_gen(blocksplitter)

#add_custom_target(${CMAKE_SOURCE_DIR}/blocksplitter.cpp
#	          DEPENDS ${CMAKE_SOURCE_DIR}/comment.rl)

add_dependencies(ragel_blocksplitter ${CMAKE_CURRENT_SOURCE_DIR}/comment.rl)

add_executable(esc
    main
    loader
    compiler
    blocksplitter
    symtab
    misc)

add_library(module STATIC
    module
    registry)

target_link_libraries(esc module dl m)
