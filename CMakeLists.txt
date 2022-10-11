cmake_minimum_required(VERSION 3.10)

project(segmented_signature
    VERSION 0.1
    DESCRIPTION "File segmented signature"
    LANGUAGES CXX)

# global setup

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()


add_subdirectory(lib)
add_subdirectory(src)


if(UNIX)
    if(NOT DEFINED SS_TESTS_DISABLED)
        enable_testing()
        add_subdirectory(test)
    endif()
endif (UNIX)

add_custom_target(segmented_signature-docs
    SOURCES
        README.md
        LICENSE.md
        TODO
)
