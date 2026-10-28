#include "SoftwareDevice.h"

#include <cstring>

#include "SoftwareBuffer.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareCommandPool.h"
#include "SoftwareImage.h"
#include "SoftwareMemory.h"

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

SoftwareCommandPool* SoftwareDevice::find_command_pool(
    KxCommandPool pool) const noexcept {
    for (auto& p : command_pools_) {
        if (reinterpret_cast<KxCommandPool>(p.get()) == pool) return p.get();
    }
    return nullptr;
}

SoftwareCommandBuffer* SoftwareDevice::find_command_buffer(
    KxCommandBuffer cb) const noexcept {
    for (auto& c : command_buffers_) {
        if (reinterpret_cast<KxCommandBuffer>(c.get()) == cb) return c.get();
    }
    return nullptr;
}

KxDeviceMemory SoftwareDevice::register_memory(
    std::unique_ptr<SoftwareMemory> mem) {
    using std::unique_ptr;
    auto* raw = mem.get();
    memories_.push_back(std::move(mem));
    return reinterpret_cast<KxDeviceMemory>(raw);
}
KxBuffer SoftwareDevice::register_buffer(std::unique_ptr<SoftwareBuffer> buf) {
    auto* raw = buf.get();
    buffers_.push_back(std::move(buf));
    return reinterpret_cast<KxBuffer>(raw);
}

KxImage SoftwareDevice::register_image(std::unique_ptr<SoftwareImage> img) {
    auto* raw = img.get();
    images_.push_back(std::move(img));
    return reinterpret_cast<KxImage>(raw);
}

KxCommandPool SoftwareDevice::register_command_pool(
    std::unique_ptr<SoftwareCommandPool> pool) {
    auto* raw = pool.get();
    command_pools_.push_back(std::move(pool));
    return reinterpret_cast<KxCommandPool>(raw);
}

KxCommandBuffer SoftwareDevice::register_command_buffer(
    std::unique_ptr<SoftwareCommandBuffer> cmd) {
    auto* raw = cmd.get();
    command_buffers_.push_back(std::move(cmd));
    return reinterpret_cast<KxCommandBuffer>(raw);
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
    const KxAllocationCallbacks* /*allocator*/,
    KxDeviceMemory* memory) {
    if (!allocate_info || !memory) return KX_ERROR_INVALID_ARGUMENT;
    auto mem = std::make_unique<SoftwareMemory>(
        allocate_info->allocationSize, allocate_info->memoryTypeIndex, nullptr);
    *memory = register_memory(std::move(mem));
    return KX_SUCCESS;
}

void SoftwareDevice::free_memory(KxDeviceMemory memory,
                                 const KxAllocationCallbacks* /*allocator*/) {
    unregister_memory(memory);
}

