#ifndef KROMAX_SRC_VK_BACKEND_VKBUFFER_H
#define KROMAX_SRC_VK_BACKEND_VKBUFFER_H

/*
 * VkBuffer.h
 *
 * Vulkan buffer, image, image view, memory, and sampler wrappers.
 */

#include "VkLoader.h"
#include <kx_core.h>

#include <cstdint>

namespace kx {

// Buffer
class VkBufferImpl final {
public:
    VkBufferImpl() = default;
    ~VkBufferImpl();

    VkBufferImpl(const VkBufferImpl&) = delete;
    VkBufferImpl& operator=(const VkBufferImpl&) = delete;

    VkBufferImpl(VkBufferImpl&& other) noexcept;
    VkBufferImpl& operator=(VkBufferImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxBufferCreateInfo* create_info);

    VkBuffer handle() const noexcept { return buffer_; }
    VkDeviceSize size() const noexcept { return size_; }
    VkBufferUsageFlags usage() const noexcept { return usage_; }
    VkDeviceMemory memory() const noexcept { return memory_; }
    VkDeviceSize memory_offset() const noexcept { return memory_offset_; }

    void set_memory(VkDeviceMemory mem, VkDeviceSize offset) noexcept {
        memory_ = mem;
        memory_offset_ = offset;
    }

private:
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
    VkBufferUsageFlags usage_ = 0;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkDeviceSize memory_offset_ = 0;
};

// Image
class VkImageImpl final {
public:
    VkImageImpl() = default;
    ~VkImageImpl();

    VkImageImpl(const VkImageImpl&) = delete;
    VkImageImpl& operator=(const VkImageImpl&) = delete;

    VkImageImpl(VkImageImpl&& other) noexcept;
    VkImageImpl& operator=(VkImageImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxImageCreateInfo* create_info);

    VkImage handle() const noexcept { return image_; }
    VkFormat format() const noexcept { return format_; }
    VkExtent3D extent() const noexcept { return extent_; }
    uint32_t mip_levels() const noexcept { return mip_levels_; }
    uint32_t array_layers() const noexcept { return array_layers_; }
    VkImageTiling tiling() const noexcept { return tiling_; }
    VkImageLayout layout() const noexcept { return layout_; }
    VkDeviceMemory memory() const noexcept { return memory_; }
    VkDeviceSize memory_offset() const noexcept { return memory_offset_; }

    void set_memory(VkDeviceMemory mem, VkDeviceSize offset) noexcept {
        memory_ = mem;
        memory_offset_ = offset;
    }
    void set_layout(VkImageLayout new_layout) noexcept { layout_ = new_layout; }

private:
    VkImage image_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkExtent3D extent_ = {1, 1, 1};
    uint32_t mip_levels_ = 1;
    uint32_t array_layers_ = 1;
    VkImageTiling tiling_ = VK_IMAGE_TILING_OPTIMAL;
    VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkDeviceSize memory_offset_ = 0;
};

// Image view
class VkImageViewImpl final {
public:
    VkImageViewImpl() = default;
    ~VkImageViewImpl();

    VkImageViewImpl(const VkImageViewImpl&) = delete;
    VkImageViewImpl& operator=(const VkImageViewImpl&) = delete;

    VkImageViewImpl(VkImageViewImpl&& other) noexcept;
    VkImageViewImpl& operator=(VkImageViewImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxImageViewCreateInfo* create_info);

    VkImageView handle() const noexcept { return view_; }
    VkImage image() const noexcept { return image_; }
    VkFormat format() const noexcept { return format_; }
    VkImageViewType view_type() const noexcept { return view_type_; }

private:
    VkImageView view_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkImage image_ = VK_NULL_HANDLE;
    VkFormat format_ = VK_FORMAT_UNDEFINED;
    VkImageViewType view_type_ = VK_IMAGE_VIEW_TYPE_2D;
};

// Device memory
class VkMemoryImpl final {
public:
    VkMemoryImpl() = default;
    ~VkMemoryImpl();

    VkMemoryImpl(const VkMemoryImpl&) = delete;
    VkMemoryImpl& operator=(const VkMemoryImpl&) = delete;

    VkMemoryImpl(VkMemoryImpl&& other) noexcept;
    VkMemoryImpl& operator=(VkMemoryImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxMemoryAllocateInfo* allocate_info);

    VkDeviceMemory handle() const noexcept { return memory_; }
    VkDeviceSize size() const noexcept { return size_; }
    uint32_t memory_type_index() const noexcept { return memory_type_index_; }
    void* mapped_ptr() const noexcept { return mapped_ptr_; }
    bool is_mapped() const noexcept { return mapped_ptr_ != nullptr; }

    void set_mapped_ptr(void* ptr) noexcept { mapped_ptr_ = ptr; }

private:
    VkDeviceMemory memory_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkDeviceSize size_ = 0;
    uint32_t memory_type_index_ = 0;
    void* mapped_ptr_ = nullptr;
};

// Sampler
class VkSamplerImpl final {
public:
    VkSamplerImpl() = default;
    ~VkSamplerImpl();

    VkSamplerImpl(const VkSamplerImpl&) = delete;
    VkSamplerImpl& operator=(const VkSamplerImpl&) = delete;

    VkSamplerImpl(VkSamplerImpl&& other) noexcept;
    VkSamplerImpl& operator=(VkSamplerImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxSamplerCreateInfo* create_info);

    VkSampler handle() const noexcept { return sampler_; }

private:
    VkSampler sampler_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKBUFFER_H