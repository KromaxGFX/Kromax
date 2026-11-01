/*
 * VkPipeline.cpp
 *
 * Vulkan pipeline and pipeline layout implementation.
 */

#include "VkPipeline.h"

#include <vector>

namespace kx {

// ============================================================================
// VkPipelineLayoutImpl
// ============================================================================

VkPipelineLayoutImpl::~VkPipelineLayoutImpl() {
    if (layout_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkPipelineLayoutImpl::VkPipelineLayoutImpl(VkPipelineLayoutImpl&& other) noexcept
    : layout_(other.layout_)
    , device_(other.device_)
    , set_layouts_(std::move(other.set_layouts_))
    , push_constants_(std::move(other.push_constants_)) {
    other.layout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkPipelineLayoutImpl& VkPipelineLayoutImpl::operator=(VkPipelineLayoutImpl&& other) noexcept {
    if (this != &other) {
        layout_ = other.layout_;
        device_ = other.device_;
        set_layouts_ = std::move(other.set_layouts_);
        push_constants_ = std::move(other.push_constants_);
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkPipelineLayoutImpl::initialize(VkFunctionTable& functions,
                                      VkDevice device,
                                      const KxPipelineLayoutCreateInfo* create_info) {
    device_ = device;

    // Convert KxDescriptorSetLayout to VkDescriptorSetLayout
    // Note: The create_info contains opaque pointers that need to be resolved
    // For now, we'll assume the set layouts are already VkDescriptorSetLayout handles
    set_layouts_.reserve(create_info->setLayoutCount);
    for (uint32_t i = 0; i < create_info->setLayoutCount; ++i) {
        // The opaque pointer is a VkDescriptorSetLayoutImpl*
        auto* layout_impl = reinterpret_cast<VkDescriptorSetLayoutImpl*>(create_info->pSetLayouts[i]);
        if (layout_impl) {
            set_layouts_.push_back(layout_impl->handle());
        }
    }

    // Convert push constant ranges
    push_constants_.reserve(create_info->pushConstantRangeCount);
    for (uint32_t i = 0; i < create_info->pushConstantRangeCount; ++i) {
        const KxPushConstantRange& kx_range = create_info->pPushConstantRanges[i];
        VkPushConstantRange vk_range{};
        vk_range.stageFlags = static_cast<VkShaderStageFlags>(kx_range.stageFlags);
        vk_range.offset = kx_range.offset;
        vk_range.size = kx_range.size;
        push_constants_.push_back(vk_range);
    }

    VkPipelineLayoutCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineLayoutCreateFlags>(create_info->flags);
    vk_info.setLayoutCount = static_cast<uint32_t>(set_layouts_.size());
    vk_info.pSetLayouts = set_layouts_.data();
    vk_info.pushConstantRangeCount = static_cast<uint32_t>(push_constants_.size());
    vk_info.pPushConstantRanges = push_constants_.data();

    VkResult result = functions.vkCreatePipelineLayout(device, &vk_info, nullptr, &layout_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkPipelineImpl
// ============================================================================

VkPipelineImpl::~VkPipelineImpl() {
    if (pipeline_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkPipelineImpl::VkPipelineImpl(VkPipelineImpl&& other) noexcept
    : pipeline_(other.pipeline_)
    , layout_(other.layout_)
    , device_(other.device_)
    , is_compute_(other.is_compute_) {
    other.pipeline_ = VK_NULL_HANDLE;
    other.layout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.is_compute_ = false;
}

VkPipelineImpl& VkPipelineImpl::operator=(VkPipelineImpl&& other) noexcept {
    if (this != &other) {
        pipeline_ = other.pipeline_;
        layout_ = other.layout_;
        device_ = other.device_;
        is_compute_ = other.is_compute_;
        other.pipeline_ = VK_NULL_HANDLE;
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.is_compute_ = false;
    }
    return *this;
}

bool VkPipelineImpl::initialize_graphics(VkFunctionTable& functions,
                                         VkDevice device,
                                         VkPipelineCache cache,
                                         const KxPipelineCreateInfo* create_info) {
    device_ = device;
    is_compute_ = false;

    // This is a simplified implementation
    // A full implementation would convert all the Kx pipeline create info to Vk
    // including vertex input, input assembly, viewport, rasterization, multisample,
    // depth stencil, color blend, dynamic state, etc.

    // For now, create a minimal pipeline
    VkGraphicsPipelineCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineCreateFlags>(create_info->flags);

    // Get pipeline layout
    auto* layout_impl = reinterpret_cast<VkPipelineLayoutImpl*>(create_info->layout);
    if (layout_impl) {
        layout_ = layout_impl->handle();
        vk_info.layout = layout_;
    }

    // TODO: Convert all pipeline stages, vertex input, etc.

    VkResult result = functions.vkCreateGraphicsPipelines(device, cache, 1, &vk_info, nullptr, &pipeline_);
    return result == VK_SUCCESS;
}

bool VkPipelineImpl::initialize_compute(VkFunctionTable& functions,
                                        VkDevice device,
                                        VkPipelineCache cache,
                                        const KxPipelineCreateInfo* create_info) {
    device_ = device;
    is_compute_ = true;

    // Get pipeline layout
    auto* layout_impl = reinterpret_cast<VkPipelineLayoutImpl*>(create_info->layout);
    if (layout_impl) {
        layout_ = layout_impl->handle();
    }

    VkComputePipelineCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineCreateFlags>(create_info->flags);
    vk_info.layout = layout_;

    // TODO: Set compute shader stage

    VkResult result = functions.vkCreateComputePipelines(device, cache, 1, &vk_info, nullptr, &pipeline_);
    return result == VK_SUCCESS;
}

} // namespace kx