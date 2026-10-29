#include "SoftwareDevice.h"

#include <cstring>

#include "Allocator.h"

namespace kx {

SoftwareDevice::SoftwareDevice(PhysicalDevice* physical_device)
    : physical_device_(physical_device) {}

SoftwareMemory* SoftwareDevice::find_memory(KxDeviceMemory mem) const noexcept {
    for (auto& m : memories_) {
        if (reinterpret_cast<KxDeviceMemory>(m.get()) == mem) return m.get();
    }
    return nullptr;
}

SoftwareBuffer* SoftwareDevice::find_buffer(KxBuffer buf) const noexcept {
    for (auto& b : buffers_) {
        if (reinterpret_cast<KxBuffer>(b.get()) == buf) return b.get();
    }
    return nullptr;
}

SoftwareImage* SoftwareDevice::find_image(KxImage img) const noexcept {
    for (auto& i : images_) {
        if (reinterpret_cast<KxImage>(i.get()) == img) return i.get();
    }
    return nullptr;
}

SoftwareCommandPool* SoftwareDevice::find_command_pool(KxCommandPool pool) const noexcept {
    for (auto& p : command_pools_) {
        if (reinterpret_cast<KxCommandPool>(p.get()) == pool) return p.get();
    }
    return nullptr;
}

SoftwareCommandBuffer* SoftwareDevice::find_command_buffer(KxCommandBuffer cb) const noexcept {
    for (auto& c : command_buffers_) {
        if (reinterpret_cast<KxCommandBuffer>(c.get()) == cb) return c.get();
    }
    return nullptr;
}

KxDeviceMemory SoftwareDevice::register_memory(std::unique_ptr<SoftwareMemory> mem) {
    KxDeviceMemory handle = reinterpret_cast<KxDeviceMemory>(mem.get());
    memories_.push_back(std::move(mem));
    return handle;
}

KxBuffer SoftwareDevice::register_buffer(std::unique_ptr<SoftwareBuffer> buf) {
    KxBuffer handle = reinterpret_cast<KxBuffer>(buf.get());
    buffers_.push_back(std::move(buf));
    return handle;
}

KxImage SoftwareDevice::register_image(std::unique_ptr<SoftwareImage> img) {
    KxImage handle = reinterpret_cast<KxImage>(img.get());
    images_.push_back(std::move(img));
    return handle;
}

KxCommandPool SoftwareDevice::register_command_pool(std::unique_ptr<SoftwareCommandPool> pool) {
    KxCommandPool handle = reinterpret_cast<KxCommandPool>(pool.get());
    command_pools_.push_back(std::move(pool));
    return handle;
}

KxCommandBuffer SoftwareDevice::register_command_buffer(
    std::unique_ptr<SoftwareCommandBuffer> cmd) {
    KxCommandBuffer handle = reinterpret_cast<KxCommandBuffer>(cmd.get());
    command_buffers_.push_back(std::move(cmd));
    return handle;
}

void SoftwareDevice::unregister_memory(KxDeviceMemory mem) {
    for (auto it = memories_.begin(); it != memories_.end(); ++it) {
        if (reinterpret_cast<KxDeviceMemory>(it->get()) == mem) {
            memories_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_buffer(KxBuffer buf) {
    for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
        if (reinterpret_cast<KxBuffer>(it->get()) == buf) {
            buffers_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_image(KxImage img) {
    for (auto it = images_.begin(); it != images_.end(); ++it) {
        if (reinterpret_cast<KxImage>(it->get()) == img) {
            images_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_command_pool(KxCommandPool pool) {
    for (auto it = command_pools_.begin(); it != command_pools_.end(); ++it) {
        if (reinterpret_cast<KxCommandPool>(it->get()) == pool) {
            command_pools_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_command_buffer(KxCommandBuffer cb) {
    for (auto it = command_buffers_.begin(); it != command_buffers_.end(); ++it) {
        if (reinterpret_cast<KxCommandBuffer>(it->get()) == cb) {
            command_buffers_.erase(it);
            return;
        }
    }
}

KxResult SoftwareDevice::allocate_memory(
    const KxMemoryAllocateInfo* allocate_info,
    const KxAllocationCallbacks* allocator,
    KxDeviceMemory* memory) {
    if (!memory) return KX_ERROR_INVALID_ARGUMENT;
    *memory = nullptr;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;

    auto mem = std::make_unique<SoftwareMemory>(
        allocate_info->allocationSize,
        allocate_info->memoryTypeIndex,
        allocator);
    *memory = register_memory(std::move(mem));
    return KX_SUCCESS;
}

void SoftwareDevice::free_memory(KxDeviceMemory memory,
                                 const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_memory(memory);
}

KxResult SoftwareDevice::create_buffer(
    const KxBufferCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxBuffer* buffer) {
    (void)create_info;
    (void)allocator;
    (void)buffer;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void SoftwareDevice::destroy_buffer(KxBuffer buffer,
                                    const KxAllocationCallbacks* allocator) {
    (void)buffer;
    (void)allocator;
}

KxResult SoftwareDevice::create_image(
    const KxImageCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxImage* image) {
    (void)create_info;
    (void)allocator;
    (void)image;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void SoftwareDevice::destroy_image(KxImage image,
                                   const KxAllocationCallbacks* allocator) {
    (void)image;
    (void)allocator;
}

KxResult SoftwareDevice::create_command_pool(
    const KxCommandPoolCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxCommandPool* command_pool) {
    (void)create_info;
    (void)allocator;
    (void)command_pool;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void SoftwareDevice::destroy_command_pool(KxCommandPool command_pool,
                                          const KxAllocationCallbacks* allocator) {
    (void)command_pool;
    (void)allocator;
}

KxResult SoftwareDevice::allocate_command_buffers(
    const KxCommandBufferAllocateInfo* allocate_info,
    KxCommandBuffer* command_buffers) {
    (void)allocate_info;
    (void)command_buffers;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void SoftwareDevice::free_command_buffers(KxCommandPool command_pool,
                                          uint32_t count,
                                          const KxCommandBuffer* command_buffers) {
    (void)command_pool;
    (void)count;
    (void)command_buffers;
}

KxResult SoftwareDevice::begin_command_buffer(
    KxCommandBuffer command_buffer,
    const KxCommandBufferBeginInfo* begin_info) {
    (void)command_buffer;
    (void)begin_info;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult SoftwareDevice::end_command_buffer(KxCommandBuffer command_buffer) {
    (void)command_buffer;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult SoftwareDevice::queue_submit(uint32_t submit_count,
                                      const KxSubmitInfo* submits,
                                      KxFence fence) {
    (void)submit_count;
    (void)submits;
    (void)fence;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult SoftwareDevice::queue_wait_idle() {
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult SoftwareDevice::device_wait_idle() {
    return KX_SUCCESS;
}

} /* namespace kx */