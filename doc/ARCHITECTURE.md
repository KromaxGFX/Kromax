# Kromax Architecture

## Design Philosophy

Kromax follows these core principles:

1. **Explicit Control** - Application manages all resources, synchronization, and memory
2. **Minimal Abstraction** - Thin layer over hardware/driver capabilities
3. **Portability** - Same API works across GPU vendors and software implementations
4. **Performance** - Zero-overhead abstractions, predictable behavior
5. **Safety** - Strong typing, validation layers, explicit lifetimes

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application                              │
└──────────────────────────────┬──────────────────────────────────┘
                               │ Kromax API (kx_*)
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                      API Dispatch Layer                         │
│  (api_instance.cpp, api_device.cpp, api_command.cpp,           │
│   api_renderpass.cpp, api_pipeline.cpp)                        │
│  - Parameter validation                                         │
│  - Handle translation                                           │
│  - Dispatch to driver                                           │
└──────────────────────────────┬──────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                      Driver Registry                            │
│  (DriverRegistry.h/cpp)                                        │
│  - Driver discovery/loading                                     │
│  - Instance/device creation routing                             │
└──────────────────────────────┬──────────────────────────────────┘
                               │
        ┌──────────────────────┼──────────────────────┐
        ▼                      ▼                      ▼
┌───────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Software    │    │     AMDGPU      │    │    Vulkan       │
│   Driver      │    │    Driver       │    │    Driver       │
│  (CPU only)   │    │  (HW accel)     │    │  (HW accel)     │
└───────────────┘    └─────────────────┘    └─────────────────┘
```

## Core Components

### 1. API Dispatch Layer (`src/api/`)

Thin wrapper that validates parameters and forwards to driver:

```cpp
// api_device.cpp
KX_API KxResult kxCreateDevice(KxPhysicalDevice physical_device,
                               const KxDeviceCreateInfo* create_info,
                               const KxAllocationCallbacks* allocator,
                               KxDevice* device) {
    // Validate parameters
    if (!physical_device || !create_info || !device)
        return KX_ERROR_INVALID_ARGUMENT;
    
    // Get driver instance
    auto* driver = get_driver(physical_device);
    if (!driver) return KX_ERROR_DEVICE_LOST;
    
    // Forward to driver
    return driver->create_device(physical_device, create_info, allocator, device);
}
```

### 2. Driver Interface (`src/driver/`)

Abstract interfaces implemented by each driver:

```cpp
// IDriverInstance.h
class IDriverInstance {
public:
    virtual ~IDriverInstance() = default;
    virtual KxResult enumerate_physical_devices(uint32_t* count, KxPhysicalDevice* devices) = 0;
    virtual KxResult get_physical_device_properties(KxPhysicalDevice device, KxPhysicalDeviceProperties* props) = 0;
    virtual KxResult get_physical_device_features(KxPhysicalDevice device, KxPhysicalDeviceFeatures* features) = 0;
    virtual KxResult get_physical_device_queue_family_properties(KxPhysicalDevice device, uint32_t* count, KxQueueFamilyProperties* props) = 0;
    virtual KxResult create_device(KxPhysicalDevice physical_device, const KxDeviceCreateInfo* create_info, const KxAllocationCallbacks* allocator, KxDevice* device) = 0;
};

// IDriverDevice.h
class IDriverDevice {
public:
    virtual ~IDriverDevice() = default;
    virtual KxResult allocate_memory(...) = 0;
    virtual void free_memory(...) = 0;
    virtual KxResult create_buffer(...) = 0;
    virtual void destroy_buffer(...) = 0;
    virtual KxResult create_image(...) = 0;
    virtual void destroy_image(...) = 0;
    // ... all resource creation/destruction
    virtual KxResult queue_submit(...) = 0;
    virtual KxResult device_wait_idle() = 0;
};
```

### 3. Object Model

All Kromax objects are opaque handles (pointers to driver-internal objects):

```cpp
// Opaque handle types
typedef struct KxInstance_T* KxInstance;
typedef struct KxPhysicalDevice_T* KxPhysicalDevice;
typedef struct KxDevice_T* KxDevice;
typedef struct KxQueue_T* KxQueue;
typedef struct KxBuffer_T* KxBuffer;
typedef struct KxImage_T* KxImage;
// ... etc
```

**Lifetime Rules:**
- Parent owns children (Instance → PhysicalDevice → Device → Resources)
- Explicit destroy required (no refcounting)
- Allocator callbacks for custom memory management

### 4. Command Recording Model

Commands are recorded into command buffers, then submitted to queues:

```
CommandPool → allocates → CommandBuffer(s)
                │
                ▼
        Record Commands
        (kxCmd*)
                │
                ▼
        kxEndCommandBuffer
                │
                ▼
        Queue Submit
        (kxQueueSubmit)
                │
                ▼
        Driver executes
