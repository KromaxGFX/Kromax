/*
 * VkCommand.cpp
 *
 * Vulkan command pool and command buffer implementation.
 */

#include "VkCommand.h"

#include <vector>

namespace kx {

// ============================================================================
// VkCommandPoolImpl
// ============================================================================

VkCommandPoolImpl::~VkCommandPoolImpl() {
    if (pool_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkCommandPoolImpl::VkCommandPoolImpl(VkCommandPoolImpl&& other) noexcept
    : pool_(other.pool_)
    , device_(other.device_)
    , queue_family_index_(other.queue_family_index_) {
    other.pool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.queue_family_index_ = 0;
}

VkCommandPoolImpl& VkCommandPoolImpl::operator=(VkCommandPoolImpl&& other) noexcept {
    if (this != &other) {
        pool_ = other.pool_;
        device_ = other.device_;
        queue_family_index_ = other.queue_family_index_;
        other.pool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.queue_family_index_ = 0;
    }
    return *this;
}

bool VkCommandPoolImpl::initialize(VkFunctionTable& functions,
                                   VkDevice device,
                                   const KxCommandPoolCreateInfo* create_info) {
    device_ = device;
    queue_family_index_ = create_info->queueFamilyIndex;

    VkCommandPoolCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vk_info.flags = static_cast<VkCommandPoolCreateFlags>(create_info->flags);
    vk_info.queueFamilyIndex = create_info->queueFamilyIndex;

    VkResult result = functions.vkCreateCommandPool(device, &vk_info, nullptr, &pool_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkCommandBufferImpl
// ============================================================================

VkCommandBufferImpl::~VkCommandBufferImpl() {
    // Command buffers are freed via the pool, not individually destroyed
}

VkCommandBufferImpl::VkCommandBufferImpl(VkCommandBufferImpl&& other) noexcept
    : buffer_(other.buffer_)
    , pool_(other.pool_)
    , device_(other.device_)
    , level_(other.level_)
    , recording_(other.recording_)
    , inside_render_pass_(other.inside_render_pass_)
    , current_pipeline_layout_(other.current_pipeline_layout_)
    , current_pipeline_bind_point_(other.current_pipeline_bind_point_)
    , current_descriptor_sets_(std::move(other.current_descriptor_sets_))
    , first_descriptor_set_(other.first_descriptor_set_)
    , vertex_buffers_(std::move(other.vertex_buffers_))
    , index_buffer_(other.index_buffer_)
    , index_buffer_offset_(other.index_buffer_offset_)
    , index_type_(other.index_type_)
    , push_constants_(std::move(other.push_constants_))
    , viewports_(std::move(other.viewports_))
    , scissors_(std::move(other.scissors_))
    , line_width_(other.line_width_)
    , depth_bias_constant_factor_(other.depth_bias_constant_factor_)
    , depth_bias_clamp_(other.depth_bias_clamp_)
    , depth_bias_slope_factor_(other.depth_bias_slope_factor_)
    , blend_constants_(other.blend_constants_)
    , stencil_compare_mask_(other.stencil_compare_mask_)
    , stencil_write_mask_(other.stencil_write_mask_)
    , stencil_reference_(other.stencil_reference_) {
    other.buffer_ = VK_NULL_HANDLE;
    other.pool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.level_ = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    other.recording_ = false;
    other.inside_render_pass_ = false;
    other.current_pipeline_layout_ = VK_NULL_HANDLE;
    other.current_pipeline_bind_point_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    other.first_descriptor_set_ = 0;
    other.index_buffer_ = VK_NULL_HANDLE;
    other.index_buffer_offset_ = 0;
    other.index_type_ = VK_INDEX_TYPE_UINT16;
}

VkCommandBufferImpl& VkCommandBufferImpl::operator=(VkCommandBufferImpl&& other) noexcept {
    if (this != &other) {
        buffer_ = other.buffer_;
        pool_ = other.pool_;
        device_ = other.device_;
        level_ = other.level_;
        recording_ = other.recording_;
        inside_render_pass_ = other.inside_render_pass_;
        current_pipeline_layout_ = other.current_pipeline_layout_;
        current_pipeline_bind_point_ = other.current_pipeline_bind_point_;
        current_descriptor_sets_ = std::move(other.current_descriptor_sets_);
        first_descriptor_set_ = other.first_descriptor_set_;
        vertex_buffers_ = std::move(other.vertex_buffers_);
        index_buffer_ = other.index_buffer_;
        index_buffer_offset_ = other.index_buffer_offset_;
        index_type_ = other.index_type_;
        push_constants_ = std::move(other.push_constants_);
        viewports_ = std::move(other.viewports_);
        scissors_ = std::move(other.scissors_);
        line_width_ = other.line_width_;
        depth_bias_constant_factor_ = other.depth_bias_constant_factor_;
        depth_bias_clamp_ = other.depth_bias_clamp_;
        depth_bias_slope_factor_ = other.depth_bias_slope_factor_;
        blend_constants_ = other.blend_constants_;
        stencil_compare_mask_ = other.stencil_compare_mask_;
        stencil_write_mask_ = other.stencil_write_mask_;
        stencil_reference_ = other.stencil_reference_;
        other.buffer_ = VK_NULL_HANDLE;
        other.pool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.level_ = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
        other.recording_ = false;
        other.inside_render_pass_ = false;
        other.current_pipeline_layout_ = VK_NULL_HANDLE;
        other.current_pipeline_bind_point_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
        other.first_descriptor_set_ = 0;
        other.index_buffer_ = VK_NULL_HANDLE;
        other.index_buffer_offset_ = 0;
        other.index_type_ = VK_INDEX_TYPE_UINT16;
    }
    return *this;
}

bool VkCommandBufferImpl::initialize(VkFunctionTable& functions,
                                     VkDevice device,
                                     VkCommandPool pool,
                                     KxCommandBufferLevel level) {
    device_ = device;
    pool_ = pool;
    level_ = level;

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pool;
    alloc_info.level = (level == KX_COMMAND_BUFFER_LEVEL_PRIMARY) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;

    VkResult result = functions.vkAllocateCommandBuffers(device, &alloc_info, &buffer_);
    return result == VK_SUCCESS;
}

} // namespace kx