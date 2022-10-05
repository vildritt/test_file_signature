cmake_minimum_required(VERSION 3.10)


project(ssig_tests
    VERSION 0.1
    DESCRIPTION "Segmented signatures tests")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# this one works only in debug mode due limits, replace it with other bigger test
add_test(NAME simplest_data_test
    COMMAND segmented_signature_cli "${CMAKE_CURRENT_SOURCE_DIR}/test.data.01" - 10)
set_property(
    TEST simplest_data_test
    PROPERTY PASS_REGULAR_EXPRESSION "781e5e245d69b566979b86e28d23f2c7")
