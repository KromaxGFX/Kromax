# Kromax API Reference

## Overview

Kromax is a low-level graphics and compute API designed for explicit GPU control. This reference documents the core API surface.

## Core Concepts

### Object Model
- **Instance**: Top-level API context
- **PhysicalDevice**: GPU hardware representation
- **Device**: Logical device for resource creation and command submission
- **Queue**: Command execution interface
- **CommandPool/CommandBuffer**: Command recording and submission

### Memory Management
- **DeviceMemory**: Raw GPU memory allocation
- **Buffer**: Linear memory for vertex/index/uniform data
- **Image**: Multi-dimensional memory for textures/render targets
- **ImageView**: View into an image with format/swizzle/subresource

### Rendering
- **RenderPass**: Render pass structure with attachments/subpasses
- **Framebuffer**: Attachment bindings for render pass
- **PipelineLayout**: Descriptor set layouts and push constants
- **Pipeline**: Graphics/compute pipeline state
- **ShaderModule**: Shader bytecode or native callbacks

## Quick Reference Tables

### Result Codes

| Code | Value | Description |
|------|-------|-------------|
| KX_SUCCESS | 0 | Operation succeeded |
| KX_NOT_READY | 1 | Fence/semaphore not ready |
| KX_TIMEOUT | 2 | Wait operation timed out |
| KX_ERROR_OUT_OF_HOST_MEMORY | -1 | Host memory allocation failed |
| KX_ERROR_OUT_OF_DEVICE_MEMORY | -2 | Device memory allocation failed |
| KX_ERROR_INITIALIZATION_FAILED | -3 | Initialization failed |
| KX_ERROR_DEVICE_LOST | -4 | Device lost |
| KX_ERROR_FEATURE_NOT_PRESENT | -8 | Feature not supported |

### Format Enums (Common)

| Format | Description |
|--------|-------------|
| KX_FORMAT_R8_UNORM | 8-bit unsigned normalized R |
| KX_FORMAT_R8G8_UNORM | 16-bit unsigned normalized RG |
| KX_FORMAT_R8G8B8A8_UNORM | 32-bit unsigned normalized RGBA |
| KX_FORMAT_B8G8R8A8_UNORM | 32-bit unsigned normalized BGRA |
| KX_FORMAT_B8G8R8A8_SRGB | 32-bit sRGB BGRA |
| KX_FORMAT_R32_SFLOAT | 32-bit float R |
| KX_FORMAT_R32G32_SFLOAT | 64-bit float RG |
| KX_FORMAT_R32G32B32A32_SFLOAT | 128-bit float RGBA |
| KX_FORMAT_D16_UNORM | 16-bit depth |
| KX_FORMAT_D32_SFLOAT | 32-bit float depth |
| KX_FORMAT_D24_UNORM_S8_UINT | 24-bit depth + 8-bit stencil |

### Image Usage Flags

| Flag | Value | Description |
|------|-------|-------------|
| KX_IMAGE_USAGE_TRANSFER_SRC_BIT | 0x00000001 | Transfer source |
| KX_IMAGE_USAGE_TRANSFER_DST_BIT | 0x00000002 | Transfer destination |
| KX_IMAGE_USAGE_SAMPLED_BIT | 0x00000004 | Sampled in shader |
| KX_IMAGE_USAGE_STORAGE_BIT | 0x00000008 | Storage image |
| KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 0x00000010 | Color attachment |
| KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | 0x00000020 | Depth/stencil attachment |
| KX_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | 0x00000040 | Transient attachment |

### Buffer Usage Flags

| Flag | Value | Description |
|------|-------|-------------|
| KX_BUFFER_USAGE_TRANSFER_SRC_BIT | 0x00000001 | Transfer source |
| KX_BUFFER_USAGE_TRANSFER_DST_BIT | 0x00000002 | Transfer destination |
| KX_BUFFER_USAGE_UNIFORM_BUFFER_BIT | 0x00000004 | Uniform buffer |
| KX_BUFFER_USAGE_STORAGE_BUFFER_BIT | 0x00000008 | Storage buffer |
| KX_BUFFER_USAGE_INDEX_BUFFER_BIT | 0x00000010 | Index buffer |
| KX_BUFFER_USAGE_VERTEX_BUFFER_BIT | 0x00000020 | Vertex buffer |
| KX_BUFFER_USAGE_INDIRECT_BUFFER_BIT | 0x00000040 | Indirect buffer |

