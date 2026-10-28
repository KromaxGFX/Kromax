# Kromax Backends Documentation

## Display Backends

### KMS Backend (DRM/KMS)

**Purpose**: Presentation backend using Linux DRM/KMS interface.

**Requirements**:
- Must run from a TTY without a compositor active (DRM master)
- Uses dumb buffers for frame storage
- Modeset, page flip, and vblank event handling

**Interface** (`IDisplayBackend`):
- `initialize()`: Setup DRM connector, crtc, framebuffer
- `create_framebuffer(width, height, format)`: Create dumb buffer
- `present(framebuffer)`: Page flip to present
- `wait_vblank()`: Wait for vertical blank interrupt
- `shutdown()`: Cleanup resources

**Example Usage**:
```c
KxDisplayEXT display;
KX_CHECK(kxCreateDisplay(NULL, &display));
KxSwapchainEXT swapchain;
KX_CHECK(kxCreateSwapchainEXT(display, &swapchain, ...));
```

### Offscreen Backend

**Purpose**: Write rendered frames to PPM file format. Used for CI and development within a desktop session.

**Interface** (`IDisplayBackend`):
- `initialize(NULL)`: No special setup needed
- `create_framebuffer(width, height, format)`: Allocate internal buffer
- `present(framebuffer)`: Write PPM file
- `wait_vblank()`: No-op (no vblank on desktop)
- `shutdown()`: Cleanup

**PPM Writer** (`PpmWriter`):
- Writes PPM P6 format header
- Row-by-row pixel data
- Configurable max value (default 255)

## Driver Backends

### Software Driver

**Purpose**: CPU-based reference implementation. Used for CI, development, and offline rendering.

**Features**:
- Memory: System RAM backed
- Buffers: Software rasterization
- Images: Software with PPM output
- Command buffers: Interpreted execution
- Compute: Callback-based shader execution
- Formats: `KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT`

**Limitations**:
- No hardware acceleration
- Limited shader support (callbacks only)
- Performance not suitable for production graphics

### amdgpu Driver

**Purpose**: Hardware driver skeleton for AMD GPUs.

**Status**: Disabled by default (`KX_BUILD_DRIVER_AMDGPU=OFF`)

**Interface** (`AmdGpuDriver`):
- Empty implementation, meant to be filled in future ADR
- Plugin-style `.so` loading (ICD pattern)
- Would expose AMD-specific features: async compute, shader cores, etc.

## Quick Start: Hello World Example

```c
#include <kromax/kromax.h>

int main() {
    // Create instance
    KxInstanceCreateInfo ici = {
        .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &(KxApplicationInfo){
            .sType = KX_STRUCTURE_TYPE_APPLICATION_INFO,
            .apiVersion = KX_API_VERSION_0_1,
        },
    };
    KxInstance instance;
    KX_CHECK(kxCreateInstance(&ici, NULL, &instance));

    // Enumerate physical devices
    uint32_t count = 0;
    kxEnumeratePhysicalDevices(instance, &count, NULL);
    KxPhysicalDevice* devices = malloc(count * sizeof(KxPhysicalDevice));
    kxEnumeratePhysicalDevices(instance, &count, devices);

    // Create device
    KxDeviceCreateInfo dci = {
        .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    };
    KxDevice device;
    KX_CHECK(kxCreateDevice(devices[0], &dci, NULL, &device));

    // Cleanup
    kxDestroyDevice(device);
    kxDestroyInstance(instance);
    free(devices);
    
    return 0;
}
```