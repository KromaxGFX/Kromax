# Example 03: triangle (stub)
add_executable(kromax_example_03_triangle main.cpp)
target_link_libraries(kromax_example_03_triangle PRIVATE kromax)
target_include_directories(kromax_example_03_triangle PRIVATE ${CMAKE_SOURCE_DIR}/include)