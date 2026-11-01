/*
 * VkCommand.cpp
 *
 * Vulkan command pool and command buffer implementation.
 */

#include "VkCommand.h"

#include <vector>
#include <cstring>

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
    , functions_(other.functions_)
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
    other.functions_ = nullptr;
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
        functions_ = other.functions_;
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
        other.functions_ = nullptr;
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
    functions_ = &functions;

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = pool;
    alloc_info.level = (level == KX_COMMAND_BUFFER_LEVEL_PRIMARY) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 1;

    VkResult result = functions.vkAllocateCommandBuffers(device, &alloc_info, &buffer_);
    return result == VK_SUCCESS;
}

// ============================================================================
// RECORDING METHODS IMPLEMENTATION
// ============================================================================

void VkCommandBufferImpl::cmd_bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) {
    if (!functions_ || !recording_) return;
    current_pipeline_bind_point_ = bind_point;
    functions_->vkCmdBindPipeline(buffer_, bind_point, pipeline);
}

void VkCommandBufferImpl::cmd_bind_vertex_buffers(uint32_t first_binding, uint32_t binding_count, const VkBuffer* buffers, const VkDeviceSize* offsets) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBindVertexBuffers(buffer_, first_binding, binding_count, buffers, offsets);
    
    // Update internal state tracking
    if (first_binding + binding_count > vertex_buffers_.size()) {
        vertex_buffers_.resize(first_binding + binding_count);
    }
    for (uint32_t i = 0; i < binding_count; ++i) {
        vertex_buffers_[first_binding + i] = {buffers[i], offsets ? offsets[i] : 0};
    }
}

void VkCommandBufferImpl::cmd_bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBindIndexBuffer(buffer_, buffer, offset, index_type);
    index_buffer_ = buffer;
    index_buffer_offset_ = offset;
    index_type_ = index_type;
}

void VkCommandBufferImpl::cmd_bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t first_set, uint32_t set_count, const VkDescriptorSet* sets, uint32_t dynamic_offset_count, const uint32_t* dynamic_offsets) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBindDescriptorSets(buffer_, bind_point, layout, first_set, set_count, sets, dynamic_offset_count, dynamic_offsets);
    
    // Update internal state
    current_pipeline_bind_point_ = bind_point;
    current_pipeline_layout_ = layout;
    if (first_set + set_count > current_descriptor_sets_.size()) {
        current_descriptor_sets_.resize(first_set + set_count);
    }
    for (uint32_t i = 0; i < set_count; ++i) {
        current_descriptor_sets_[first_set + i] = sets[i];
    }
    first_descriptor_set_ = first_set;
}

void VkCommandBufferImpl::cmd_push_constants(VkPipelineLayout layout, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdPushConstants(buffer_, layout, stages, offset, size, data);
    
    // Update internal state
    if (offset + size > push_constants_.size()) {
        push_constants_.resize(offset + size);
    }
    std::memcpy(push_constants_.data() + offset, data, size);
}

void VkCommandBufferImpl::cmd_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDraw(buffer_, vertex_count, instance_count, first_vertex, first_instance);
}

void VkCommandBufferImpl::cmd_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDrawIndexed(buffer_, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void VkCommandBufferImpl::cmd_draw_indirect(VkBuffer buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDrawIndirect(buffer_, buffer, offset, draw_count, stride);
}

void VkCommandBufferImpl::cmd_draw_indexed_indirect(VkBuffer buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDrawIndexedIndirect(buffer_, buffer, offset, draw_count, stride);
}

void VkCommandBufferImpl::cmd_dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDispatch(buffer_, group_count_x, group_count_y, group_count_z);
}

void VkCommandBufferImpl::cmd_dispatch_indirect(VkBuffer buffer, VkDeviceSize offset) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdDispatchIndirect(buffer_, buffer, offset);
}

void VkCommandBufferImpl::cmd_begin_render_pass(const VkRenderPassBeginInfo* render_pass_begin, VkSubpassContents contents) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBeginRenderPass(buffer_, render_pass_begin, contents);
    inside_render_pass_ = true;
}

