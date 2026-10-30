# Kromax Getting Started Guide

## Introduction

Kromax is a low-level graphics and compute API similar to Vulkan but with a simpler design. This guide will help you create your first Kromax application.

## Prerequisites

- C++17 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- Linux/macOS/Windows

## Building Kromax

```bash
git clone https://github.com/yourorg/Kromax.git
cd Kromax
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Your First Application

### 1. Create Instance

```cpp
#include <kromax/kx_core.h>

int main() {
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Kromax";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = 1;
    app_info.apiVersion = KX_API_VERSION;

    KxInstanceCreateInfo create_info{};
    create_info.sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    KxInstance instance;
    KxResult result = kxCreateInstance(&create_info, nullptr, &instance);
    if (result != KX_SUCCESS) {
        return -1;
    }

    // ... use instance

    kxDestroyInstance(instance, nullptr);
    return 0;
}
```

### 2. Select Physical Device

```cpp
uint32_t device_count = 0;
kxEnumeratePhysicalDevices(instance, &device_count, nullptr);

std::vector<KxPhysicalDevice> devices(device_count);
kxEnumeratePhysicalDevices(instance, &device_count, devices.data());

// Use first device for simplicity
KxPhysicalDevice physical_device = devices[0];
```

### 3. Create Logical Device

```cpp
// Find graphics queue family
uint32_t queue_family_count = 0;
kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

std::vector<KxQueueFamilyProperties> queue_families(queue_family_count);
kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

uint32_t graphics_queue_family = UINT32_MAX;
for (uint32_t i = 0; i < queue_family_count; ++i) {
    if (queue_families[i].queueFlags & KX_QUEUE_GRAPHICS_BIT) {
        graphics_queue_family = i;
        break;
    }
}

float queue_priority = 1.0f;
KxDeviceQueueCreateInfo queue_create_info{};
queue_create_info.sType = KX_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
queue_create_info.queueFamilyIndex = graphics_queue_family;
queue_create_info.queueCount = 1;
queue_create_info.pQueuePriorities = &queue_priority;

KxDeviceCreateInfo device_create_info{};
device_create_info.sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
device_create_info.queueCreateInfoCount = 1;
device_create_info.pQueueCreateInfos = &queue_create_info;

KxDevice device;
result = kxCreateDevice(physical_device, &device_create_info, nullptr, &device);
```

### 4. Get Queue

```cpp
KxQueue queue;
kxGetDeviceQueue(device, graphics_queue_family, 0, &queue);
```

### 5. Create Command Pool and Buffer

```cpp
KxCommandPoolCreateInfo pool_info{};
pool_info.sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
pool_info.queueFamilyIndex = graphics_queue_family;
pool_info.flags = KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

KxCommandPool command_pool;
kxCreateCommandPool(device, &pool_info, nullptr, &command_pool);

KxCommandBufferAllocateInfo alloc_info{};
alloc_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
alloc_info.commandPool = command_pool;
alloc_info.level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
alloc_info.commandBufferCount = 1;

KxCommandBuffer command_buffer;
kxAllocateCommandBuffers(device, &alloc_info, &command_buffer);
```

### 6. Record Commands

```cpp
KxCommandBufferBeginInfo begin_info{};
begin_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
begin_info.flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

kxBeginCommandBuffer(command_buffer, &begin_info);

// Record commands here...

kxEndCommandBuffer(command_buffer);
```

### 7. Submit and Wait

```cpp
KxSubmitInfo submit_info{};
submit_info.sType = KX_STRUCTURE_TYPE_SUBMIT_INFO;
submit_info.commandBufferCount = 1;
submit_info.pCommandBuffers = &command_buffer;

