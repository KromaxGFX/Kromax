# Example 00: enumerate physical devices
add_executable(kromax_example_00_enumerate main.cpp)
target_link_libraries(kromax_example_00_enumerate PRIVATE kromax)
target_include_directories(kromax_example_00_enumerate PRIVATE ${CMAKE_SOURCE_DIR}/include)