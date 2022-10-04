cmake_minimum_required(VERSION 3.10)

project(segmented_signature
    VERSION 0.1
    DESCRIPTION "File segmented signature"
    LANGUAGES CXX)

# global setup

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_subdirectory(src)

add_custom_target(segmented_signature-docs
    SOURCES
        README.md
        LICENSE.md
        TODO
)
