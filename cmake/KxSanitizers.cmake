# ---------------------------------------------------------------------------
# KxSanitizers.cmake
# Optional AddressSanitizer / UndefinedBehaviorSanitizer support.
# Cache variables: KX_SANITIZER_ADDRESS, KX_SANITIZER_UNDEFINED
# ---------------------------------------------------------------------------

option(KX_SANITIZER_ADDRESS "Enable AddressSanitizer" OFF)
option(KX_SANITIZER_UNDEFINED "Enable UndefinedBehaviorSanitizer" OFF)

function(kx_apply_sanitizers target)
    if(KX_SANITIZER_ADDRESS OR KX_SANITIZER_UNDEFINED)
        set(sanitizers "")
        if(KX_SANITIZER_ADDRESS)
            list(APPEND sanitizers address)
        endif()
        if(KX_SANITIZER_UNDEFINED)
            list(APPEND sanitizers undefined)
        endif()

        string(REPLACE ";" "," sanitizer_str "${sanitizers}")

        target_compile_options(${target} PRIVATE
            -fsanitize=${sanitizer_str}
            -fno-omit-frame-pointer
        )
        target_link_options(${target} PRIVATE
            -fsanitize=${sanitizer_str}
        )

        # When using Clang, add the sanitizer runtime library path and link explicitly
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # Find the Clang resource directory
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
                OUTPUT_VARIABLE CLANG_RESOURCE_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            if(CLANG_RESOURCE_DIR)
                target_link_directories(${target} PRIVATE
                    "${CLANG_RESOURCE_DIR}/lib/x86_64-unknown-linux-gnu"
                )
                # Explicitly link sanitizer runtime libraries
                if(KX_SANITIZER_ADDRESS)
                    target_link_libraries(${target} PRIVATE clang_rt.asan)
                endif()
                if(KX_SANITIZER_UNDEFINED)
                    target_link_libraries(${target} PRIVATE clang_rt.ubsan_standalone)
                endif()
            endif()
        endif()
    endif()
endfunction()