void VkCommandBufferImpl::cmd_begin_render_pass(VkRenderPass render_pass, VkFramebuffer framebuffer,
                                                const VkRect2D& render_area, const VkClearValue* clear_values,
                                                uint32_t clear_value_count, VkSubpassContents contents) {
    if (!functions_ || !recording_) return;
    
    VkRenderPassBeginInfo rp_begin{};
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass = render_pass;
    rp_begin.framebuffer = framebuffer;
    rp_begin.renderArea = render_area;
    rp_begin.clearValueCount = clear_value_count;
    rp_begin.pClearValues = clear_values;
    
    functions_->vkCmdBeginRenderPass(buffer_, &rp_begin, contents);
    inside_render_pass_ = true;
}

void VkCommandBufferImpl::cmd_begin_render_pass(VkRenderPass render_pass, VkFramebuffer framebuffer,
                                                const KxRect2D& render_area, const KxClearValue* clear_values,
                                                uint32_t clear_value_count, KxSubpassContents contents) {
    if (!functions_ || !recording_) return;
    
    VkRenderPassBeginInfo rp_begin{};
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass = render_pass;
    rp_begin.framebuffer = framebuffer;
    rp_begin.renderArea = *reinterpret_cast<const VkRect2D*>(&render_area);
    rp_begin.clearValueCount = clear_value_count;
    rp_begin.pClearValues = reinterpret_cast<const VkClearValue*>(clear_values);
    
    functions_->vkCmdBeginRenderPass(buffer_, &rp_begin, static_cast<VkSubpassContents>(contents));
    inside_render_pass_ = true;
}

void VkCommandBufferImpl::cmd_end_render_pass() {
    if (!functions_ || !recording_) return;
    functions_->vkCmdEndRenderPass(buffer_);
    inside_render_pass_ = false;
}

void VkCommandBufferImpl::cmd_next_subpass(VkSubpassContents contents) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdNextSubpass(buffer_, contents);
}

bool VkCommandBufferImpl::reset() {
    if (!functions_) return false;
    VkResult result = functions_->vkResetCommandBuffer(buffer_, 0);
    return result == VK_SUCCESS;
}

bool VkCommandBufferImpl::begin(const KxCommandBufferBeginInfo* begin_info) {
    if (!functions_ || !begin_info) return false;
    
    VkCommandBufferBeginInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_info.flags = static_cast<VkCommandBufferUsageFlags>(begin_info->flags);
    vk_info.pInheritanceInfo = nullptr; // TODO: Support secondary command buffers
    
    VkResult result = functions_->vkBeginCommandBuffer(buffer_, &vk_info);
    if (result == VK_SUCCESS) {
        recording_ = true;
    }
    return result == VK_SUCCESS;
}

bool VkCommandBufferImpl::end() {
    if (!functions_) return false;
    VkResult result = functions_->vkEndCommandBuffer(buffer_);
    if (result == VK_SUCCESS) {
        recording_ = false;
    }
    return result == VK_SUCCESS;
}

void VkCommandBufferImpl::cmd_copy_buffer(VkBuffer src, VkBuffer dst, uint32_t region_count, const VkBufferCopy* regions) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdCopyBuffer(buffer_, src, dst, region_count, regions);
}

void VkCommandBufferImpl::cmd_copy_image(VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkImageCopy* regions) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdCopyImage(buffer_, src, src_layout, dst, dst_layout, region_count, regions);
}

void VkCommandBufferImpl::cmd_copy_buffer_to_image(VkBuffer src, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkBufferImageCopy* regions) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdCopyBufferToImage(buffer_, src, dst, dst_layout, region_count, regions);
}

void VkCommandBufferImpl::cmd_copy_image_to_buffer(VkImage src, VkImageLayout src_layout, VkBuffer dst, uint32_t region_count, const VkBufferImageCopy* regions) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdCopyImageToBuffer(buffer_, src, src_layout, dst, region_count, regions);
}

