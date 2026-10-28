# Example 01: clear image offscreen to PPM
add_executable(kromax_example_01_clear_offscreen main.cpp)
target_link_libraries(kromax_example_01_clear_offscreen PRIVATE kromax)
target_include_directories(kromax_example_01_clear_offscreen PRIVATE ${CMAKE_SOURCE_DIR}/include)