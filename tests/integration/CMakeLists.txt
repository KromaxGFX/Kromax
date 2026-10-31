# Integration tests
add_executable(test_integration
    test_enumerate.cpp
)
target_include_directories(test_integration PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_integration PRIVATE kromax)
target_include_directories(test_integration PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME integration_tests COMMAND test_integration)

# Triangle offscreen integration test
add_executable(test_triangle_offscreen test_triangle_offscreen.cpp)
target_include_directories(test_triangle_offscreen PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_triangle_offscreen PRIVATE kromax)
target_include_directories(test_triangle_offscreen PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME triangle_offscreen_tests COMMAND test_triangle_offscreen)

# Shader languages integration test
add_executable(test_shader_languages test_shader_languages.cpp)
target_include_directories(test_shader_languages PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_shader_languages PRIVATE kromax)
target_include_directories(test_shader_languages PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME shader_languages_tests COMMAND test_shader_languages)

# Material PBR integration test
add_executable(test_material_pbr test_material_pbr.cpp)
target_include_directories(test_material_pbr PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_material_pbr PRIVATE kromax)
target_include_directories(test_material_pbr PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME material_pbr_tests COMMAND test_material_pbr)

# Compute shader integration test
add_executable(test_compute_shader test_compute_shader.cpp)
target_include_directories(test_compute_shader PRIVATE ${DOCTEST_INCLUDE_DIR})
target_link_libraries(test_compute_shader PRIVATE kromax)
target_include_directories(test_compute_shader PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME compute_shader_tests COMMAND test_compute_shader)