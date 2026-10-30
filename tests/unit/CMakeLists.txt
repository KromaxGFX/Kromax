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

# Buffer/Image tests
add_executable(test_buffer_image test_buffer_image.cpp)
target_link_libraries(test_buffer_image PRIVATE kromax)
target_include_directories(test_buffer_image PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME buffer_image_tests COMMAND test_buffer_image)

# Command buffer tests
add_executable(test_commands test_commands.cpp)
target_link_libraries(test_commands PRIVATE kromax)
target_include_directories(test_commands PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME command_tests COMMAND test_commands)

# RenderPass/ImageView/Framebuffer tests
add_executable(test_renderpass test_renderpass.cpp)
target_link_libraries(test_renderpass PRIVATE kromax)
target_include_directories(test_renderpass PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME renderpass_tests COMMAND test_renderpass)