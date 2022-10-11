cmake_minimum_required(VERSION 3.10)


project(segmented_signature_cli
    VERSION 0.1
    DESCRIPTION "File segmented signature cli tool"
    LANGUAGES CXX)


set(SOURCES
    main.cpp
    misc.cpp
    reader.cpp
    slices_scheme.cpp
    digest.cpp
    strategies/abstract.cpp
    strategies/sequental.cpp
    strategies/threaded.cpp
)

set(HEADERS
    misc.hpp
    consts.hpp
    types.hpp
    reader.hpp
    slices_scheme.hpp
    digest.hpp
    strategies/abstract.hpp
    strategies/sequental.hpp
    strategies/threaded.hpp
)


add_executable(segmented_signature_cli
    ${SOURCES}
    ${HEADERS}
    #usage.txt
)


target_link_libraries(segmented_signature_cli
    PRIVATE tools)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(segmented_signature_cli
        PRIVATE pthread)
endif()


target_include_directories(segmented_signature_cli
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
