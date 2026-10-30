#include "Device.h"
#include "SoftwareDevice.h"

namespace kx {

/* Descriptor Set Layout */

KxResult Device::dispatch_create_descriptor_set_layout(KxDevice device,
                                                        const KxDescriptorSetLayoutCreateInfo* info,
                                                        const KxAllocationCallbacks* alloc,
                                                        KxDescriptorSetLayout* layout) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_descriptor_set_layout(info, alloc, layout)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_descriptor_set_layout(KxDevice device, KxDescriptorSetLayout layout,
                                                     const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_descriptor_set_layout(layout, alloc);
}

/* Descriptor Pool */

KxResult Device::dispatch_create_descriptor_pool(KxDevice device,
                                                 const KxDescriptorPoolCreateInfo* info,
                                                 const KxAllocationCallbacks* alloc,
                                                 KxDescriptorPool* pool) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_descriptor_pool(info, alloc, pool)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_descriptor_pool(KxDevice device, KxDescriptorPool pool,
                                              const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_descriptor_pool(pool, alloc);
}

KxResult Device::dispatch_reset_descriptor_pool(KxDevice device, KxDescriptorPool pool,
                                                KxDescriptorPoolResetFlags flags) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->reset_descriptor_pool(pool, flags)
               : KX_ERROR_DEVICE_LOST;
}

/* Descriptor Set */

KxResult Device::dispatch_allocate_descriptor_sets(KxDevice device,
                                                    const KxDescriptorSetAllocateInfo* info,
                                                    KxDescriptorSet* sets) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->allocate_descriptor_sets(info, sets)
               : KX_ERROR_DEVICE_LOST;
}

KxResult Device::dispatch_free_descriptor_sets(KxDevice device, KxDescriptorPool pool,
                                                uint32_t count, const KxDescriptorSet* sets) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->free_descriptor_sets(pool, count, sets)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_update_descriptor_sets(KxDevice device, uint32_t descriptor_write_count,
                                             const KxWriteDescriptorSet* descriptor_writes,
                                             uint32_t descriptor_copy_count,
                                             const KxCopyDescriptorSet* descriptor_copies) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->update_descriptor_sets(descriptor_write_count, descriptor_writes,
                                                          descriptor_copy_count, descriptor_copies);
}

/* Sampler */

KxResult Device::dispatch_create_sampler(KxDevice device,
                                         const KxSamplerCreateInfo* info,
                                         const KxAllocationCallbacks* alloc,
                                         KxSampler* sampler) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_sampler(info, alloc, sampler)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_sampler(KxDevice device, KxSampler sampler,
                                      const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_sampler(sampler, alloc);
}

} /* namespace kx */