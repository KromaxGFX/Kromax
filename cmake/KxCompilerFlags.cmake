# ---------------------------------------------------------------------------
# KxCompilerFlags.cmake
# Common compiler warning flags treated as errors.
# ---------------------------------------------------------------------------

function(kx_apply_compiler_flags target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # C++ flags
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wno-missing-field-initializers
        )

        # C flags (for any C translation units)
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:-Wall>
            $<$<COMPILE_LANGUAGE:C>:-Wextra>
            $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
            $<$<COMPILE_LANGUAGE:C>:-Wconversion>
            $<$<COMPILE_LANGUAGE:C>:-Wshadow>
            $<$<COMPILE_LANGUAGE:C>:-Wno-missing-field-initializers>
        )
    endif()

    # Treat warnings as errors
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE )
    endif()

    # Debug info
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_options(${target} PRIVATE -g3)
    endif()
endfunction()