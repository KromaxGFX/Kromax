# Unit tests
add_executable(test_unit
    test_header_abi.cpp
    test_result.cpp
    test_allocator.cpp
    test_expected.cpp
)
target_include_directories(test_unit PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_unit PRIVATE kromax)
target_include_directories(test_unit PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME unit_tests COMMAND test_unit)