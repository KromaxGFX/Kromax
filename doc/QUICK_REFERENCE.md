# Kromax Quick Reference Card

## Essential Structures

### Instance Creation
```c
KxInstanceCreateInfo {
    KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    NULL,
    0,
    &app_info,
    0, NULL,
    0, NULL
}
```

### Device Creation
```c
KxDeviceCreateInfo {
    KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    NULL,
    0,
    1, &queue_create_info,
    0, NULL,
    0, NULL,
    NULL
}
```

### Buffer Creation
```c
KxBufferCreateInfo {
    KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    NULL,
    0,
    size,
    KX_BUFFER_USAGE_VERTEX_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT,
    KX_SHARING_MODE_EXCLUSIVE,
    0, NULL
}
```

### Image Creation (2D Texture)
```c
KxImageCreateInfo {
    KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    NULL,
    0,
    KX_IMAGE_TYPE_2D,
    KX_FORMAT_R8G8B8A8_UNORM,
    {width, height, 1},
    1, 1,
    KX_SAMPLE_COUNT_1_BIT,
    KX_IMAGE_TILING_OPTIMAL,
    KX_IMAGE_USAGE_SAMPLED_BIT | KX_IMAGE_USAGE_TRANSFER_DST_BIT,
    KX_SHARING_MODE_EXCLUSIVE,
    0, NULL,
    KX_IMAGE_LAYOUT_UNDEFINED
}
```

### Render Pass (Single Subpass, Color + Depth)
```c
// Attachments
KxAttachmentDescription color_att = {
    0, format, KX_SAMPLE_COUNT_1_BIT,
    KX_ATTACHMENT_LOAD_OP_CLEAR, KX_ATTACHMENT_STORE_OP_STORE,
    KX_ATTACHMENT_LOAD_OP_DONT_CARE, KX_ATTACHMENT_STORE_OP_DONT_CARE,
    KX_IMAGE_LAYOUT_UNDEFINED, KX_IMAGE_LAYOUT_PRESENT_SRC_KHR
};

KxAttachmentDescription depth_att = {
    0, depth_format, KX_SAMPLE_COUNT_1_BIT,
    KX_ATTACHMENT_LOAD_OP_CLEAR, KX_ATTACHMENT_STORE_OP_DONT_CARE,
    KX_ATTACHMENT_LOAD_OP_DONT_CARE, KX_ATTACHMENT_STORE_OP_DONT_CARE,
    KX_IMAGE_LAYOUT_UNDEFINED, KX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
};

// Subpass
KxAttachmentReference color_ref = {0, KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
KxAttachmentReference depth_ref = {1, KX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

KxSubpassDescription subpass = {
    0, KX_PIPELINE_BIND_POINT_GRAPHICS,
    0, NULL,
    1, &color_ref,
    NULL,
    &depth_ref,
    0, NULL
};

// Dependency
KxSubpassDependency dep = {
    KX_SUBPASS_EXTERNAL, 0,
    KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    0,
    KX_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    0
};

KxRenderPassCreateInfo rp_info = {
    KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    NULL, 0,
    2, attachments,
    1, &subpass,
    1, &dep
};
```

### Pipeline (Graphics)
```c
KxPipelineCreateInfo pipeline_info = {
    KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO,
    NULL, 0,
    KX_PIPELINE_BIND_POINT_GRAPHICS,
    pipeline_layout,
    vertex_shader, fragment_shader, NULL,
    &vertex_input_state,
    KX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false,
    1, &viewport, 1, &scissor,
    KX_POLYGON_MODE_FILL, KX_FRONT_FACE_COUNTER_CLOCKWISE,
    KX_CULL_MODE_BACK_BIT, 1.0f,
    true, true, KX_COMPARE_OP_LESS, false,
    false, {0}, {0}, 0.0f, 1.0f,
    KX_SAMPLE_COUNT_1_BIT, false, 0.0f, NULL, false, false,
    &color_blend_state,
    0, NULL
};
```

