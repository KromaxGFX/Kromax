# Kromax Software Driver

## Overview

The software driver is a CPU-only implementation of the Kromax API. It requires no GPU hardware and is ideal for:
- Development and testing on headless systems
- CI/CD pipelines
- Debugging rendering logic
- Learning graphics pipeline concepts

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Kromax API Layer                       │
│  (kxCreateInstance, kxCreateDevice, kxCmdDraw, etc.)       │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                    Driver Registry                          │
│         (Selects software/amdgpu/vulkan driver)            │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                  SoftwareDriver                             │
│  (Implements IDriverInstance, creates SoftwareDevice)      │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                  SoftwareDevice                             │
│  (Resource management, command execution)                  │
│  ┌─────────────┬─────────────┬─────────────┬─────────────┐  │
│  │  Memory     │  Buffer     │  Image      │  Pipeline   │  │
│  │  Manager    │  Manager    │  Manager    │  Manager    │  │
│  └─────────────┴─────────────┴─────────────┴─────────────┘  │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│              SoftwareRasterizer                             │
│  (Vertex processing, clipping, rasterization,               │
│   fragment shading, depth test, blending)                  │
└─────────────────────────────────────────────────────────────┘
```

## Module Structure

| File | Responsibility |
|------|----------------|
| `SoftwareDriver.h/cpp` | Driver entry point, instance creation |
| `SoftwareDevice.h/cpp` | Main device interface, resource registry |
| `SoftwareDeviceMemory.cpp` | Memory allocation/deallocation |
| `SoftwareDeviceBuffer.cpp` | Buffer creation/destruction |
| `SoftwareDeviceImage.cpp` | Image creation/destruction |
| `SoftwareDeviceRenderPass.cpp` | Render pass/framebuffer creation |
| `SoftwareDeviceCommand.cpp` | Command pool/buffer allocation |
| `SoftwareDeviceExecute.cpp` | Command execution, draw/dispatch |
| `SoftwareDevicePipeline.cpp` | Pipeline/layout/shader creation |
| `SoftwareCommandBuffer.h/cpp` | Command recording |
| `SoftwarePipeline.h/cpp` | Pipeline state object |
| `SoftwarePipelineLayout.h/cpp` | Pipeline layout (descriptors, push constants) |
| `SoftwareShaderModule.h/cpp` | Shader module (bytecode or callbacks) |
| `SoftwareRasterizer.h/cpp` | Software rasterizer implementation |
| `SoftwareMemory.h/cpp` | Memory object |
| `SoftwareBuffer.h/cpp` | Buffer object |
| `SoftwareImage.h/cpp` | Image object |
| `SoftwareImageView.h/cpp` | Image view object |
| `SoftwareRenderPass.h/cpp` | Render pass object |
| `SoftwareFramebuffer.h/cpp` | Framebuffer object |
| `SoftwareCommandPool.h/cpp` | Command pool object |

## Key Implementation Details

### Memory Management

All allocations use host memory (std::vector<uint8_t>):

```cpp
// SoftwareMemory holds a contiguous allocation
class SoftwareMemory {
    std::vector<uint8_t> data_;
    uint64_t size_;
    uint64_t offset_;
    bool mapped_;
    void* mapped_ptr_;
};
```

### Buffer/Image Binding

Buffers and images are bound to memory at creation time:

```cpp
// Bind buffer memory - maps buffer range to memory range
kxBindBufferMemory(device, buffer, memory, offset);

// Bind image memory - maps entire image to memory
kxBindImageMemory(device, image, memory, offset);
```

### Command Recording

Commands are recorded into a `std::vector<CommandVariant>`:

```cpp
using CommandVariant = std::variant<
    CmdClearImage,
    CmdCopyBuffer,
    CmdCopyBufferToImage,
    CmdBindPipeline,
    CmdBindVertexBuffers,
    CmdBindIndexBuffer,
    CmdDraw,
    CmdDrawIndexed,
    CmdDispatch,
    CmdBeginRenderPass,
    CmdEndRenderPass,
    CmdNextSubpass>;
```

### Command Execution

On `queue_submit`, commands are executed sequentially:

```cpp
for (const auto& command : cmd_buffer->commands()) {
    execute_command(command);
}
```

### Software Rasterizer

The rasterizer implements a basic triangle rasterization pipeline:

1. **Vertex Processing** - Run vertex shader (or pass-through)
2. **Perspective Divide** - Convert clip space to NDC
3. **Viewport Transform** - NDC to screen coordinates
4. **Backface Culling** - Remove back-facing triangles
5. **Clipping** - Clip against view frustum (simplified)
6. **Rasterization** - Generate fragments using edge functions
7. **Depth Test** - Compare fragment depth with depth buffer
8. **Fragment Shader** - Run fragment shader (or pass-through)
9. **Blending** - Blend with framebuffer (basic alpha blend)

### Vertex Format

```cpp
struct Vertex {
    float position[4];  // Clip space (x, y, z, w)
    float normal[3];    // Normal vector
    float texcoord[2];  // Texture coordinates
    float color[4];     // Vertex color (RGBA)
};
```

### Fragment Format

```cpp
struct Fragment {
    float x, y;         // Screen coordinates
    float depth;        // Interpolated depth [0, 1]
    float normal[3];    // Interpolated normal
    float texcoord[2];  // Interpolated texcoord
    float color[4];     // Interpolated color
};
```

## Pipeline State

Graphics pipeline state is stored in `KxPipelineCreateInfo`:

```cpp
// Key pipeline state
KxPrimitiveTopology primitiveTopology;  // Triangle, line, point
KxPolygonMode polygonMode;              // Fill, line, point
KxFrontFace frontFace;                  // CCW, CW
KxCullModeFlags cullMode;               // None, front, back, both
bool depthTestEnable;
bool depthWriteEnable;
KxCompareOp depthCompareOp;             // Never, less, equal, etc.
KxPipelineColorBlendStateCreateInfo* pColorBlendState;
```

## Shader Modules

Two shader formats supported:

### 1. Native Callback (KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT)

```cpp
KxShaderModuleNativeCallbackInfo native_info{};
native_info.vertex_fn = my_vertex_shader;
native_info.fragment_fn = my_fragment_shader;
native_info.compute_fn = nullptr;
native_info.push_constants = my_push_constants;
native_info.push_constants_size = sizeof(MyPushConstants);