void VkCommandBufferImpl::cmd_blit_image(VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkImageBlit* regions, VkFilter filter) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBlitImage(buffer_, src, src_layout, dst, dst_layout, region_count, regions, filter);
}

void VkCommandBufferImpl::cmd_clear_color_image(VkImage image, VkImageLayout layout, const VkClearColorValue* color, uint32_t range_count, const VkImageSubresourceRange* ranges) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdClearColorImage(buffer_, image, layout, color, range_count, ranges);
}

void VkCommandBufferImpl::cmd_clear_depth_stencil_image(VkImage image, VkImageLayout layout, const VkClearDepthStencilValue* depth_stencil, uint32_t range_count, const VkImageSubresourceRange* ranges) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdClearDepthStencilImage(buffer_, image, layout, depth_stencil, range_count, ranges);
}

void VkCommandBufferImpl::cmd_fill_buffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, uint32_t data) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdFillBuffer(buffer_, buffer, offset, size, data);
}

void VkCommandBufferImpl::cmd_update_buffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, const void* data) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdUpdateBuffer(buffer_, buffer, offset, size, data);
}

void VkCommandBufferImpl::cmd_pipeline_barrier(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkDependencyFlags flags, 
                                               uint32_t memory_barrier_count, const VkMemoryBarrier* memory_barriers,
                                               uint32_t buffer_memory_barrier_count, const VkBufferMemoryBarrier* buffer_memory_barriers,
                                               uint32_t image_memory_barrier_count, const VkImageMemoryBarrier* image_memory_barriers) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdPipelineBarrier(buffer_, src_stage, dst_stage, flags,
                                     memory_barrier_count, memory_barriers,
                                     buffer_memory_barrier_count, buffer_memory_barriers,
                                     image_memory_barrier_count, image_memory_barriers);
}

// Dynamic state setters
void VkCommandBufferImpl::cmd_set_viewport(uint32_t first_viewport, uint32_t viewport_count, const VkViewport* viewports) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetViewport(buffer_, first_viewport, viewport_count, viewports);
    if (first_viewport + viewport_count > viewports_.size()) {
        viewports_.resize(first_viewport + viewport_count);
    }
    for (uint32_t i = 0; i < viewport_count; ++i) {
        viewports_[first_viewport + i] = viewports[i];
    }
}

void VkCommandBufferImpl::cmd_set_scissor(uint32_t first_scissor, uint32_t scissor_count, const VkRect2D* scissors) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetScissor(buffer_, first_scissor, scissor_count, scissors);
    if (first_scissor + scissor_count > scissors_.size()) {
        scissors_.resize(first_scissor + scissor_count);
    }
    for (uint32_t i = 0; i < scissor_count; ++i) {
        scissors_[first_scissor + i] = scissors[i];
    }
}

void VkCommandBufferImpl::cmd_set_line_width(float line_width) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetLineWidth(buffer_, line_width);
    line_width_ = line_width;
}

void VkCommandBufferImpl::cmd_set_depth_bias(float constant_factor, float clamp, float slope_factor) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetDepthBias(buffer_, constant_factor, clamp, slope_factor);
    depth_bias_constant_factor_ = constant_factor;
    depth_bias_clamp_ = clamp;
    depth_bias_slope_factor_ = slope_factor;
}

void VkCommandBufferImpl::cmd_set_blend_constants(const float blend_constants[4]) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetBlendConstants(buffer_, blend_constants);
    blend_constants_[0] = blend_constants[0];
    blend_constants_[1] = blend_constants[1];
    blend_constants_[2] = blend_constants[2];
    blend_constants_[3] = blend_constants[3];
}

void VkCommandBufferImpl::cmd_set_depth_bounds(float min_depth_bounds, float max_depth_bounds) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetDepthBounds(buffer_, min_depth_bounds, max_depth_bounds);
}

