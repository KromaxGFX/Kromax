# Kromax Roadmap

## Phase 0: Base (Completed)
- ✅ Scaffolding complete, CI configured
- ✅ Public headers with full API declared
- ✅ `kxCreateInstance`, enumeration of physical devices (software driver appears)
- ✅ `KxResult`, allocator, basic debug messenger
- ✅ Example `00_enumerate`
- ✅ All documentation

## Phase 1: Software Driver, Objects and Commands
- **Device**, **Queue**, **memory**, **buffers**, **images**
- **Command pools/buffers**, **fences**
- **`kxQueueSubmit`**, **`kxCmdClearImage`**, **`kxCmdCopy*`**
- Display **offscreen** (PPM)
- Example `01_clear_offscreen`

## Phase 2: Display KMS
- **Backend DRM/KMS** with swapchain over dumb buffers
- **Modeset**, **page flip**, **vblank**
- Example `02_clear_kms` (real color on screen, without X or Wayland)

## Phase 3: Rasterization and Compute
- **Graphic pipelines** with shader callbacks
- **Vertex buffers**, **depth buffer**
- **`kxCmdDraw`** / **`kxCmdDrawIndexed`**, **`kxCmdDispatch`**
- Examples `03_triangle` and `04_compute`

## Phase 4: Layers and Tools
- **Validation layer**, **complete debug utils**
- **Timestamps/profiling hooks**

## Phase 5: R&D (Document only, not implement)
- **Hardware driver** (AMD first)
- **Own IR shader** (KIR)
- **Driver plugins** `.so`

## Finished Criteria

`cmake --preset ci && cmake --build --preset ci && ctest --preset ci` passes without warnings;
examples 00 and 01 run;
`README.md` allows an unknown person to compile and run the first example in under 5 minutes.

## Current Status

- Phase 0: ✅ Complete
- Phase 1: ⚠️ In progress - software driver has `kx::std` namespace resolution issue with g++ 15.3.0
- Phase 2: ❌ Not started (requires KMS backend, DRM master)
- Phase 3: ❌ Not started
- Phase 4: ❌ Not started
- Phase 5: 📝 Documented only