KxShaderModuleCreateInfo create_info{};
create_info.format = KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT;
create_info.pCode = &native_info;
create_info.codeSize = sizeof(native_info);
```

### 2. SPIR-V (KX_SHADER_FORMAT_SPIRV)

```cpp
std::vector<uint32_t> spirv = read_spirv_file("shader.spv");

KxShaderModuleCreateInfo create_info{};
create_info.format = KX_SHADER_FORMAT_SPIRV;
create_info.pCode = spirv.data();
create_info.codeSize = spirv.size() * sizeof(uint32_t);
```

## Render Pass Execution

Render passes track:
- Active attachments (color, depth/stencil)
- Current subpass index
- Clear values
- Framebuffer dimensions

```cpp
// In execute_command for CmdBeginRenderPass:
in_render_pass_ = true;
current_subpass_ = 0;
active_render_pass_ = render_pass;
active_framebuffer_ = framebuffer;

// Clear attachments if needed
for (auto& clear_value : clear_values) {
    clear_attachment(attachment, clear_value);
}
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Memory alloc | O(1) | std::vector allocation |
| Buffer create | O(1) | Just registers metadata |
| Image create | O(1) | Allocates pixel buffer |
| Command record | O(1) | Pushes to vector |
| Draw call | O(vertices + fragments) | CPU rasterization |
| Clear | O(pixels) | Memset per attachment |

## Limitations

- No hardware acceleration
- Limited format support (RGBA8, D32_FLOAT primary)
- No texture sampling (storage images only)
- No compute shader dispatch (stub)
- Basic blending only (no advanced blend ops)
- No multisampling
- No geometry/tessellation shaders

## Extending the Driver

### Adding New Formats

1. Add format to `format_pixel_size()` in `SoftwareDeviceExecute.cpp`
2. Handle format in `SoftwareImage::data()` access
3. Update rasterizer for format-specific blending

### Adding New Commands

1. Add command struct to `SoftwareCommandBuffer.h`
2. Add to `CommandVariant` type
3. Add `record_*()` method to `SoftwareCommandBuffer`
4. Add execution case in `SoftwareDeviceExecute.cpp`

### Adding Shader Stages

1. Add callback type to `kx_core.h`
2. Add field to `SoftwareShaderModule`
3. Add execution path in `SoftwareRasterizer`

## Debugging Tips

### Enable Debug Output

```cpp
#define KX_DEBUG 1
#include <kromax/kx_core.h>

// Or set environment variable
export KROMAX_DEBUG=1
```

### Inspect Command Buffers

```cpp
auto* cmd_buffer = device->find_command_buffer(cmd);
for (const auto& command : cmd_buffer->commands()) {
    std::visit([](auto&& cmd) {
        std::cout << "Command: " << typeid(cmd).name() << "\n";
    }, command);
}
```

### Visualize Rasterization

Add debug output in `SoftwareRasterizer::draw_triangle()`:
```cpp
std::cout << "Drawing triangle: " << v0.position[0] << ", " 
          << v0.position[1] << " -> " << v1.position[0] << ", "
          << v1.position[1] << " -> " << v2.position[0] << ", "
          << v2.position[1] << "\n";
```

## Testing

Run software driver tests:

```bash
cd build-asan
# Unit tests
./bin/test_unit
./bin/test_buffer_image
./bin/test_commands
./bin/test_renderpass
./bin/test_api
./bin/test_integration

# Run examples
./bin/kromax_example_01_hello_kromax
./bin/kromax_example_02_clear_kms
./bin/kromax_example_03_triangle
./bin/kromax_example_04_compute
```

## Future Improvements

- [ ] Full SPIR-V interpreter
- [ ] Compute shader support
- [ ] Texture sampling
- [ ] Multisample anti-aliasing
- [ ] Geometry/tessellation shaders
- [ ] Async compute (thread pool)
- [ ] SIMD-optimized rasterizer
- [ ] Better clipping (Sutherland-Hodgman)
- [ ] Stencil operations
- [ ] Occlusion queries
- [ ] Transform feedback

## See Also

- [API Reference](API_REFERENCE.md)
- [Getting Started](GETTING_STARTED.md)
- [Architecture](ARCHITECTURE.md)
- [Source Code](../src/driver/software/)