### Pipeline Stages

| Stage | Description |
|-------|-------------|
| KX_PIPELINE_STAGE_TOP_OF_PIPE_BIT | Before any stage |
| KX_PIPELINE_STAGE_VERTEX_SHADER_BIT | Vertex shader |
| KX_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | Fragment shader |
| KX_PIPELINE_STAGE_COMPUTE_SHADER_BIT | Compute shader |
| KX_PIPELINE_STAGE_TRANSFER_BIT | Transfer operations |
| KX_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | After all stages |

## Function Reference

### Instance

```c
KxResult kxCreateInstance(const KxInstanceCreateInfo* create_info,
                          const KxAllocationCallbacks* allocator,
                          KxInstance* instance);

void kxDestroyInstance(KxInstance instance,
                       const KxAllocationCallbacks* allocator);

KxResult kxEnumeratePhysicalDevices(KxInstance instance,
                                    uint32_t* physical_device_count,
                                    KxPhysicalDevice* physical_devices);
```

### Device

```c
KxResult kxCreateDevice(KxPhysicalDevice physical_device,
                        const KxDeviceCreateInfo* create_info,
                        const KxAllocationCallbacks* allocator,
                        KxDevice* device);

void kxDestroyDevice(KxDevice device,
                     const KxAllocationCallbacks* allocator);

KxResult kxGetDeviceQueue(KxDevice device,
                          uint32_t queue_family_index,
                          uint32_t queue_index,
                          KxQueue* queue);
```

### Memory

```c
KxResult kxAllocateMemory(KxDevice device,
                          const KxMemoryAllocateInfo* allocate_info,
                          const KxAllocationCallbacks* allocator,
                          KxDeviceMemory* memory);

void kxFreeMemory(KxDevice device,
                  KxDeviceMemory memory,
                  const KxAllocationCallbacks* allocator);

KxResult kxMapMemory(KxDevice device,
                     KxDeviceMemory memory,
                     uint64_t offset,
                     uint64_t size,
                     KxMemoryMapFlags flags,
                     void** pp_data);

void kxUnmapMemory(KxDevice device, KxDeviceMemory memory);

KxResult kxBindBufferMemory(KxDevice device,
                            KxBuffer buffer,
                            KxDeviceMemory memory,
                            uint64_t memory_offset);

KxResult kxBindImageMemory(KxDevice device,
                           KxImage image,
                           KxDeviceMemory memory,
                           uint64_t memory_offset);
```

### Buffers

```c
KxResult kxCreateBuffer(KxDevice device,
                        const KxBufferCreateInfo* create_info,
                        const KxAllocationCallbacks* allocator,
                        KxBuffer* buffer);

void kxDestroyBuffer(KxDevice device,
                     KxBuffer buffer,
                     const KxAllocationCallbacks* allocator);
```

### Images

```c
KxResult kxCreateImage(KxDevice device,
                       const KxImageCreateInfo* create_info,
                       const KxAllocationCallbacks* allocator,
                       KxImage* image);

void kxDestroyImage(KxDevice device,
                    KxImage image,
                    const KxAllocationCallbacks* allocator);

KxResult kxCreateImageView(KxDevice device,
                           const KxImageViewCreateInfo* create_info,
                           const KxAllocationCallbacks* allocator,
                           KxImageView* image_view);

void kxDestroyImageView(KxDevice device,
                        KxImageView image_view,
                        const KxAllocationCallbacks* allocator);
```

### Command Buffers

```c
KxResult kxCreateCommandPool(KxDevice device,
                             const KxCommandPoolCreateInfo* create_info,
                             const KxAllocationCallbacks* allocator,
                             KxCommandPool* command_pool);

void kxDestroyCommandPool(KxDevice device,
                          KxCommandPool command_pool,
                          const KxAllocationCallbacks* allocator);

KxResult kxAllocateCommandBuffers(KxDevice device,
                                  const KxCommandBufferAllocateInfo* allocate_info,
                                  KxCommandBuffer* command_buffers);

void kxFreeCommandBuffers(KxDevice device,
                          KxCommandPool command_pool,
                          uint32_t count,
                          const KxCommandBuffer* command_buffers);

KxResult kxBeginCommandBuffer(KxCommandBuffer command_buffer,
                              const KxCommandBufferBeginInfo* begin_info);

KxResult kxEndCommandBuffer(KxCommandBuffer command_buffer);
```