### Command Buffer Begin
```c
KxCommandBufferBeginInfo {
    KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    NULL,
    KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    NULL
}
```

### Render Pass Begin
```c
KxRenderPassBeginInfo {
    KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    NULL,
    render_pass,
    framebuffer,
    {{0, 0}, {width, height}},
    2, clear_values
}
```

## Common Command Sequences

### Clear Color Attachment
```c
kxCmdBeginRenderPass(cmd, &rp_begin, KX_SUBPASS_CONTENTS_INLINE);
// ... draw commands ...
kxCmdEndRenderPass(cmd);
```

### Draw Triangle
```c
kxCmdBindPipeline(cmd, KX_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
kxCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer, &offset);
kxCmdDraw(cmd, 3, 1, 0, 0);
```

### Draw Indexed
```c
kxCmdBindPipeline(cmd, KX_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
kxCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer, &offset);
kxCmdBindIndexBuffer(cmd, index_buffer, 0, KX_INDEX_TYPE_UINT32);
kxCmdDrawIndexed(cmd, index_count, 1, 0, 0, 0);
```

### Copy Buffer to Image
```c
KxBufferImageCopy region = {
    0, 0, 0,
    {KX_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
    {0, 0, 0},
    {width, height, 1}
};
kxCmdCopyBufferToImage(cmd, src_buffer, dst_image,
                       KX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region);
```

## Memory Management

### Allocate and Bind Buffer
```c
// 1. Create buffer
kxCreateBuffer(device, &buf_info, NULL, &buffer);

// 2. Get requirements
KxMemoryRequirements mem_req;
kxGetBufferMemoryRequirements(device, buffer, &mem_req);

// 3. Find memory type
uint32_t mem_type = find_memory_type(mem_req.memoryTypeBits, 
    KX_MEMORY_PROPERTY_HOST_VISIBLE_BIT | KX_MEMORY_PROPERTY_HOST_COHERENT_BIT);

// 4. Allocate memory
KxMemoryAllocateInfo alloc_info = {
    KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL,
    mem_req.size, mem_type
};
kxAllocateMemory(device, &alloc_info, NULL, &memory);

// 5. Bind
kxBindBufferMemory(device, buffer, memory, 0);

// 6. Map and write
void* mapped;
kxMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mapped);
memcpy(mapped, data, size);
kxUnmapMemory(device, memory);
```

## Shader Module (Native Callback)

```c
// Callbacks
void vertex_shader(const KxVertexInput* in, KxVertexOutput* out, const void* push) {
    out->position[0] = in->attributes[0].data[0];
    out->position[1] = in->attributes[0].data[1];
    out->position[2] = 0.0f;
    out->position[3] = 1.0f;
    out->color[0] = in->attributes[1].data[0];
    out->color[1] = in->attributes[1].data[1];
    out->color[2] = in->attributes[1].data[2];
    out->color[3] = in->attributes[1].data[3];
}

void fragment_shader(const KxFragmentInput* in, KxFragmentOutput* out, const void* push) {
    out->color[0] = in->color[0];
    out->color[1] = in->color[1];
    out->color[2] = in->color[2];
    out->color[3] = in->color[3];
}

// Create module
KxShaderModuleNativeCallbackInfo native = {
    vertex_shader, fragment_shader, NULL, NULL, 0
};

KxShaderModuleCreateInfo shader_info = {
    KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, NULL, 0,
    KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT,
    sizeof(native), &native
};

kxCreateShaderModule(device, &shader_info, NULL, &shader_module);
```

## Synchronization

### Submit with Semaphores
```c
KxPipelineStageFlags wait_stages[] = {KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

KxSubmitInfo submit = {
    KX_STRUCTURE_TYPE_SUBMIT_INFO, NULL,
    1, &image_acquired_semaphore, wait_stages,
    1, &cmd_buffer,
    1, &render_finished_semaphore
};

kxQueueSubmit(queue, 1, &submit, fence);
```