void VkCommandBufferImpl::cmd_set_stencil_compare_mask(VkStencilFaceFlags face_mask, uint32_t compare_mask) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetStencilCompareMask(buffer_, face_mask, compare_mask);
    if (face_mask & VK_STENCIL_FACE_FRONT_BIT) stencil_compare_mask_ = compare_mask;
    if (face_mask & VK_STENCIL_FACE_BACK_BIT) stencil_compare_mask_ = compare_mask;
}

void VkCommandBufferImpl::cmd_set_stencil_write_mask(VkStencilFaceFlags face_mask, uint32_t write_mask) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetStencilWriteMask(buffer_, face_mask, write_mask);
    if (face_mask & VK_STENCIL_FACE_FRONT_BIT) stencil_write_mask_ = write_mask;
    if (face_mask & VK_STENCIL_FACE_BACK_BIT) stencil_write_mask_ = write_mask;
}

void VkCommandBufferImpl::cmd_set_stencil_reference(VkStencilFaceFlags face_mask, uint32_t reference) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdSetStencilReference(buffer_, face_mask, reference);
    if (face_mask & VK_STENCIL_FACE_FRONT_BIT) stencil_reference_ = reference;
    if (face_mask & VK_STENCIL_FACE_BACK_BIT) stencil_reference_ = reference;
}

// Extended dynamic state (VK_EXT_extended_dynamic_state)
void VkCommandBufferImpl::cmd_set_cull_mode(VkCullModeFlags cull_mode) {
    if (!functions_ || !recording_) return;
    // Check if extension function is available
    if (functions_->vkCmdSetCullModeEXT) {
        functions_->vkCmdSetCullModeEXT(buffer_, cull_mode);
    }
}

void VkCommandBufferImpl::cmd_set_front_face(VkFrontFace front_face) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetFrontFaceEXT) {
        functions_->vkCmdSetFrontFaceEXT(buffer_, front_face);
    }
}

void VkCommandBufferImpl::cmd_set_primitive_topology(VkPrimitiveTopology primitive_topology) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetPrimitiveTopologyEXT) {
        functions_->vkCmdSetPrimitiveTopologyEXT(buffer_, primitive_topology);
    }
}

void VkCommandBufferImpl::cmd_set_viewport_with_count(uint32_t viewport_count, const VkViewport* viewports) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetViewportWithCountEXT) {
        functions_->vkCmdSetViewportWithCountEXT(buffer_, viewport_count, viewports);
    }
}

void VkCommandBufferImpl::cmd_set_scissor_with_count(uint32_t scissor_count, const VkRect2D* scissors) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetScissorWithCountEXT) {
        functions_->vkCmdSetScissorWithCountEXT(buffer_, scissor_count, scissors);
    }
}

void VkCommandBufferImpl::cmd_set_depth_test_enable(VkBool32 depth_test_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthTestEnableEXT) {
        functions_->vkCmdSetDepthTestEnableEXT(buffer_, depth_test_enable);
    }
}

void VkCommandBufferImpl::cmd_set_depth_write_enable(VkBool32 depth_write_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthWriteEnableEXT) {
        functions_->vkCmdSetDepthWriteEnableEXT(buffer_, depth_write_enable);
    }
}

void VkCommandBufferImpl::cmd_set_depth_compare_op(VkCompareOp depth_compare_op) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthCompareOpEXT) {
        functions_->vkCmdSetDepthCompareOpEXT(buffer_, depth_compare_op);
    }
}

void VkCommandBufferImpl::cmd_set_depth_bounds_test_enable(VkBool32 depth_bounds_test_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthBoundsTestEnableEXT) {
        functions_->vkCmdSetDepthBoundsTestEnableEXT(buffer_, depth_bounds_test_enable);
    }
}

void VkCommandBufferImpl::cmd_set_stencil_test_enable(VkBool32 stencil_test_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetStencilTestEnableEXT) {
        functions_->vkCmdSetStencilTestEnableEXT(buffer_, stencil_test_enable);
    }
}

