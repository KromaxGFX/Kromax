# Unit tests
add_executable(test_unit
    test_header_abi.cpp
    test_result.cpp
    test_allocator.cpp
    test_expected.cpp
    test_rasterizer.cpp
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

# Descriptor/Sampler tests
add_executable(test_descriptors test_descriptors.cpp)
target_link_libraries(test_descriptors PRIVATE kromax)
target_include_directories(test_descriptors PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME descriptor_tests COMMAND test_descriptors)

# Shader Lexer tests
add_executable(test_shader_lexer test_shader_lexer.cpp)
target_link_libraries(test_shader_lexer PRIVATE kromax)
target_include_directories(test_shader_lexer PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_lexer_tests COMMAND test_shader_lexer)

# Shader Parser tests
add_executable(test_shader_parser test_shader_parser.cpp)
target_link_libraries(test_shader_parser PRIVATE kromax)
target_include_directories(test_shader_parser PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_parser_tests COMMAND test_shader_parser)

# Shader IR tests
add_executable(test_shader_ir test_shader_ir.cpp)
target_link_libraries(test_shader_ir PRIVATE kromax)
target_include_directories(test_shader_ir PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_ir_tests COMMAND test_shader_ir)

# Shader Interpreter tests
add_executable(test_shader_interpreter test_shader_interpreter.cpp)
target_link_libraries(test_shader_interpreter PRIVATE kromax)
target_include_directories(test_shader_interpreter PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_interpreter_tests COMMAND test_shader_interpreter)

# Shader Builtins tests
add_executable(test_shader_builtins test_shader_builtins.cpp)
target_link_libraries(test_shader_builtins PRIVATE kromax)
target_include_directories(test_shader_builtins PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_builtins_tests COMMAND test_shader_builtins)

# Shader Compiler tests
add_executable(test_shader_compiler test_shader_compiler.cpp)
target_link_libraries(test_shader_compiler PRIVATE kromax)
target_include_directories(test_shader_compiler PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/shader)
add_test(NAME shader_compiler_tests COMMAND test_shader_compiler)

# Sampler Filtering tests
add_executable(test_sampler_filtering test_sampler_filtering.cpp)
target_link_libraries(test_sampler_filtering PRIVATE kromax)
target_include_directories(test_sampler_filtering PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src)
add_test(NAME sampler_filtering_tests COMMAND test_sampler_filtering)

# Mipmap Generation tests
add_executable(test_mipmap_generation test_mipmap_generation.cpp)
target_link_libraries(test_mipmap_generation PRIVATE kromax)
target_include_directories(test_mipmap_generation PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src)
add_test(NAME mipmap_generation_tests COMMAND test_mipmap_generation)

# Material Lifecycle tests
add_executable(test_material_lifecycle test_material_lifecycle.cpp)
target_link_libraries(test_material_lifecycle PRIVATE kromax)
target_include_directories(test_material_lifecycle PRIVATE ${DOCTEST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/kromax ${CMAKE_SOURCE_DIR}/src)
add_test(NAME material_lifecycle_tests COMMAND test_material_lifecycle)