### Wait for Fence
```c
kxWaitForFences(device, 1, &fence, true, UINT64_MAX);
kxResetFences(device, 1, &fence);
```

## Common Enums

### Image Layouts
| Layout | Use Case |
|--------|----------|
| UNDEFINED | Initial, content discarded |
| GENERAL | Any usage |
| COLOR_ATTACHMENT_OPTIMAL | Color render target |
| DEPTH_STENCIL_ATTACHMENT_OPTIMAL | Depth/stencil render target |
| SHADER_READ_ONLY_OPTIMAL | Sampled in shader |
| TRANSFER_SRC_OPTIMAL | Transfer source |
| TRANSFER_DST_OPTIMAL | Transfer destination |
| PRESENT_SRC_KHR | Presentation |

### Access Flags
| Flag | Stage |
|------|-------|
| INDIRECT_COMMAND_READ_BIT | Draw indirect |
| INDEX_READ_BIT | Index buffer |
| VERTEX_ATTRIBUTE_READ_BIT | Vertex buffer |
| UNIFORM_READ_BIT | Uniform buffer |
| SHADER_READ_BIT | Sampled/storage |
| SHADER_WRITE_BIT | Storage image/buffer |
| COLOR_ATTACHMENT_READ_BIT | Blend read |
| COLOR_ATTACHMENT_WRITE_BIT | Color output |
| DEPTH_STENCIL_ATTACHMENT_READ_BIT | Depth test |
| DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | Depth write |
| TRANSFER_READ_BIT | Copy source |
| TRANSFER_WRITE_BIT | Copy dest |
| HOST_READ_BIT | CPU read |
| HOST_WRITE_BIT | CPU write |

### Pipeline Stages
| Stage | Description |
|-------|-------------|
| TOP_OF_PIPE_BIT | Before any work |
| VERTEX_SHADER_BIT | Vertex processing |
| FRAGMENT_SHADER_BIT | Fragment processing |
| COMPUTE_SHADER_BIT | Compute dispatch |
| TRANSFER_BIT | Copy operations |
| BOTTOM_OF_PIPE_BIT | After all work |
| COLOR_ATTACHMENT_OUTPUT_BIT | Blend/write color |
| EARLY_FRAGMENT_TESTS_BIT | Depth test before FS |
| LATE_FRAGMENT_TESTS_BIT | Depth test after FS |

## Error Codes Quick Check

```c
// Always check!
KxResult result = kxCreateBuffer(device, &info, NULL, &buffer);
if (result != KX_SUCCESS) {
    switch (result) {
        case KX_ERROR_OUT_OF_HOST_MEMORY:    // OOM host
        case KX_ERROR_OUT_OF_DEVICE_MEMORY:  // OOM device
        case KX_ERROR_INVALID_ARGUMENT:      // Bad parameter
        case KX_ERROR_FEATURE_NOT_PRESENT:   // Unsupported
        default:                             // Unknown
    }
}
```

## Utility Macros

```c
#define KX_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define KX_OFFSET_OF(type, member) ((size_t)&((type*)0)->member)
#define KX_ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
```

## File Locations

| File | Purpose |
|------|---------|
| `include/kromax/kx_core.h` | Main API header |
| `include/kromax/kx_platform.h` | Platform types |
| `src/api/api_*.cpp` | API dispatch |
| `src/driver/software/*.cpp` | Software driver |
| `examples/*/main.cpp` | Usage examples |
| `tests/unit/*.cpp` | Unit tests |

## Environment Variables

| Variable | Effect |
|----------|--------|
| `KROMAX_DRIVER=software` | Force software driver |
| `KROMAX_DRIVER=amdgpu` | Force AMDGPU driver |
| `KROMAX_DEBUG=1` | Enable debug output |
| `KROMAX_VALIDATION=1` | Enable validation layer |

## Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build -j$(nproc)

# Test
cd build && ctest --output-on-failure

# Run example
./build/bin/kromax_example_03_triangle
```