void VkCommandBufferImpl::cmd_set_stencil_op(VkStencilFaceFlags face_mask, VkStencilOp fail_op, VkStencilOp pass_op, VkStencilOp depth_fail_op, VkCompareOp compare_op) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetStencilOpEXT) {
        functions_->vkCmdSetStencilOpEXT(buffer_, face_mask, fail_op, pass_op, depth_fail_op, compare_op);
    }
}

void VkCommandBufferImpl::cmd_set_primitive_restart_enable(VkBool32 primitive_restart_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetPrimitiveRestartEnableEXT) {
        functions_->vkCmdSetPrimitiveRestartEnableEXT(buffer_, primitive_restart_enable);
    }
}

void VkCommandBufferImpl::cmd_set_rasterizer_discard_enable(VkBool32 rasterizer_discard_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetRasterizerDiscardEnableEXT) {
        functions_->vkCmdSetRasterizerDiscardEnableEXT(buffer_, rasterizer_discard_enable);
    }
}

void VkCommandBufferImpl::cmd_set_depth_bias_enable(VkBool32 depth_bias_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthBiasEnableEXT) {
        functions_->vkCmdSetDepthBiasEnableEXT(buffer_, depth_bias_enable);
    }
}

void VkCommandBufferImpl::cmd_set_logic_op(VkLogicOp logic_op) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetLogicOpEXT) {
        functions_->vkCmdSetLogicOpEXT(buffer_, logic_op);
    }
}

// Dynamic Rendering (VK_KHR_dynamic_rendering)
void VkCommandBufferImpl::cmd_begin_rendering(const VkRenderingInfoKHR* rendering_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdBeginRenderingKHR) {
        functions_->vkCmdBeginRenderingKHR(buffer_, rendering_info);
    }
}

void VkCommandBufferImpl::cmd_end_rendering() {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdEndRenderingKHR) {
        functions_->vkCmdEndRenderingKHR(buffer_);
    }
}

// Synchronization 2 (VK_KHR_synchronization2)
void VkCommandBufferImpl::cmd_pipeline_barrier2(const VkDependencyInfoKHR* dependency_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdPipelineBarrier2KHR) {
        functions_->vkCmdPipelineBarrier2KHR(buffer_, dependency_info);
    }
}

void VkCommandBufferImpl::cmd_write_timestamp2(VkPipelineStageFlags2KHR stage, VkQueryPool query_pool, uint32_t query) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdWriteTimestamp2KHR) {
        functions_->vkCmdWriteTimestamp2KHR(buffer_, stage, query_pool, query);
    }
}

void VkCommandBufferImpl::cmd_set_event2(VkEvent event, const VkDependencyInfoKHR* dependency_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetEvent2KHR) {
        functions_->vkCmdSetEvent2KHR(buffer_, event, dependency_info);
    }
}

void VkCommandBufferImpl::cmd_reset_event2(VkEvent event, VkPipelineStageFlags2KHR stage_mask) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdResetEvent2KHR) {
        functions_->vkCmdResetEvent2KHR(buffer_, event, stage_mask);
    }
}

void VkCommandBufferImpl::cmd_wait_events2(uint32_t event_count, const VkEvent* events, const VkDependencyInfoKHR* dependency_infos) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdWaitEvents2KHR) {
        functions_->vkCmdWaitEvents2KHR(buffer_, event_count, events, dependency_infos);
    }
}

// Copy Commands v2 (VK_KHR_copy_commands2)
void VkCommandBufferImpl::cmd_copy_buffer2(const VkCopyBufferInfo2KHR* copy_buffer_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdCopyBuffer2KHR) {
        functions_->vkCmdCopyBuffer2KHR(buffer_, copy_buffer_info);
    }
}

void VkCommandBufferImpl::cmd_copy_image2(const VkCopyImageInfo2KHR* copy_image_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdCopyImage2KHR) {
        functions_->vkCmdCopyImage2KHR(buffer_, copy_image_info);
    }
}

void VkCommandBufferImpl::cmd_blit_image2(const VkBlitImageInfo2KHR* blit_image_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdBlitImage2KHR) {
        functions_->vkCmdBlitImage2KHR(buffer_, blit_image_info);
    }
}

