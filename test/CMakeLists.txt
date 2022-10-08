cmake_minimum_required(VERSION 3.10)


project(ssig_tests
    VERSION 0.1
    DESCRIPTION "Segmented signatures tests")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# TODO 1: this one works only in debug mode due block size limit (512), fix it to support release!
if(false)
    add_test(NAME md5_simple_validity
        COMMAND segmented_signature_cli "${CMAKE_CURRENT_SOURCE_DIR}/test.data.01" - 10)
    set_property(
        TEST md5_simple_validity
        PROPERTY PASS_REGULAR_EXPRESSION "781e5e245d69b566979b86e28d23f2c7")
endif()

add_test(NAME files_sigs
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
