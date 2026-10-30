#include "Device.h"
#include "SoftwareDevice.h"

namespace kx {

/* Static dispatch thunks - forward to driver device via find_device_locked. */

KxResult Device::dispatch_allocate_memory(KxDevice device,
                                          const KxMemoryAllocateInfo* info,
                                          const KxAllocationCallbacks* alloc,
                                          KxDeviceMemory* mem) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->allocate_memory(info, alloc, mem)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_free_memory(KxDevice device, KxDeviceMemory mem,
                                  const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->free_memory(mem, alloc);
}

KxResult Device::dispatch_create_buffer(KxDevice device,
                                        const KxBufferCreateInfo* info,
                                        const KxAllocationCallbacks* alloc,
                                        KxBuffer* buf) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_buffer(info, alloc, buf)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_buffer(KxDevice device, KxBuffer buf,
                                     const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_buffer(buf, alloc);
}

KxResult Device::dispatch_create_image(KxDevice device,
                                       const KxImageCreateInfo* info,
                                       const KxAllocationCallbacks* alloc,
                                       KxImage* img) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_image(info, alloc, img)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_image(KxDevice device, KxImage img,
                                    const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_image(img, alloc);
}

KxResult Device::dispatch_create_image_view(KxDevice device,
                                            const KxImageViewCreateInfo* info,
                                            const KxAllocationCallbacks* alloc,
                                            KxImageView* view) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_image_view(info, alloc, view)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_image_view(KxDevice device, KxImageView view,
                                         const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_image_view(view, alloc);
}

KxResult Device::dispatch_create_render_pass(KxDevice device,
                                             const KxRenderPassCreateInfo* info,
                                             const KxAllocationCallbacks* alloc,
                                             KxRenderPass* rp) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_render_pass(info, alloc, rp)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_render_pass(KxDevice device, KxRenderPass rp,
                                          const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_render_pass(rp, alloc);
}

KxResult Device::dispatch_create_framebuffer(KxDevice device,
                                             const KxFramebufferCreateInfo* info,
                                             const KxAllocationCallbacks* alloc,
                                             KxFramebuffer* fb) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_framebuffer(info, alloc, fb)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_framebuffer(KxDevice device, KxFramebuffer fb,
                                          const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_framebuffer(fb, alloc);
}

KxResult Device::dispatch_create_command_pool(KxDevice device,
                                              const KxCommandPoolCreateInfo* info,
                                              const KxAllocationCallbacks* alloc,
                                              KxCommandPool* pool) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_command_pool(info, alloc, pool)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_command_pool(KxDevice device, KxCommandPool pool,
                                           const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_command_pool(pool, alloc);
}

KxResult Device::dispatch_allocate_command_buffers(KxDevice device,
                                                   const KxCommandBufferAllocateInfo* info,
                                                   KxCommandBuffer* buffers) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->allocate_command_buffers(info, buffers)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_free_command_buffers(KxDevice device, KxCommandPool pool,
                                           uint32_t count,
                                           const KxCommandBuffer* buffers) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->free_command_buffers(pool, count, buffers);
}

KxResult Device::dispatch_begin_command_buffer(KxCommandBuffer cb,
                                               const KxCommandBufferBeginInfo* info) {
    auto* dev = find_device_locked_by_command_buffer(cb);
    return dev ? dev->driver_device()->begin_command_buffer(cb, info)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_end_command_buffer(KxCommandBuffer cb) {
    auto* dev = find_device_locked_by_command_buffer(cb);
    return dev ? dev->driver_device()->end_command_buffer(cb)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_queue_submit(KxQueue queue, uint32_t count,
                                       const KxSubmitInfo* submits, KxFence fence) {
    auto* dev = find_device_locked_by_queue(queue);
    return dev ? dev->driver_device()->queue_submit(count, submits, fence)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_queue_wait_idle(KxQueue queue) {
    auto* dev = find_device_locked_by_queue(queue);
    return dev ? dev->driver_device()->queue_wait_idle()
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_device_wait_idle(KxDevice device) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->device_wait_idle()
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_bind_buffer_memory(KxDevice device,
                                             KxBuffer buffer,
                                             KxDeviceMemory memory,
                                             uint64_t memory_offset) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->bind_buffer_memory(buffer, memory, memory_offset)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_bind_image_memory(KxDevice device,
                                            KxImage image,
                                            KxDeviceMemory memory,
                                            uint64_t memory_offset) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->bind_image_memory(image, memory, memory_offset)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_map_memory(KxDevice device,
                                     KxDeviceMemory memory,
                                     uint64_t offset,
                                     uint64_t size,
                                     KxMemoryMapFlags flags,
                                     void** pp_data) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->map_memory(memory, offset, size, flags, pp_data)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_unmap_memory(KxDevice device, KxDeviceMemory memory) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->unmap_memory(memory);
}

} /* namespace kx */