void VkCommandBufferImpl::cmd_copy_buffer_to_image2(const VkCopyBufferToImageInfo2KHR* copy_buffer_to_image_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdCopyBufferToImage2KHR) {
        functions_->vkCmdCopyBufferToImage2KHR(buffer_, copy_buffer_to_image_info);
    }
}

void VkCommandBufferImpl::cmd_copy_image_to_buffer2(const VkCopyImageToBufferInfo2KHR* copy_image_to_buffer_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdCopyImageToBuffer2KHR) {
        functions_->vkCmdCopyImageToBuffer2KHR(buffer_, copy_image_to_buffer_info);
    }
}

void VkCommandBufferImpl::cmd_resolve_image2(const VkResolveImageInfo2KHR* resolve_image_info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdResolveImage2KHR) {
        functions_->vkCmdResolveImage2KHR(buffer_, resolve_image_info);
    }
}

// Query
void VkCommandBufferImpl::cmd_begin_query(VkQueryPool query_pool, uint32_t query, VkQueryControlFlags flags) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdBeginQuery(buffer_, query_pool, query, flags);
}

void VkCommandBufferImpl::cmd_end_query(VkQueryPool query_pool, uint32_t query) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdEndQuery(buffer_, query_pool, query);
}

void VkCommandBufferImpl::cmd_reset_query_pool(VkQueryPool query_pool, uint32_t first_query, uint32_t query_count) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdResetQueryPool(buffer_, query_pool, first_query, query_count);
}

void VkCommandBufferImpl::cmd_write_timestamp(VkPipelineStageFlagBits pipeline_stage, VkQueryPool query_pool, uint32_t query) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdWriteTimestamp(buffer_, pipeline_stage, query_pool, query);
}

// Conditional rendering
void VkCommandBufferImpl::cmd_begin_conditional_rendering(VkConditionalRenderingBeginInfoEXT* info) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdBeginConditionalRenderingEXT) {
        functions_->vkCmdBeginConditionalRenderingEXT(buffer_, info);
    }
}

void VkCommandBufferImpl::cmd_end_conditional_rendering() {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdEndConditionalRenderingEXT) {
        functions_->vkCmdEndConditionalRenderingEXT(buffer_);
    }
}

// Execute commands
void VkCommandBufferImpl::cmd_execute_commands(uint32_t command_buffer_count, const VkCommandBuffer* command_buffers) {
    if (!functions_ || !recording_) return;
    functions_->vkCmdExecuteCommands(buffer_, command_buffer_count, command_buffers);
}

// Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
void VkCommandBufferImpl::cmd_set_depth_clamp_enable(VkBool32 depth_clamp_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthClampEnableEXT) {
        functions_->vkCmdSetDepthClampEnableEXT(buffer_, depth_clamp_enable);
    }
}

void VkCommandBufferImpl::cmd_set_polygon_mode(VkPolygonMode polygon_mode) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetPolygonModeEXT) {
        functions_->vkCmdSetPolygonModeEXT(buffer_, polygon_mode);
    }
}

void VkCommandBufferImpl::cmd_set_rasterization_samples(VkSampleCountFlagBits rasterization_samples) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetRasterizationSamplesEXT) {
        functions_->vkCmdSetRasterizationSamplesEXT(buffer_, rasterization_samples);
    }
}

void VkCommandBufferImpl::cmd_set_sample_mask(VkSampleMask sample_mask) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetSampleMaskEXT) {
        functions_->vkCmdSetSampleMaskEXT(buffer_, VK_SAMPLE_COUNT_1_BIT, &sample_mask);
    }
}

void VkCommandBufferImpl::cmd_set_alpha_to_coverage_enable(VkBool32 alpha_to_coverage_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetAlphaToCoverageEnableEXT) {
        functions_->vkCmdSetAlphaToCoverageEnableEXT(buffer_, alpha_to_coverage_enable);
    }
}