### Command Recording

```c
void kxCmdClearImage(KxCommandBuffer command_buffer,
                     KxImage image,
                     KxImageLayout layout,
                     const KxClearValue* color,
                     uint32_t range_count,
                     const KxImageSubresourceRange* ranges);

void kxCmdCopyBuffer(KxCommandBuffer command_buffer,
                     KxBuffer src_buffer,
                     KxBuffer dst_buffer,
                     uint32_t region_count,
                     const KxBufferCopy* regions);

void kxCmdCopyBufferToImage(KxCommandBuffer command_buffer,
                            KxBuffer src_buffer,
                            KxImage dst_image,
                            KxImageLayout dst_image_layout,
                            uint32_t region_count,
                            const KxBufferImageCopy* regions);

void kxCmdBindPipeline(KxCommandBuffer command_buffer,
                       KxPipelineBindPoint bind_point,
                       KxPipeline pipeline);

void kxCmdBindVertexBuffers(KxCommandBuffer command_buffer,
                            uint32_t first_binding,
                            uint32_t binding_count,
                            const KxBuffer* buffers,
                            const uint64_t* offsets);

void kxCmdBindIndexBuffer(KxCommandBuffer command_buffer,
                          KxBuffer buffer,
                          uint64_t offset,
                          KxIndexType index_type);

void kxCmdDraw(KxCommandBuffer command_buffer,
               uint32_t vertex_count,
               uint32_t instance_count,
               uint32_t first_vertex,
               uint32_t first_instance);

void kxCmdDrawIndexed(KxCommandBuffer command_buffer,
                      uint32_t index_count,
                      uint32_t instance_count,
                      uint32_t first_index,
                      int32_t vertex_offset,
                      uint32_t first_instance);

void kxCmdDispatch(KxCommandBuffer command_buffer,
                   uint32_t group_x,
                   uint32_t group_y,
                   uint32_t group_z);
```

### Render Pass

```c
KxResult kxCreateRenderPass(KxDevice device,
                            const KxRenderPassCreateInfo* create_info,
                            const KxAllocationCallbacks* allocator,
                            KxRenderPass* render_pass);

void kxDestroyRenderPass(KxDevice device,
                         KxRenderPass render_pass,
                         const KxAllocationCallbacks* allocator);

void kxCmdBeginRenderPass(KxCommandBuffer command_buffer,
                          const KxRenderPassBeginInfo* render_pass_begin,
                          KxSubpassContents contents);

void kxCmdNextSubpass(KxCommandBuffer command_buffer,
                      KxSubpassContents contents);

void kxCmdEndRenderPass(KxCommandBuffer command_buffer);
```

### Framebuffer

```c
KxResult kxCreateFramebuffer(KxDevice device,
                             const KxFramebufferCreateInfo* create_info,
                             const KxAllocationCallbacks* allocator,
                             KxFramebuffer* framebuffer);

void kxDestroyFramebuffer(KxDevice device,
                          KxFramebuffer framebuffer,
                          const KxAllocationCallbacks* allocator);
```

### Pipeline

```c
KxResult kxCreatePipelineLayout(KxDevice device,
                                const KxPipelineLayoutCreateInfo* create_info,
                                const KxAllocationCallbacks* allocator,
                                KxPipelineLayout* pipeline_layout);

void kxDestroyPipelineLayout(KxDevice device,
                             KxPipelineLayout pipeline_layout,
                             const KxAllocationCallbacks* allocator);

KxResult kxCreatePipeline(KxDevice device,
                          const KxPipelineCreateInfo* create_info,
                          const KxAllocationCallbacks* allocator,
                          KxPipeline* pipeline);

void kxDestroyPipeline(KxDevice device,
                       KxPipeline pipeline,
                       const KxAllocationCallbacks* allocator);
```

### Shader Module

```c
KxResult kxCreateShaderModule(KxDevice device,
                              const KxShaderModuleCreateInfo* create_info,
                              const KxAllocationCallbacks* allocator,
                              KxShaderModule* shader_module);

void kxDestroyShaderModule(KxDevice device,
                           KxShaderModule shader_module,
                           const KxAllocationCallbacks* allocator);
```

### Queue Operations

```c
KxResult kxQueueSubmit(KxQueue queue,
                       uint32_t submit_count,
                       const KxSubmitInfo* submits,
                       KxFence fence);

KxResult kxQueueWaitIdle(KxQueue queue);

KxResult kxDeviceWaitIdle(KxDevice device);
```

