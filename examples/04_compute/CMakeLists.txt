# Example 04: compute (stub)
add_executable(kromax_example_04_compute main.cpp)
target_link_libraries(kromax_example_04_compute PRIVATE kromax)
target_include_directories(kromax_example_04_compute PRIVATE ${CMAKE_SOURCE_DIR}/include)