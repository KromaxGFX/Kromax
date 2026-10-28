# ---------------------------------------------------------------------------
# KxInstall.cmake
# Install rules for the Kromax library.
# ---------------------------------------------------------------------------

function(kx_install_targets target)
    install(TARGETS ${target}
        EXPORT kromax-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )

    install(DIRECTORY include/kromax
        DESTINATION include/kromax
    )

    install(EXPORT kromax-targets
        NAMESPACE kromax::
        DESTINATION lib/cmake/kromax
    )
endfunction()