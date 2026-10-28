# Kromax Architecture Documentation

## Layers (from top to bottom)

1. **`src/api/`**: Implements C functions. Validates arguments, translates handle ↔ internal object, captures errors and returns `KxResult`. No business logic here.

2. **`src/layers/`**: Decorator layers over the dispatch table. First layer: `validation` (verifies correct usage, object lifecycle, command buffer states).

3. **`src/core/`**: Instance, PhysicalDevice, Device, Queue and object management; allocator (`kx::Allocator` compatible with STL and backed by `KxAllocationCallbacks`), logging, `Expected<T>`, utilities.

4. **`src/driver/`**: Abstract interface `kx::IDriver`/`IDevice` and driver registration (static at startup; future ADR for `.so` plugin ICD-style).
   - `software/`: CPU executor. Memory, buffers, images, interprets command buffers, rasterizer (edge functions, depth, clipping) and compute. Shaders are callbacks C++ registered as `KxShaderModule` with format `KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT`; format `KX_SHADER_FORMAT_KIR_EXT` reserved for future.
   - `amdgpu/`: Empty skeleton, disabled by default (`KX_BUILD_DRIVER_AMDGPU=OFF`).

5. **`src/display/`**: Presentation backends behind `IDisplayBackend`:
   - `kms/`: DRM/KMS with dumb buffers, modeset, page flip and vblank. Requires running from a TTY without active compositor (DRM master).
   - `offscreen/`: Writes frames to PPM (own writer). Default backend for CI and development within a desktop session.

## Principles

- RAII and internal `std::unique_ptr`, zero mutable global state except driver registry, clear object lifecycle, and `const`-correctness.
- Assignable everything goes through the allocator.
- Errors with custom type `kx::Expected<T>` (C++20 does not have `std::expected`); `KxResult` at the boundary.
- No exceptions or RTTI crossing the API boundary.

## Object Hierarchy

```
KxInstance → KxPhysicalDevice → KxDevice → KxQueue
       ↓
KxDeviceMemory, KxBuffer, KxImage, KxImageView,
KxShaderModule, KxPipelineLayout, KxPipeline (compute & graphics),
KxCommandPool, KxCommandBuffer, KxFence, KxSemaphore

Extension display: KxDisplayEXT, KxSwapchainEXT
Extension debug: KxDebugMessengerEXT
```

## End-to-End Flow: `kxQueueSubmit`

1. User records commands into a `KxCommandBuffer` (via `kxCmd*`)
2. Submit via `kxQueueSubmit` with submit info (command buffers, fences, semaphores)
3. Internal dispatch translates handles to internal objects
4. Command buffer replay/interpretation (software driver) or ICD handoff
5. Synchronization via fences/semaphores

## Error Handling

- `KxResult`: successes >= 0, errors < 0
- Internal: `kx::Expected<T>` for recoverable errors
- Boundary: `KxResult` returned to C API caller
- All public functions document valid usage and synchronization requirements