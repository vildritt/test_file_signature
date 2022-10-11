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
    strategies/abstract.cpp
    strategies/sequental.cpp
    strategies/threaded.cpp
    strategies/threaded/shared_context.cpp
    strategies/threaded/reader_and_hasher_job.cpp
    writers/writer.cpp
    writers/stream_writer.cpp
    writers/file_stream_writer.cpp
)

set(HEADERS
    misc.hpp
    consts.hpp
    types.hpp
    reader.hpp
    slices_scheme.hpp
    strategies/abstract.hpp
    strategies/sequental.hpp
    strategies/threaded.hpp
    strategies/threaded/shared_context.hpp
    strategies/threaded/reader_and_hasher_job.hpp
    writers/writer.hpp
    writers/stream_writer.hpp
    writers/file_stream_writer.hpp
)


add_executable(segmented_signature_cli
    ${SOURCES}
    ${HEADERS}
    usage.txt
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
