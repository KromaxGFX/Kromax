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
    endif()
endfunction()