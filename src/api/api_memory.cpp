/*
 * api_memory.cpp
 *
 * Memory, Buffer, Image C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"
#include "IDriver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Memory / Buffer / Image (delegated to device dispatch)
 * ======================================================================= */

KxResult KX_CALL kxAllocateMemory(
    KxDevice device,
    const KxMemoryAllocateInfo* pAllocateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDeviceMemory* pMemory) {
    if (!pMemory) return KX_ERROR_INVALID_ARGUMENT;
    *pMemory = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->allocate_memory)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->allocate_memory(device, pAllocateInfo, pAllocator,
                                            pMemory);
}

void KX_CALL kxFreeMemory(
    KxDevice device,
    KxDeviceMemory memory,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->free_memory) return;
    dev->dispatch()->free_memory(device, memory, pAllocator);
}

KxResult KX_CALL kxMapMemory(
    KxDevice device,
    KxDeviceMemory memory,
    uint64_t offset,
    uint64_t size,
    KxMemoryMapFlags flags,
    void** ppData) {
    if (!ppData) return KX_ERROR_INVALID_ARGUMENT;
    *ppData = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->map_memory)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->map_memory(device, memory, offset, size, flags, ppData);
}

void KX_CALL kxUnmapMemory(
    KxDevice device,
    KxDeviceMemory memory) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->unmap_memory) return;
    dev->dispatch()->unmap_memory(device, memory);
}

KxResult KX_CALL kxCreateBuffer(
    KxDevice device,
    const KxBufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxBuffer* pBuffer) {
    if (!pBuffer) return KX_ERROR_INVALID_ARGUMENT;
    *pBuffer = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_buffer)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_buffer(device, pCreateInfo, pAllocator,
                                          pBuffer);
}

void KX_CALL kxDestroyBuffer(
    KxDevice device,
    KxBuffer buffer,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_buffer) return;
    dev->dispatch()->destroy_buffer(device, buffer, pAllocator);
}

KxResult KX_CALL kxBindBufferMemory(
    KxDevice device,
    KxBuffer buffer,
    KxDeviceMemory memory,
    uint64_t memoryOffset) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->bind_buffer_memory)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->bind_buffer_memory(device, buffer, memory, memoryOffset);
}

KxResult KX_CALL kxCreateImage(
    KxDevice device,
    const KxImageCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage) {
    if (!pImage) return KX_ERROR_INVALID_ARGUMENT;
    *pImage = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_image)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_image(device, pCreateInfo, pAllocator,
                                         pImage);
}

void KX_CALL kxDestroyImage(
    KxDevice device,
    KxImage image,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_image) return;
    dev->dispatch()->destroy_image(device, image, pAllocator);
}

KxResult KX_CALL kxBindImageMemory(
    KxDevice device,
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memoryOffset) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->bind_image_memory)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->bind_image_memory(device, image, memory, memoryOffset);
}

#ifdef __cplusplus
} /* extern "C" */
#endif