KxResult SoftwareDevice::create_buffer(
    const KxBufferCreateInfo* create_info,
    const KxAllocationCallbacks* /*allocator*/,
    KxBuffer* buffer) {
    if (!create_info || !buffer) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->size == 0) return KX_ERROR_INVALID_ARGUMENT;

    KxMemoryAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = create_info->size;
    alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory mem = nullptr;
    KxResult res = allocate_memory(&alloc_info, nullptr, &mem);
    if (res != KX_SUCCESS) return res;

    SoftwareMemory* smem = find_memory(mem);
    if (!smem) {
        free_memory(mem, nullptr);
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    auto buf = std::make_unique<SoftwareBuffer>(
        smem, 0, create_info->size, create_info->usage);
    *buffer = register_buffer(std::move(buf));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_buffer(KxBuffer buffer,
                                    const KxAllocationCallbacks* /*allocator*/) {
    SoftwareBuffer* buf = find_buffer(buffer);
    if (!buf) return;
    KxDeviceMemory mem = reinterpret_cast<KxDeviceMemory>(buf->memory());
    unregister_buffer(buffer);
    unregister_memory(mem);
}

KxResult SoftwareDevice::create_image(
    const KxImageCreateInfo* create_info,
    const KxAllocationCallbacks* /*allocator*/,
    KxImage* image) {
    if (!create_info || !image) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->width == 0 || create_info->height == 0)
        return KX_ERROR_INVALID_ARGUMENT;

    auto img = std::make_unique<SoftwareImage>(*create_info, nullptr);
    *image = register_image(std::move(img));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_image(KxImage image,
                                   const KxAllocationCallbacks* /*allocator*/) {
    unregister_image(image);
}

KxResult SoftwareDevice::create_command_pool(
    const KxCommandPoolCreateInfo* create_info,
    const KxAllocationCallbacks* /*allocator*/,
    KxCommandPool* command_pool) {
    if (!create_info || !command_pool) return KX_ERROR_INVALID_ARGUMENT;
    auto pool = std::make_unique<SoftwareCommandPool>(
        create_info->queueFamilyIndex);
    *command_pool = register_command_pool(std::move(pool));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_command_pool(
    KxCommandPool command_pool,
    const KxAllocationCallbacks* /*allocator*/) {
    unregister_command_pool(command_pool);
}
KxResult SoftwareDevice::allocate_command_buffers(
    const KxCommandBufferAllocateInfo* allocate_info,
    KxCommandBuffer* command_buffers) {
    if (!allocate_info || !command_buffers) return KX_ERROR_INVALID_ARGUMENT;
    SoftwareCommandPool* pool = find_command_pool(allocate_info->commandPool);
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;

    auto cmd = std::make_unique<SoftwareCommandBuffer>();
    cmd->begin();
    cmd->end();
    *command_buffers = register_command_buffer(std::move(cmd));
    return KX_SUCCESS;
}

void SoftwareDevice::free_command_buffers(
    KxCommandPool /*command_pool*/,
    uint32_t /*count*/,
    const KxCommandBuffer* /*command_buffers*/) {}

KxResult SoftwareDevice::begin_command_buffer(
    KxCommandBuffer command_buffer,
    const KxCommandBufferBeginInfo* /*begin_info*/) {
    SoftwareCommandBuffer* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;
    cmd->begin();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::end_command_buffer(KxCommandBuffer command_buffer) {
    SoftwareCommandBuffer* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;
    cmd->end();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::queue_submit(uint32_t submit_count,
                                      const KxSubmitInfo* submits,
                                      KxFence /*fence*/) {
    for (uint32_t i = 0; i < submit_count; ++i) {
        const KxSubmitInfo& submit = submits[i];
        for (uint32_t j = 0; j < submit.commandBufferCount; ++j) {
            SoftwareCommandBuffer* cmd = find_command_buffer(
                submit.pCommandBuffers[j]);
            if (!cmd) continue;
            for (const auto& command : cmd->commands()) {
                std::visit(
                    [this](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, kx::kx::CmdClearImage>) {
                            if (arg.image) {
                                arg.image->clear_color(arg.color[0], arg.color[1],
                                                       arg.color[2], arg.color[3]);
                                arg.image->set_layout(arg.layout);
                            }
                        } else if constexpr (std::is_same_v<T, kx::kx::CmdCopyBuffer>) {
                            if (arg.src && arg.dst) {
                                uint8_t* src_ptr =
                                    arg.src->mapped_ptr() + arg.src_offset;
                                uint8_t* dst_ptr =
                                    arg.dst->mapped_ptr() + arg.dst_offset;
                                std::memcpy(dst_ptr, src_ptr,
                                            static_cast<std::size_t>(arg.size));
                            }
                        } else if constexpr (std::is_same_v<T,
                                                           CmdCopyBufferToImage>) {
                            if (arg.src && arg.dst) {
                                const uint8_t* src_ptr =
                                    arg.src->mapped_ptr() + arg.src_offset;
                                uint8_t* dst_ptr = arg.dst->data();
                                std::size_t row_pitch = arg.dst->row_pitch();
                                for (uint32_t y = 0; y < arg.dst_height; ++y) {
                                    std::memcpy(dst_ptr + y * row_pitch,
                                                src_ptr +
                                                    y * arg.dst_width * 4,
                                                static_cast<std::size_t>(
                                                    arg.dst_width * 4));
                                }
                            }
                        }
                    },
                    command);
            }
        }
    }
    return KX_SUCCESS;
}

KxResult SoftwareDevice::queue_wait_idle() { return KX_SUCCESS; }

KxResult SoftwareDevice::device_wait_idle() { return KX_SUCCESS; }

} /* namespace kx */
