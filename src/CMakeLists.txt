cmake_minimum_required(VERSION 3.10)


project(segmented_signature_cli
    VERSION 0.1
    DESCRIPTION "File segmented signature cli tool"
    LANGUAGES CXX)


set(SOURCES
    main.cpp
    usage.cpp
    md5.cpp
)

set(HEADERS
    md5.hpp
    usage.hpp
)


add_executable(segmented_signature_cli
    ${SOURCES}
    ${HEADERS}
)


target_include_directories(segmented_signature_cli
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
