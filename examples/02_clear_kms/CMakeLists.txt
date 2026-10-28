# Example 02: clear image via KMS (stub)
add_executable(kromax_example_02_clear_kms main.cpp)
target_link_libraries(kromax_example_02_clear_kms PRIVATE kromax)
target_include_directories(kromax_example_02_clear_kms PRIVATE ${CMAKE_SOURCE_DIR}/include)