#include "SoftwareDevice.h"

#include <cstring>
#include <variant>

#include "Allocator.h"
#include "SoftwareCommandBuffer.h"

namespace kx {

SoftwareDevice::SoftwareDevice(PhysicalDevice* physical_device)
    : physical_device_(physical_device) {}

SoftwareMemory* SoftwareDevice::find_memory(KxDeviceMemory mem) noexcept {
    for (auto& m : memories_) {
        if (reinterpret_cast<KxDeviceMemory>(m.get()) == mem) return m.get();
    }
    return nullptr;
}

SoftwareBuffer* SoftwareDevice::find_buffer(KxBuffer buf) noexcept {
    for (auto& b : buffers_) {
        if (reinterpret_cast<KxBuffer>(b.get()) == buf) return b.get();
    }
    return nullptr;
}

SoftwareImage* SoftwareDevice::find_image(KxImage img) noexcept {
    for (auto& i : images_) {
        if (reinterpret_cast<KxImage>(i.get()) == img) return i.get();
    }
    return nullptr;
}

SoftwareCommandPool* SoftwareDevice::find_command_pool(KxCommandPool pool) noexcept {
    for (auto& p : command_pools_) {
        if (reinterpret_cast<KxCommandPool>(p.get()) == pool) return p.get();
    }
    return nullptr;
}

SoftwareCommandBuffer* SoftwareDevice::find_command_buffer(KxCommandBuffer cb) noexcept {
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
    if (!buffer) return KX_ERROR_INVALID_ARGUMENT;
    *buffer = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->size == 0) return KX_ERROR_INVALID_ARGUMENT;

    // Allocate memory for the buffer (use memory type 0 = host visible)
    KxMemoryAllocateInfo mem_info{};
    mem_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = create_info->size;
    mem_info.memoryTypeIndex = 0;

    KxDeviceMemory memory = nullptr;
    KxResult result = allocate_memory(&mem_info, allocator, &memory);
    if (result != KX_SUCCESS) return result;

