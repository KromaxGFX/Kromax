#ifndef KROMAX_SRC_DRIVER_IDRIVERDEVICE_H
#define KROMAX_SRC_DRIVER_IDRIVERDEVICE_H

/*
 * IDriverDevice.h
 *
 * Abstract device interface implemented by each driver's device object.
 * The Kromax Device wraps this and forwards dispatch calls to it.
 */

#include "kx_core.h"

namespace kx {

class IDriverDevice {
public:
    virtual ~IDriverDevice() = default;

    virtual KxResult allocate_memory(
        const KxMemoryAllocateInfo* allocate_info,
        const KxAllocationCallbacks* allocator,
        KxDeviceMemory* memory) = 0;

    virtual void free_memory(
        KxDeviceMemory memory,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_buffer(
        const KxBufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxBuffer* buffer) = 0;

    virtual void destroy_buffer(
        KxBuffer buffer,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_image(
        const KxImageCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImage* image) = 0;

    virtual void destroy_image(
        KxImage image,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_command_pool(
        const KxCommandPoolCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxCommandPool* command_pool) = 0;

    virtual void destroy_command_pool(
        KxCommandPool command_pool,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult allocate_command_buffers(
        const KxCommandBufferAllocateInfo* allocate_info,
        KxCommandBuffer* command_buffers) = 0;

    virtual void free_command_buffers(
        KxCommandPool command_pool,
        uint32_t count,
        const KxCommandBuffer* command_buffers) = 0;

    virtual KxResult begin_command_buffer(
        KxCommandBuffer command_buffer,
        const KxCommandBufferBeginInfo* begin_info) = 0;

    virtual KxResult end_command_buffer(KxCommandBuffer command_buffer) = 0;

    virtual KxResult queue_submit(
        uint32_t submit_count,
        const KxSubmitInfo* submits,
        KxFence fence) = 0;

    virtual KxResult queue_wait_idle() = 0;
    virtual KxResult device_wait_idle() = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_IDRIVERDEVICE_H */