## Structures Reference

### KxInstanceCreateInfo
```c
typedef struct KxInstanceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxInstanceCreateFlags flags;
    const KxApplicationInfo* pApplicationInfo;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
} KxInstanceCreateInfo;
```

### KxDeviceCreateInfo
```c
typedef struct KxDeviceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxDeviceCreateFlags flags;
    uint32_t queueCreateInfoCount;
    const KxDeviceQueueCreateInfo* pQueueCreateInfos;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
    const KxPhysicalDeviceFeatures* pEnabledFeatures;
} KxDeviceCreateInfo;
```

### KxBufferCreateInfo
```c
typedef struct KxBufferCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBufferCreateFlags flags;
    uint64_t size;
    KxBufferUsageFlags usage;
    KxSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
} KxBufferCreateInfo;
```

### KxImageCreateInfo
```c
typedef struct KxImageCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxImageCreateFlags flags;
    KxImageType imageType;
    KxFormat format;
    KxExtent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    KxSampleCountFlagBits samples;
    KxImageTiling tiling;
    KxImageUsageFlags usage;
    KxSharingMode sharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxImageLayout initialLayout;
} KxImageCreateInfo;
```

### KxPipelineCreateInfo
```c
typedef struct KxPipelineCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineCreateFlags flags;
    KxPipelineBindPoint bindPoint;
    KxPipelineLayout layout;
    
    // Shader stages
    KxShaderModule vertexShaderModule;
    KxShaderModule fragmentShaderModule;
    KxShaderModule computeShaderModule;
    
    // Vertex input
    const KxPipelineVertexInputStateCreateInfo* pVertexInputState;
    
    // Input assembly
    KxPrimitiveTopology primitiveTopology;
    KxBool primitiveRestartEnable;
    
    // Viewport state
    uint32_t viewportCount;
    const KxViewport* pViewports;
    uint32_t scissorCount;
    const KxRect2D* pScissors;
    
    // Rasterization
    KxPolygonMode polygonMode;
    KxFrontFace frontFace;
    KxCullModeFlags cullMode;
    float lineWidth;
    
    // Depth/stencil
    KxBool depthTestEnable;
    KxBool depthWriteEnable;
    KxCompareOp depthCompareOp;
    KxBool depthBoundsTestEnable;
    KxBool stencilTestEnable;
    KxStencilOpState front;
    KxStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;
    
    // Multisample
    KxSampleCountFlagBits rasterizationSamples;
    KxBool sampleShadingEnable;
    float minSampleShading;
    const KxSampleMask* pSampleMask;
    KxBool alphaToCoverageEnable;
    KxBool alphaToOneEnable;
    
    // Color blend
    const KxPipelineColorBlendStateCreateInfo* pColorBlendState;
    
    // Dynamic state
    uint32_t dynamicStateCount;
    const KxDynamicState* pDynamicStates;
} KxPipelineCreateInfo;
```

## Error Handling

All functions returning `KxResult` must be checked. Common patterns:

```c
KxResult result = kxCreateInstance(&create_info, NULL, &instance);
if (result != KX_SUCCESS) {
    // Handle error
    return result;
}
```

## Version Macros

```c
#define KX_API_VERSION_MAJOR 1
#define KX_API_VERSION_MINOR 0
#define KX_API_VERSION_PATCH 0
#define KX_API_VERSION (KX_API_VERSION_MAJOR << 22 | KX_API_VERSION_MINOR << 12 | KX_API_VERSION_PATCH)
```

## Extensions

- `KX_EXT_DEBUG_UTILS` - Debug utilities
- `KX_EXT_SWAPCHAIN` - Swapchain support
- `KX_EXT_SHADER_NATIVE_CALLBACK` - Native callback shaders

## Limits

| Limit | Value |
|-------|-------|
| Max vertex bindings | 16 |
| Max vertex attributes | 16 |
| Max color attachments | 8 |
| Max descriptor sets | 8 |
| Max push constant size | 256 bytes |
| Max image dimension 2D | 8192 |
| Max image dimension 3D | 2048 |
| Max image array layers | 2048 |

## See Also

- [Getting Started](GETTING_STARTED.md)
- [Software Driver](SOFTWARE_DRIVER.md)
- [Examples](../examples/)