```

**Command Buffer Levels:**
- `PRIMARY` - Can be submitted directly, can call secondary
- `SECONDARY` - Called from primary, inherits state

## Memory Model

### Memory Types

| Type | Description |
|------|-------------|
| DEVICE_LOCAL | GPU VRAM (fast device access) |
| HOST_VISIBLE | CPU accessible (mappable) |
| HOST_COHERENT | Host writes visible to device without flush |
| HOST_CACHED | Cached on host side |

### Allocation Flow

```
kxAllocateMemory
       │
       ▼
kxGetBufferMemoryRequirements / kxGetImageMemoryRequirements
       │
       ▼
Find compatible memory type (device_local + host_visible preferred)
       │
       ▼
Driver allocates memory block
       │
       ▼
kxBindBufferMemory / kxBindImageMemory
```

### Buffer/Image Memory Binding

- Buffers: Can bind at any offset within memory
- Images: Must bind at offset meeting alignment requirements
- Sparse binding: Not yet supported

## Synchronization

### Fences

```cpp
// CPU-GPU synchronization
KxFenceCreateInfo fence_info{};
fence_info.sType = KX_STRUCTURE_TYPE_FENCE_CREATE_INFO;
fence_info.flags = KX_FENCE_CREATE_SIGNALED_BIT;

KxFence fence;
kxCreateFence(device, &fence_info, nullptr, &fence);

// Wait on CPU
kxWaitForFences(device, 1, &fence, true, UINT64_MAX);

// Reset for reuse
kxResetFences(device, 1, &fence);
```

### Semaphores

```cpp
// GPU-GPU synchronization
KxSemaphoreCreateInfo sem_info{};
sem_info.sType = KX_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

KxSemaphore semaphore;
kxCreateSemaphore(device, &sem_info, nullptr, &semaphore);

// In submit info:
KxSubmitInfo submit{};
submit.waitSemaphoreCount = 1;
submit.pWaitSemaphores = &semaphore;
submit.pWaitDstStageMask = &stage_mask;  // Pipeline stage to wait at
submit.signalSemaphoreCount = 1;
submit.pSignalSemaphores = &semaphore;
```

### Events

```cpp
// Fine-grained GPU synchronization
KxEventCreateInfo event_info{};
event_info.sType = KX_STRUCTURE_TYPE_EVENT_CREATE_INFO;

KxEvent event;
kxCreateEvent(device, &event_info, nullptr, &event);

