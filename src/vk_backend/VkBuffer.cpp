/*
 * VkBuffer.cpp
 *
 * Vulkan buffer, image, image view, memory, and sampler implementation.
 */

#include "VkBuffer.h"

#include <cstring>

namespace kx {

// ============================================================================
// VkBufferImpl
// ============================================================================

VkBufferImpl::~VkBufferImpl() {
    if (buffer_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkBufferImpl::VkBufferImpl(VkBufferImpl&& other) noexcept
    : buffer_(other.buffer_)
    , device_(other.device_)
    , size_(other.size_)
    , usage_(other.usage_)
    , memory_(other.memory_)
    , memory_offset_(other.memory_offset_) {
    other.buffer_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.size_ = 0;
    other.usage_ = 0;
    other.memory_ = VK_NULL_HANDLE;
    other.memory_offset_ = 0;
}

VkBufferImpl& VkBufferImpl::operator=(VkBufferImpl&& other) noexcept {
    if (this != &other) {
        buffer_ = other.buffer_;
        device_ = other.device_;
        size_ = other.size_;
        usage_ = other.usage_;
        memory_ = other.memory_;
        memory_offset_ = other.memory_offset_;
        other.buffer_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.size_ = 0;
        other.usage_ = 0;
        other.memory_ = VK_NULL_HANDLE;
        other.memory_offset_ = 0;
    }
    return *this;
}

bool VkBufferImpl::initialize(VkFunctionTable& functions,
                              VkDevice device,
                              const KxBufferCreateInfo* create_info) {
    device_ = device;
    size_ = create_info->size;
    usage_ = static_cast<VkBufferUsageFlags>(create_info->usage);

    VkBufferCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vk_info.flags = static_cast<VkBufferCreateFlags>(create_info->flags);
    vk_info.size = create_info->size;
    vk_info.usage = static_cast<VkBufferUsageFlags>(create_info->usage);
    vk_info.sharingMode = static_cast<VkSharingMode>(create_info->sharingMode);
    vk_info.queueFamilyIndexCount = create_info->queueFamilyIndexCount;
    vk_info.pQueueFamilyIndices = create_info->pQueueFamilyIndices;

    VkResult result = functions.vkCreateBuffer(device, &vk_info, nullptr, &buffer_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkImageImpl
// ============================================================================

VkImageImpl::~VkImageImpl() {
    if (image_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkImageImpl::VkImageImpl(VkImageImpl&& other) noexcept
    : image_(other.image_)
    , device_(other.device_)
    , format_(other.format_)
    , extent_(other.extent_)
    , mip_levels_(other.mip_levels_)
    , array_layers_(other.array_layers_)
    , tiling_(other.tiling_)
    , layout_(other.layout_)
    , memory_(other.memory_)
    , memory_offset_(other.memory_offset_) {
    other.image_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.format_ = VK_FORMAT_UNDEFINED;
    other.extent_ = {1, 1, 1};
    other.mip_levels_ = 1;
    other.array_layers_ = 1;
    other.tiling_ = VK_IMAGE_TILING_OPTIMAL;
    other.layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
    other.memory_ = VK_NULL_HANDLE;
    other.memory_offset_ = 0;
}

VkImageImpl& VkImageImpl::operator=(VkImageImpl&& other) noexcept {
    if (this != &other) {
        image_ = other.image_;
        device_ = other.device_;
        format_ = other.format_;
        extent_ = other.extent_;
        mip_levels_ = other.mip_levels_;
        array_layers_ = other.array_layers_;
        tiling_ = other.tiling_;
        layout_ = other.layout_;
        memory_ = other.memory_;
        memory_offset_ = other.memory_offset_;
        other.image_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.format_ = VK_FORMAT_UNDEFINED;
        other.extent_ = {1, 1, 1};
        other.mip_levels_ = 1;
        other.array_layers_ = 1;
        other.tiling_ = VK_IMAGE_TILING_OPTIMAL;
        other.layout_ = VK_IMAGE_LAYOUT_UNDEFINED;
        other.memory_ = VK_NULL_HANDLE;
        other.memory_offset_ = 0;
    }
    return *this;
}

bool VkImageImpl::initialize(VkFunctionTable& functions,
                             VkDevice device,
                             const KxImageCreateInfo* create_info) {
    device_ = device;
    format_ = static_cast<VkFormat>(create_info->format);
    extent_ = create_info->extent;
    mip_levels_ = create_info->mipLevels;
    array_layers_ = create_info->arrayLayers;
    tiling_ = static_cast<VkImageTiling>(create_info->tiling);
    layout_ = static_cast<VkImageLayout>(create_info->initialLayout);

    VkImageCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vk_info.flags = static_cast<VkImageCreateFlags>(create_info->flags);
    vk_info.imageType = static_cast<VkImageType>(create_info->imageType);
    vk_info.format = format_;
    vk_info.extent = extent_;
    vk_info.mipLevels = mip_levels_;
    vk_info.arrayLayers = array_layers_;
    vk_info.samples = static_cast<VkSampleCountFlagBits>(create_info->samples);
    vk_info.tiling = tiling_;
    vk_info.usage = static_cast<VkImageUsageFlags>(create_info->usage);
    vk_info.sharingMode = static_cast<VkSharingMode>(create_info->sharingMode);
    vk_info.queueFamilyIndexCount = create_info->queueFamilyIndexCount;
    vk_info.pQueueFamilyIndices = create_info->pQueueFamilyIndices;
    vk_info.initialLayout = layout_;

    VkResult result = functions.vkCreateImage(device, &vk_info, nullptr, &image_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkImageViewImpl
// ============================================================================

VkImageViewImpl::~VkImageViewImpl() {
    if (view_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkImageViewImpl::VkImageViewImpl(VkImageViewImpl&& other) noexcept
    : view_(other.view_)
    , device_(other.device_)
    , image_(other.image_)
    , format_(other.format_)
    , view_type_(other.view_type_) {
    other.view_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.image_ = VK_NULL_HANDLE;
    other.format_ = VK_FORMAT_UNDEFINED;
    other.view_type_ = VK_IMAGE_VIEW_TYPE_2D;
}

VkImageViewImpl& VkImageViewImpl::operator=(VkImageViewImpl&& other) noexcept {
    if (this != &other) {
        view_ = other.view_;
        device_ = other.device_;
        image_ = other.image_;
        format_ = other.format_;
        view_type_ = other.view_type_;
        other.view_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.image_ = VK_NULL_HANDLE;
        other.format_ = VK_FORMAT_UNDEFINED;
        other.view_type_ = VK_IMAGE_VIEW_TYPE_2D;
    }
    return *this;
}

bool VkImageViewImpl::initialize(VkFunctionTable& functions,
                                 VkDevice device,
                                 const KxImageViewCreateInfo* create_info) {
    device_ = device;
    image_ = static_cast<VkImage>(create_info->image);
    format_ = static_cast<VkFormat>(create_info->format);
    view_type_ = static_cast<VkImageViewType>(create_info->viewType);

    VkImageViewCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vk_info.flags = static_cast<VkImageViewCreateFlags>(create_info->flags);
    vk_info.image = image_;
    vk_info.viewType = view_type_;
    vk_info.format = format_;
    vk_info.components = *reinterpret_cast<const VkComponentMapping*>(&create_info->components);
    vk_info.subresourceRange = *reinterpret_cast<const VkImageSubresourceRange*>(&create_info->subresourceRange);

    VkResult result = functions.vkCreateImageView(device, &vk_info, nullptr, &view_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkMemoryImpl
// ============================================================================

VkMemoryImpl::~VkMemoryImpl() {
    if (memory_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkMemoryImpl::VkMemoryImpl(VkMemoryImpl&& other) noexcept
    : memory_(other.memory_)
    , device_(other.device_)
    , size_(other.size_)
    , memory_type_index_(other.memory_type_index_)
    , mapped_ptr_(other.mapped_ptr_) {
    other.memory_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.size_ = 0;
    other.memory_type_index_ = 0;
    other.mapped_ptr_ = nullptr;
}

VkMemoryImpl& VkMemoryImpl::operator=(VkMemoryImpl&& other) noexcept {
    if (this != &other) {
        memory_ = other.memory_;
        device_ = other.device_;
        size_ = other.size_;
        memory_type_index_ = other.memory_type_index_;
        mapped_ptr_ = other.mapped_ptr_;
        other.memory_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.size_ = 0;
        other.memory_type_index_ = 0;
        other.mapped_ptr_ = nullptr;
    }
    return *this;
}

bool VkMemoryImpl::initialize(VkFunctionTable& functions,
                              VkDevice device,
                              const KxMemoryAllocateInfo* allocate_info) {
    device_ = device;
    size_ = allocate_info->allocationSize;
    memory_type_index_ = allocate_info->memoryTypeIndex;

    VkMemoryAllocateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vk_info.allocationSize = allocate_info->allocationSize;
    vk_info.memoryTypeIndex = allocate_info->memoryTypeIndex;

    VkResult result = functions.vkAllocateMemory(device, &vk_info, nullptr, &memory_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkSamplerImpl
// ============================================================================

VkSamplerImpl::~VkSamplerImpl() {
    if (sampler_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkSamplerImpl::VkSamplerImpl(VkSamplerImpl&& other) noexcept
    : sampler_(other.sampler_)
    , device_(other.device_) {
    other.sampler_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkSamplerImpl& VkSamplerImpl::operator=(VkSamplerImpl&& other) noexcept {
    if (this != &other) {
        sampler_ = other.sampler_;
        device_ = other.device_;
        other.sampler_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkSamplerImpl::initialize(VkFunctionTable& functions,
                               VkDevice device,
                               const KxSamplerCreateInfo* create_info) {
    device_ = device;

    VkSamplerCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vk_info.flags = static_cast<VkSamplerCreateFlags>(create_info->flags);
    vk_info.magFilter = static_cast<VkFilter>(create_info->magFilter);
    vk_info.minFilter = static_cast<VkFilter>(create_info->minFilter);
    vk_info.mipmapMode = static_cast<VkSamplerMipmapMode>(create_info->mipmapMode);
    vk_info.addressModeU = static_cast<VkSamplerAddressMode>(create_info->addressModeU);
    vk_info.addressModeV = static_cast<VkSamplerAddressMode>(create_info->addressModeV);
    vk_info.addressModeW = static_cast<VkSamplerAddressMode>(create_info->addressModeW);
    vk_info.mipLodBias = create_info->mipLodBias;
    vk_info.anisotropyEnable = create_info->anisotropyEnable;
    vk_info.maxAnisotropy = create_info->maxAnisotropy;
    vk_info.compareEnable = create_info->compareEnable;
    vk_info.compareOp = static_cast<VkCompareOp>(create_info->compareOp);
    vk_info.minLod = create_info->minLod;
    vk_info.maxLod = create_info->maxLod;
    vk_info.borderColor = static_cast<VkBorderColor>(create_info->borderColor);
    vk_info.unnormalizedCoordinates = create_info->unnormalizedCoordinates;

    VkResult result = functions.vkCreateSampler(device, &vk_info, nullptr, &sampler_);
    return result == VK_SUCCESS;
}

} // namespace kx