# Example 06: Material PBR
add_executable(kromax_example_06_material_pbr main.cpp)
target_link_libraries(kromax_example_06_material_pbr PRIVATE kromax)
target_include_directories(kromax_example_06_material_pbr PRIVATE ${CMAKE_SOURCE_DIR}/include)