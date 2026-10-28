# API tests
add_executable(test_api
    test_instance.cpp
)
target_include_directories(test_api PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_api PRIVATE kromax)
target_include_directories(test_api PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME api_tests COMMAND test_api)