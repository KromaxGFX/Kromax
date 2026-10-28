# Kromax ADR: No Vulkan, No Mesa

## Status: Accepted

## Context
The project goal is to create a graphics API that does not depend on Vulkan, OpenGL, Mesa (nor libgbm/EGL), SDL nor GLFW. The API should speak directly with the Linux kernel (DRM/KMS) and include a software backend as reference.

## Decision
- Zero runtime dependencies besides `libc` and `libstdc++`
- `libdrm` only for the KMS display backend, behind an `IDisplayBackend` interface for later replacement with direct ioctls
- Software driver as reference backend, disabled by default (`KX_BUILD_DRIVER_AMDGPU=OFF`)
- AMD GPU driver remains as empty skeleton, not implemented
- All GPU-specific code isolated behind driver interfaces for future plug-in style `.so` loading (ICD pattern)

## Consequences
- Portable to systems without GPU drivers or X/Wayland
- Easier CI setup (no Vulkan/Mesa packages needed)
- Clear separation between core and optional features
- Future ADR needed for plugin driver system and IR shader design