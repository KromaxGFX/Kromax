/*
 * VkDescriptor.cpp
 *
 * Vulkan descriptor set layout, pool, and set implementation.
 */

#include "VkDescriptor.h"

#include <vector>

namespace kx {

// ============================================================================
// VkDescriptorSetLayoutImpl
// ============================================================================

VkDescriptorSetLayoutImpl::~VkDescriptorSetLayoutImpl() {
    if (layout_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkDescriptorSetLayoutImpl::VkDescriptorSetLayoutImpl(VkDescriptorSetLayoutImpl&& other) noexcept
    : layout_(other.layout_)
    , device_(other.device_)
    , bindings_(std::move(other.bindings_)) {
    other.layout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkDescriptorSetLayoutImpl& VkDescriptorSetLayoutImpl::operator=(VkDescriptorSetLayoutImpl&& other) noexcept {
    if (this != &other) {
        layout_ = other.layout_;
        device_ = other.device_;
        bindings_ = std::move(other.bindings_);
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkDescriptorSetLayoutImpl::initialize(VkFunctionTable& functions,
                                           VkDevice device,
                                           const KxDescriptorSetLayoutCreateInfo* create_info) {
    device_ = device;

    bindings_.reserve(create_info->bindingCount);
    for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
        const KxDescriptorSetLayoutBinding& kx_binding = create_info->pBindings[i];
        VkDescriptorSetLayoutBinding vk_binding{};
        vk_binding.binding = kx_binding.binding;
        vk_binding.descriptorType = static_cast<VkDescriptorType>(kx_binding.descriptorType);
        vk_binding.descriptorCount = kx_binding.descriptorCount;
        vk_binding.stageFlags = static_cast<VkShaderStageFlags>(kx_binding.stageFlags);
        vk_binding.pImmutableSamplers = nullptr; // TODO: Support immutable samplers
        bindings_.push_back(vk_binding);
    }

    VkDescriptorSetLayoutCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vk_info.flags = static_cast<VkDescriptorSetLayoutCreateFlags>(create_info->flags);
    vk_info.bindingCount = static_cast<uint32_t>(bindings_.size());
    vk_info.pBindings = bindings_.data();

    VkResult result = functions.vkCreateDescriptorSetLayout(device, &vk_info, nullptr, &layout_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkDescriptorPoolImpl
// ============================================================================

VkDescriptorPoolImpl::~VkDescriptorPoolImpl() {
    if (pool_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkDescriptorPoolImpl::VkDescriptorPoolImpl(VkDescriptorPoolImpl&& other) noexcept
    : pool_(other.pool_)
    , device_(other.device_) {
    other.pool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkDescriptorPoolImpl& VkDescriptorPoolImpl::operator=(VkDescriptorPoolImpl&& other) noexcept {
    if (this != &other) {
        pool_ = other.pool_;
        device_ = other.device_;
        other.pool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkDescriptorPoolImpl::initialize(VkFunctionTable& functions,
                                      VkDevice device,
                                      const KxDescriptorPoolCreateInfo* create_info) {
    device_ = device;

    std::vector<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(create_info->poolSizeCount);
    for (uint32_t i = 0; i < create_info->poolSizeCount; ++i) {
        const KxDescriptorPoolSize& kx_size = create_info->pPoolSizes[i];
        VkDescriptorPoolSize vk_size{};
        vk_size.type = static_cast<VkDescriptorType>(kx_size.type);
        vk_size.descriptorCount = kx_size.descriptorCount;
        pool_sizes.push_back(vk_size);
    }

    VkDescriptorPoolCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vk_info.flags = static_cast<VkDescriptorPoolCreateFlags>(create_info->flags);
    vk_info.maxSets = create_info->maxSets;
    vk_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    vk_info.pPoolSizes = pool_sizes.data();

    VkResult result = functions.vkCreateDescriptorPool(device, &vk_info, nullptr, &pool_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkDescriptorSetImpl
// ============================================================================

VkDescriptorSetImpl::~VkDescriptorSetImpl() {
    // Descriptor sets are freed via the pool, not individually destroyed
}

VkDescriptorSetImpl::VkDescriptorSetImpl(VkDescriptorSetImpl&& other) noexcept
    : set_(other.set_)
    , layout_(other.layout_)
    , pool_(other.pool_)
    , device_(other.device_) {
    other.set_ = VK_NULL_HANDLE;
    other.layout_ = VK_NULL_HANDLE;
    other.pool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkDescriptorSetImpl& VkDescriptorSetImpl::operator=(VkDescriptorSetImpl&& other) noexcept {
    if (this != &other) {
        set_ = other.set_;
        layout_ = other.layout_;
        pool_ = other.pool_;
        device_ = other.device_;
        other.set_ = VK_NULL_HANDLE;
        other.layout_ = VK_NULL_HANDLE;
        other.pool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

void VkDescriptorSetImpl::update(VkFunctionTable& functions,
                                 uint32_t write_count,
                                 const KxWriteDescriptorSet* writes,
                                 uint32_t copy_count,
                                 const KxCopyDescriptorSet* copies) {
    std::vector<VkWriteDescriptorSet> vk_writes;
    vk_writes.reserve(write_count);

    // We need a temporary storage for VkDescriptorImageInfo and VkDescriptorBufferInfo
    // since the Kx structures point to data that may be temporary
    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkBufferView> texel_buffer_views;

    for (uint32_t i = 0; i < write_count; ++i) {
        const KxWriteDescriptorSet& kx_write = writes[i];
        VkWriteDescriptorSet vk_write{};
        vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vk_write.dstSet = set_;
        vk_write.dstBinding = kx_write.dstBinding;
        vk_write.dstArrayElement = kx_write.dstArrayElement;
        vk_write.descriptorCount = kx_write.descriptorCount;
        vk_write.descriptorType = static_cast<VkDescriptorType>(kx_write.descriptorType);

        if (kx_write.pImageInfo) {
            image_infos.reserve(image_infos.size() + kx_write.descriptorCount);
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                const KxDescriptorImageInfo& kx_img = kx_write.pImageInfo[j];
                VkDescriptorImageInfo vk_img{};
                vk_img.sampler = reinterpret_cast<VkSampler>(kx_img.sampler);
                vk_img.imageView = reinterpret_cast<VkImageView>(kx_img.imageView);
                vk_img.imageLayout = static_cast<VkImageLayout>(kx_img.imageLayout);
                image_infos.push_back(vk_img);
            }
            vk_write.pImageInfo = &image_infos[image_infos.size() - kx_write.descriptorCount];
        }

        if (kx_write.pBufferInfo) {
            buffer_infos.reserve(buffer_infos.size() + kx_write.descriptorCount);
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                const KxDescriptorBufferInfo& kx_buf = kx_write.pBufferInfo[j];
                VkDescriptorBufferInfo vk_buf{};
                vk_buf.buffer = reinterpret_cast<VkBuffer>(kx_buf.buffer);
                vk_buf.offset = kx_buf.offset;
                vk_buf.range = kx_buf.range;
                buffer_infos.push_back(vk_buf);
            }
            vk_write.pBufferInfo = &buffer_infos[buffer_infos.size() - kx_write.descriptorCount];
        }

        if (kx_write.pTexelBufferView) {
            texel_buffer_views.reserve(texel_buffer_views.size() + kx_write.descriptorCount);
            const VkBufferView* texel_views = reinterpret_cast<const VkBufferView*>(kx_write.pTexelBufferView);
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                texel_buffer_views.push_back(texel_views[j]);
            }
            vk_write.pTexelBufferView = &texel_buffer_views[texel_buffer_views.size() - kx_write.descriptorCount];
        }

        vk_writes.push_back(vk_write);
    }

    if (!vk_writes.empty()) {
        functions.vkUpdateDescriptorSets(device_,
                                         static_cast<uint32_t>(vk_writes.size()),
                                         vk_writes.data(),
                                         0, nullptr);
    }

    // Handle copies if needed
    if (copy_count > 0) {
        std::vector<VkCopyDescriptorSet> vk_copies;
        vk_copies.reserve(copy_count);
        for (uint32_t i = 0; i < copy_count; ++i) {
            const KxCopyDescriptorSet& kx_copy = copies[i];
            VkCopyDescriptorSet vk_copy{};
            vk_copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
            vk_copy.srcSet = reinterpret_cast<VkDescriptorSet>(kx_copy.srcSet);
            vk_copy.srcBinding = kx_copy.srcBinding;
            vk_copy.srcArrayElement = kx_copy.srcArrayElement;
            vk_copy.dstSet = set_; // Destination is this set
            vk_copy.dstBinding = kx_copy.dstBinding;
            vk_copy.dstArrayElement = kx_copy.dstArrayElement;
            vk_copy.descriptorCount = kx_copy.descriptorCount;
            vk_copies.push_back(vk_copy);
        }
        // Note: vkUpdateDescriptorSets handles both writes and copies in one call
        // We'd need to combine them
    }
}

} // namespace kx