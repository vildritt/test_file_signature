cmake_minimum_required(VERSION 3.10)


project(segmented_signature_cli
    VERSION 0.1
    DESCRIPTION "File segmented signature cli tool"
    LANGUAGES CXX)


find_package(Boost REQUIRED)

set(SOURCES
    main.cpp
    misc.cpp
    md5.cpp
    reader.cpp
    slices_scheme.cpp
    digest.cpp
    thread_pool.cpp
    strategies/abstract.cpp
    strategies/sequental.cpp
    strategies/threaded.cpp
)

set(HEADERS
    md5.hpp
    misc.hpp
    consts.hpp
    types.hpp
    reader.hpp
    slices_scheme.hpp
    digest.hpp
    thread_pool.hpp
    strategies/abstract.hpp
    strategies/sequental.hpp
    strategies/threaded.hpp
)


add_executable(segmented_signature_cli
    ${SOURCES}
    ${HEADERS}
)


target_link_libraries(segmented_signature_cli
    PRIVATE Boost::boost pthread)

target_include_directories(segmented_signature_cli
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