// In command buffer:
kxCmdSetEvent(command_buffer, event, KX_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
kxCmdWaitEvents(command_buffer, 1, &event, 
                KX_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                KX_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                0, nullptr, 0, nullptr, 0, nullptr);
```

## Pipeline Architecture

### Graphics Pipeline

```
Vertex Input → Vertex Shader → Tessellation → Geometry Shader
                    │
                    ▼
            Primitive Assembly
                    │
                    ▼
            Rasterization
                    │
                    ▼
            Fragment Shader
                    │
                    ▼
            Per-Fragment Ops (Depth, Stencil, Blend)
                    │
                    ▼
            Framebuffer
```

### Compute Pipeline

```
Dispatch → Compute Shader → Storage Buffer/Image Output
```

### Pipeline State Objects

Monolithic pipeline state (like Vulkan):
- All state baked at creation time
- No dynamic state changes (except dynamic state subset)
- Immutable after creation

## Resource Binding Model

### Descriptor Sets (Not Yet Implemented)

Future: Descriptor set layout → Pipeline layout → Descriptor pool → Descriptor sets

### Push Constants

```cpp
// In pipeline layout
KxPushConstantRange push_range{};
push_range.stageFlags = KX_SHADER_STAGE_VERTEX_BIT | KX_SHADER_STAGE_FRAGMENT_BIT;
push_range.offset = 0;
push_range.size = sizeof(MyPushConstants);

KxPipelineLayoutCreateInfo layout_info{};
layout_info.pushConstantRangeCount = 1;
layout_info.pPushConstantRanges = &push_range;

// In command buffer:
kxCmdPushConstants(command_buffer, layout, 
                   KX_SHADER_STAGE_VERTEX_BIT | KX_SHADER_STAGE_FRAGMENT_BIT,
                   0, sizeof(MyPushConstants), &push_constants);
```

## Extension System

Extensions add new functionality:

```c
// Extension structure
typedef struct KxExtensionProperties {
    char extensionName[KX_MAX_EXTENSION_NAME_SIZE];
    uint32_t specVersion;
} KxExtensionProperties;

// Enable at instance/device creation
const char* extensions[] = {
    "KX_EXT_DEBUG_UTILS",
    "KX_EXT_SWAPCHAIN",
    "KX_EXT_SHADER_NATIVE_CALLBACK"
};

create_info.enabledExtensionCount = 3;
create_info.ppEnabledExtensionNames = extensions;
```

### Current Extensions

| Extension | Description |
|-----------|-------------|
| KX_EXT_DEBUG_UTILS | Debug labels, object names, messengers |
| KX_EXT_SWAPCHAIN | Presentation to window system |
| KX_EXT_SHADER_NATIVE_CALLBACK | Native C++ shader callbacks |

## Error Handling

### Result Codes

All functions return `KxResult`:
- `>= 0`: Success codes
- `< 0`: Error codes

### Validation Layers

Enable debug layer for development:
```cpp
create_info.enabledLayerCount = 1;
create_info.ppEnabledLayerNames = (const char*[]){"KX_LAYER_VALIDATION"};
```

### Debug Utils

```cpp
// Object naming
kxSetDebugUtilsObjectName(device, &(KxDebugUtilsObjectNameInfo){
    .sType = KX_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO,
    .objectType = KX_OBJECT_TYPE_BUFFER,
    .objectHandle = (uint64_t)buffer,
    .pObjectName = "Vertex Buffer"
});

// Labels
kxCmdBeginDebugUtilsLabel(command_buffer, &(KxDebugUtilsLabel){
    .pLabelName = "Render Scene",
    .color = {1.0f, 0.0f, 0.0f, 1.0f}
});
// ... commands ...
kxCmdEndDebugUtilsLabel(command_buffer);
```

## Threading Model

- **Instance/Device creation**: Single-threaded
- **Command recording**: Thread-safe per command buffer
- **Queue submission**: Thread-safe (externally synchronized)
- **Resource creation**: Thread-safe (externally synchronized)
- **Memory mapping**: Not thread-safe for same memory object

## Versioning

```c
// API version
#define KX_API_VERSION_1_0 (1 << 22 | 0 << 12 | 0)

// Runtime version query
uint32_t api_version;
kxEnumerateInstanceVersion(&api_version);
```

## Platform Abstraction

### Platform Headers

```c
// kx_platform.h - Platform-specific types
#if defined(_WIN32)
    typedef HANDLE KxNativeHandle;
    typedef HWND KxWindow;
#elif defined(__linux__)
    typedef int KxNativeHandle;
    typedef Window KxWindow;
#elif defined(__APPLE__)
    typedef void* KxNativeHandle;
    typedef void* KxWindow;
#endif
```

### Surface/Window Integration

Swapchain extension provides platform-specific surface creation:
```cpp
// Linux/Wayland
KxSurfaceCreateInfoKHR surface_info{};
surface_info.sType = KX_STRUCTURE_TYPE_SURFACE_CREATE_INFO_KHR;
surface_info.display = wl_display;
surface_info.surface = wl_surface;

// Windows
surface_info.hwnd = hwnd;
surface_info.hinstance = hinstance;
```

## Testing Strategy

### Unit Tests
- API parameter validation
- Object lifecycle
- Memory management
- Command recording

### Integration Tests
- Full render pass execution
- Pipeline creation and draw
- Multi-threaded command recording
- Synchronization primitives

### Conformance
- Test against specification
- Compare software vs hardware drivers

## Build System

```
CMakeLists.txt
├── kromax (shared library)
│   ├── include/kromax/kx_core.h
│   ├── src/api/*.cpp
│   ├── src/driver/*.cpp
│   └── src/driver/software/*.cpp
├── examples/
│   ├── 01_hello_kromax
│   ├── 02_clear_kms
│   ├── 03_triangle
│   └── 04_compute
└── tests/
    ├── unit/
    └── integration/
```

## Code Style

- C++17
- No exceptions (use error codes)
- RAII for internal objects
- `snake_case` for C API, `PascalCase` for C++ internals
- `KX_` prefix for all public symbols
- Doxygen-style comments for public API

## Future Directions

1. **Vulkan Backend** - Wrap Vulkan for hardware acceleration
2. **Metal Backend** - macOS/iOS support
3. **DirectX 12 Backend** - Windows native
4. **Swapchain/Presentation** - Window system integration
5. **Ray Tracing** - RTX/DXR support
6. **Mesh Shaders** - Modern geometry pipeline
7. **Shader Compiler** - SPIR-V → native codegen

## See Also

- [API Reference](API_REFERENCE.md)
- [Software Driver](SOFTWARE_DRIVER.md)
- [Getting Started](GETTING_STARTED.md)
- [Source Code](../src/)