kxQueueSubmit(queue, 1, &submit_info, nullptr);
kxQueueWaitIdle(queue);
```

### 8. Cleanup

```cpp
kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
kxDestroyCommandPool(device, command_pool, nullptr);
kxDestroyDevice(device, nullptr);
kxDestroyInstance(instance, nullptr);
```

## Complete Example: Clear Screen

See `examples/01_hello_kromax/main.cpp` for a complete working example.

## Using the Software Driver

The software driver runs entirely on CPU - no GPU required:

```bash
# Set environment variable to use software driver
export KROMAX_DRIVER=software
./your_application
```

Or programmatically:
```cpp
// The software driver is selected automatically if no GPU driver is available
```

## Shader Development

### Native Callback Shaders (Extension)

```cpp
void my_vertex_shader(const KxVertexInput* input,
                      KxVertexOutput* output,
                      const void* push_constants) {
    // Simple pass-through vertex shader
    output->position[0] = input->attributes[0].data[0]; // x
    output->position[1] = input->attributes[0].data[1]; // y
    output->position[2] = 0.0f;                          // z
    output->position[3] = 1.0f;                          // w
    output->color[0] = input->attributes[1].data[0];     // r
    output->color[1] = input->attributes[1].data[1];     // g
    output->color[2] = input->attributes[1].data[2];     // b
    output->color[3] = input->attributes[1].data[3];     // a
}

void my_fragment_shader(const KxFragmentInput* input,
                        KxFragmentOutput* output,
                        const void* push_constants) {
    // Output interpolated color
    output->color[0] = input->color[0];
    output->color[1] = input->color[1];
    output->color[2] = input->color[2];
    output->color[3] = input->color[3];
}
```

### Creating Shader Modules

```cpp
KxShaderModuleNativeCallbackInfo native_info{};
native_info.vertex_fn = my_vertex_shader;
native_info.fragment_fn = my_fragment_shader;
native_info.compute_fn = nullptr;
native_info.push_constants = nullptr;
native_info.push_constants_size = 0;

KxShaderModuleCreateInfo shader_info{};
shader_info.sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
shader_info.format = KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT;
shader_info.codeSize = sizeof(native_info);
shader_info.pCode = &native_info;

KxShaderModule shader_module;
kxCreateShaderModule(device, &shader_info, nullptr, &shader_module);
```

## Building and Running Examples

```bash
cd build
# Run hello world
./bin/kromax_example_01_hello_kromax

# Run clear example (requires display)
./bin/kromax_example_02_clear_kms

# Run triangle example
./bin/kromax_example_03_triangle

# Run compute example
./bin/kromax_example_04_compute
```

## Common Patterns

### Error Checking Macro

```cpp
#define KX_CHECK(call) \
    do { \
        KxResult result = (call); \
        if (result != KX_SUCCESS) { \
            fprintf(stderr, "Kromax error at %s:%d: %d\n", __FILE__, __LINE__, result); \
            return result; \
        } \
    } while(0)
```

### Resource Cleanup Helper

```cpp
struct Deleter {
    KxDevice device;
    const KxAllocationCallbacks* allocator;
    
    void operator()(KxBuffer buffer) { kxDestroyBuffer(device, buffer, allocator); }
    void operator()(KxImage image) { kxDestroyImage(device, image, allocator); }
    // ... etc
};
```

## Next Steps

1. Read [API Reference](API_REFERENCE.md) for complete function list
2. Study [Software Driver](SOFTWARE_DRIVER.md) internals
3. Explore [examples](../examples/) for more complex usage
3. Check [Architecture](ARCHITECTURE.md) for design details

## Troubleshooting

### "KX_ERROR_FEATURE_NOT_PRESENT"
- Feature not implemented in current driver
- Check driver capabilities with `kxGetPhysicalDeviceFeatures`

### "KX_ERROR_OUT_OF_DEVICE_MEMORY"
- Reduce resource sizes
- Check memory requirements with `kxGetBufferMemoryRequirements` / `kxGetImageMemoryRequirements`

### Validation Errors
- Enable debug layer: `KX_EXT_DEBUG_UTILS`
- Check structure `sType` fields are correct
- Verify all required fields are filled

## Support

- Check existing issues on GitHub
- Read the specification in `spec.md`
- Run tests: `ctest --output-on-failure`