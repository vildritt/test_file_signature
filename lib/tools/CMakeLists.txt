cmake_minimum_required(VERSION 3.10)


project(tools
    VERSION 0.1
    DESCRIPTION "misc tools"
    LANGUAGES CXX)


set(SOURCES
    thread_pool.cpp
    hash/md5.cpp
)

set(HEADERS
    include/tools/thread_pool.hpp
    include/tools/hash/md5.hpp
)


add_library(tools
    ${SOURCES}
    ${HEADERS}
)


target_link_libraries(tools
    PUBLIC pthread)

target_include_directories(tools
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