    // Create and register the buffer
    auto sw_memory = find_memory(memory);
    if (!sw_memory) {
        free_memory(memory, allocator);
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    auto buf = std::make_unique<SoftwareBuffer>(
        sw_memory, 0, create_info->size, create_info->usage);
    *buffer = register_buffer(std::move(buf));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_buffer(KxBuffer buffer,
                                    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    auto* buf = find_buffer(buffer);
    if (buf) {
        // Free the underlying memory
        if (buf->memory()) {
            KxDeviceMemory mem = reinterpret_cast<KxDeviceMemory>(buf->memory());
            free_memory(mem, allocator);
        }
        unregister_buffer(buffer);
    }
}

KxResult SoftwareDevice::create_image(
    const KxImageCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxImage* image) {
    if (!image) return KX_ERROR_INVALID_ARGUMENT;
    *image = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->width == 0 || create_info->height == 0)
        return KX_ERROR_INVALID_ARGUMENT;

    // Calculate image size based on format and dimensions
    uint32_t pixel_size = 4; // default RGBA8
    switch (create_info->format) {
    case KX_FORMAT_R8_UNORM:
    case KX_FORMAT_R8_SNORM:
    case KX_FORMAT_R8_USCALED:
    case KX_FORMAT_R8_SSCALED:
    case KX_FORMAT_R8_UINT:
    case KX_FORMAT_R8_SINT:
        pixel_size = 1;
        break;
    case KX_FORMAT_R16_UNORM:
    case KX_FORMAT_R16_SNORM:
    case KX_FORMAT_R16_USCALED:
    case KX_FORMAT_R16_SSCALED:
    case KX_FORMAT_R16_UINT:
    case KX_FORMAT_R16_SINT:
    case KX_FORMAT_R16_SFLOAT:
        pixel_size = 2;
        break;
    case KX_FORMAT_R32_SFLOAT:
    case KX_FORMAT_D32_SFLOAT:
        pixel_size = 4;
        break;
    case KX_FORMAT_R8G8_UNORM:
    case KX_FORMAT_R8G8_SNORM:
    case KX_FORMAT_R8G8_UINT:
    case KX_FORMAT_R8G8_SINT:
        pixel_size = 2;
        break;
    case KX_FORMAT_R32G32_SFLOAT:
        pixel_size = 8;
        break;
    case KX_FORMAT_R32G32B32_SFLOAT:
        pixel_size = 12;
        break;
    case KX_FORMAT_R32G32B32A32_SFLOAT:
    case KX_FORMAT_B8G8R8A8_UNORM:
    case KX_FORMAT_B8G8R8A8_SRGB:
    case KX_FORMAT_R8G8B8A8_UNORM:
    case KX_FORMAT_R8G8B8A8_SRGB:
        pixel_size = 4;
        break;
    default:
        pixel_size = 4;
    }

    uint64_t image_size = static_cast<uint64_t>(create_info->width) *
                          static_cast<uint64_t>(create_info->height) *
                          static_cast<uint64_t>(create_info->depth) *
                          static_cast<uint64_t>(create_info->mipLevels) *
                          static_cast<uint64_t>(create_info->arrayLayers) *
                          pixel_size;

    // Allocate memory for the image
    KxMemoryAllocateInfo mem_info{};
    mem_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = image_size;
    mem_info.memoryTypeIndex = 0;

    KxDeviceMemory memory = nullptr;
    KxResult result = allocate_memory(&mem_info, allocator, &memory);
    if (result != KX_SUCCESS) return result;

    // Create and register the image
    auto img = std::make_unique<SoftwareImage>(*create_info, allocator);
    *image = register_image(std::move(img));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_image(KxImage image,
                                   const KxAllocationCallbacks* allocator) {
    (void)allocator;
    auto* img = find_image(image);
    if (img) {
        // Free the underlying memory - we need to track which memory belongs to the image
        // For now, we'll just unregister; memory tracking would need enhancement
        unregister_image(image);
    }
}

KxResult SoftwareDevice::create_command_pool(
    const KxCommandPoolCreateInfo* create_info,
    const KxAllocationCallbacks* /*allocator*/,
    KxCommandPool* command_pool) {
    if (!command_pool) return KX_ERROR_INVALID_ARGUMENT;
    *command_pool = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    auto pool = std::make_unique<SoftwareCommandPool>(create_info->queueFamilyIndex);
    *command_pool = register_command_pool(std::move(pool));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_command_pool(KxCommandPool command_pool,
                                          const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_command_pool(command_pool);
}

KxResult SoftwareDevice::allocate_command_buffers(
    const KxCommandBufferAllocateInfo* allocate_info,
    KxCommandBuffer* command_buffers) {
    if (!command_buffers) return KX_ERROR_INVALID_ARGUMENT;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;
    if (allocate_info->sType != KX_STRUCTURE_TYPE_COMMAND_BUFFER_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;
    if (allocate_info->commandBufferCount == 0) return KX_ERROR_INVALID_ARGUMENT;

    auto* pool = find_command_pool(allocate_info->commandPool);
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;

    for (uint32_t i = 0; i < allocate_info->commandBufferCount; ++i) {
        auto cmd = std::make_unique<SoftwareCommandBuffer>();
        command_buffers[i] = register_command_buffer(std::move(cmd));
    }
    return KX_SUCCESS;
}

void SoftwareDevice::free_command_buffers(KxCommandPool command_pool,
                                          uint32_t count,
                                          const KxCommandBuffer* command_buffers) {
    (void)command_pool;
    if (!command_buffers || count == 0) return;
    for (uint32_t i = 0; i < count; ++i) {
        unregister_command_buffer(command_buffers[i]);
    }
}

KxResult SoftwareDevice::begin_command_buffer(
    KxCommandBuffer command_buffer,
    const KxCommandBufferBeginInfo* begin_info) {
    (void)begin_info;
    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;
    if (cmd->is_recording()) return KX_ERROR_INVALID_ARGUMENT;
    cmd->begin();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::end_command_buffer(KxCommandBuffer command_buffer) {
    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;
    if (!cmd->is_recording()) return KX_ERROR_INVALID_ARGUMENT;
    cmd->end();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::queue_submit(uint32_t submit_count,
                                      const KxSubmitInfo* submits,
                                      KxFence fence) {
    (void)fence;
    if (!submits || submit_count == 0) return KX_SUCCESS;

    for (uint32_t i = 0; i < submit_count; ++i) {
        const auto& submit = submits[i];
        if (submit.sType != KX_STRUCTURE_TYPE_SUBMIT_INFO) return KX_ERROR_INVALID_ARGUMENT;

        for (uint32_t j = 0; j < submit.commandBufferCount; ++j) {
            auto* cmd = find_command_buffer(submit.pCommandBuffers[j]);
            if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

            // Execute all commands in the command buffer
            for (const auto& command : cmd->commands()) {
                execute_command(command);
            }
        }
    }
    return KX_SUCCESS;
}

KxResult SoftwareDevice::queue_wait_idle() {
    // For software driver, queue_submit is synchronous, so wait_idle is a no-op
    return KX_SUCCESS;
}

void SoftwareDevice::execute_command(const CommandVariant& cmd) {
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, CmdClearImage>) {
            execute_clear_image(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyBuffer>) {
            execute_copy_buffer(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyBufferToImage>) {
            execute_copy_buffer_to_image(arg);
        } else if constexpr (std::is_same_v<T, CmdBindPipeline>) {
            // Pipeline binding - store for draw/dispatch
            (void)arg;
        } else if constexpr (std::is_same_v<T, CmdDraw>) {
            // Draw - not implemented yet
            (void)arg;
        } else if constexpr (std::is_same_v<T, CmdDispatch>) {
            // Dispatch - not implemented yet
            (void)arg;
        }
    }, cmd);
}

void SoftwareDevice::execute_clear_image(const CmdClearImage& cmd) {
    if (!cmd.image) return;
    uint8_t* data = cmd.image->data();
    if (!data) return;

    uint32_t width = cmd.image->width();
    uint32_t height = cmd.image->height();
    uint32_t pixel_size = 4; // RGBA8 default

    // Simple clear - fill with color
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t* pixel = data + (y * width + x) * pixel_size;
            pixel[0] = static_cast<uint8_t>(cmd.color[0] * 255.0f);
            pixel[1] = static_cast<uint8_t>(cmd.color[1] * 255.0f);
            pixel[2] = static_cast<uint8_t>(cmd.color[2] * 255.0f);
            pixel[3] = static_cast<uint8_t>(cmd.color[3] * 255.0f);
        }
    }
}

void SoftwareDevice::execute_copy_buffer(const CmdCopyBuffer& cmd) {
    if (!cmd.src || !cmd.dst) return;
    if (cmd.size == 0) return;

    uint8_t* src_ptr = cmd.src->mapped_ptr() + cmd.src_offset;
    uint8_t* dst_ptr = cmd.dst->mapped_ptr() + cmd.dst_offset;

    if (src_ptr && dst_ptr) {
        std::memcpy(dst_ptr, src_ptr, static_cast<size_t>(cmd.size));
    }
}

void SoftwareDevice::execute_copy_buffer_to_image(const CmdCopyBufferToImage& cmd) {
    if (!cmd.src || !cmd.dst) return;
    if (cmd.dst_width == 0 || cmd.dst_height == 0) return;

    uint8_t* src_ptr = cmd.src->mapped_ptr() + cmd.src_offset;
    uint8_t* dst_ptr = cmd.dst->data();

    if (src_ptr && dst_ptr) {
        // Simple row-by-row copy assuming tightly packed
        uint32_t pixel_size = 4;
        for (uint32_t y = 0; y < cmd.dst_height; ++y) {
            std::memcpy(dst_ptr + y * cmd.dst_width * pixel_size,
                        src_ptr + y * cmd.dst_width * pixel_size,
                        cmd.dst_width * pixel_size);
        }
    }
}

KxResult SoftwareDevice::device_wait_idle() {
    return KX_SUCCESS;
}

KxResult SoftwareDevice::bind_buffer_memory(
    KxBuffer buffer,
    KxDeviceMemory memory,
    uint64_t memory_offset) {
    auto* buf = find_buffer(buffer);
    auto* mem = find_memory(memory);
    if (!buf || !mem) return KX_ERROR_INVALID_ARGUMENT;
    buf->bind_memory(mem, memory_offset);
    return KX_SUCCESS;
}

KxResult SoftwareDevice::bind_image_memory(
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memory_offset) {
    (void)memory_offset;
    auto* img = find_image(image);
    auto* mem = find_memory(memory);
    if (!img || !mem) return KX_ERROR_INVALID_ARGUMENT;
    img->bind_memory(mem);
    return KX_SUCCESS;
}

KxResult SoftwareDevice::map_memory(
    KxDeviceMemory memory,
    uint64_t offset,
    uint64_t size,
    KxMemoryMapFlags flags,
    void** pp_data) {
    (void)flags;
    if (!pp_data) return KX_ERROR_INVALID_ARGUMENT;
    *pp_data = nullptr;
    auto* mem = find_memory(memory);
    if (!mem) return KX_ERROR_INVALID_ARGUMENT;
    if (offset + size > mem->size()) return KX_ERROR_MEMORY_MAP_FAILED;
    *pp_data = mem->data() + offset;
    return KX_SUCCESS;
}

void SoftwareDevice::unmap_memory(KxDeviceMemory memory) {
    (void)memory;
    // No-op for host-visible memory
}

} /* namespace kx */