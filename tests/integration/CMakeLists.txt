# Integration tests
add_executable(test_integration
    test_enumerate.cpp
)
target_include_directories(test_integration PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_integration PRIVATE kromax)
target_include_directories(test_integration PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME integration_tests COMMAND test_integration)