void VkCommandBufferImpl::cmd_set_alpha_to_one_enable(VkBool32 alpha_to_one_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetAlphaToOneEnableEXT) {
        functions_->vkCmdSetAlphaToOneEnableEXT(buffer_, alpha_to_one_enable);
    }
}

void VkCommandBufferImpl::cmd_set_logic_op_enable(VkBool32 logic_op_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetLogicOpEnableEXT) {
        functions_->vkCmdSetLogicOpEnableEXT(buffer_, logic_op_enable);
    }
}

void VkCommandBufferImpl::cmd_set_color_blend_enable(uint32_t first_attachment, uint32_t attachment_count, const VkBool32* color_blend_enables) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetColorBlendEnableEXT) {
        functions_->vkCmdSetColorBlendEnableEXT(buffer_, first_attachment, attachment_count, color_blend_enables);
    }
}

void VkCommandBufferImpl::cmd_set_color_blend_equation(uint32_t first_attachment, uint32_t attachment_count, const VkColorBlendEquationEXT* color_blend_equations) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetColorBlendEquationEXT) {
        functions_->vkCmdSetColorBlendEquationEXT(buffer_, first_attachment, attachment_count, color_blend_equations);
    }
}

void VkCommandBufferImpl::cmd_set_color_write_mask(uint32_t first_attachment, uint32_t attachment_count, const VkColorComponentFlags* color_write_masks) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetColorWriteMaskEXT) {
        functions_->vkCmdSetColorWriteMaskEXT(buffer_, first_attachment, attachment_count, color_write_masks);
    }
}

void VkCommandBufferImpl::cmd_set_tessellation_domain_origin(VkTessellationDomainOriginKHR domain_origin) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetTessellationDomainOriginEXT) {
        functions_->vkCmdSetTessellationDomainOriginEXT(buffer_, domain_origin);
    }
}

void VkCommandBufferImpl::cmd_set_rasterization_stream(uint32_t rasterization_stream) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetRasterizationStreamEXT) {
        functions_->vkCmdSetRasterizationStreamEXT(buffer_, rasterization_stream);
    }
}

void VkCommandBufferImpl::cmd_set_conservative_rasterization_mode(VkConservativeRasterizationModeEXT conservative_rasterization_mode) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetConservativeRasterizationModeEXT) {
        functions_->vkCmdSetConservativeRasterizationModeEXT(buffer_, conservative_rasterization_mode);
    }
}

void VkCommandBufferImpl::cmd_set_extra_primitive_overestimation_size(float extra_primitive_overestimation_size) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetExtraPrimitiveOverestimationSizeEXT) {
        functions_->vkCmdSetExtraPrimitiveOverestimationSizeEXT(buffer_, extra_primitive_overestimation_size);
    }
}

void VkCommandBufferImpl::cmd_set_depth_clip_enable(VkBool32 depth_clip_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthClipEnableEXT) {
        functions_->vkCmdSetDepthClipEnableEXT(buffer_, depth_clip_enable);
    }
}

void VkCommandBufferImpl::cmd_set_depth_clip_negative_one_to_one(VkBool32 negative_one_to_one) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetDepthClipNegativeOneToOneEXT) {
        functions_->vkCmdSetDepthClipNegativeOneToOneEXT(buffer_, negative_one_to_one);
    }
}

void VkCommandBufferImpl::cmd_set_viewport_w_scaling_enable_nv(VkBool32 viewport_w_scaling_enable) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetViewportWScalingEnableNV) {
        functions_->vkCmdSetViewportWScalingEnableNV(buffer_, viewport_w_scaling_enable);
    }
}

void VkCommandBufferImpl::cmd_set_viewport_swizzle_nv(uint32_t first_viewport, uint32_t viewport_count, const VkViewportSwizzleNV* viewport_swizzles) {
    if (!functions_ || !recording_) return;
    if (functions_->vkCmdSetViewportSwizzleNV) {
        functions_->vkCmdSetViewportSwizzleNV(buffer_, first_viewport, viewport_count, viewport_swizzles);
    }
}

} // namespace kx