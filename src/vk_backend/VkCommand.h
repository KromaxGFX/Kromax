#ifndef KROMAX_SRC_VK_BACKEND_VKCOMMAND_H
#define KROMAX_SRC_VK_BACKEND_VKCOMMAND_H

/*
 * VkCommand.h
 *
 * Vulkan command pool and command buffer wrappers.
 */

#include "VkLoader.h"
#include "VkPipeline.h"
#include "VkDescriptor.h"
#include <kx_core.h>

#include <vector>
#include <string>
#include <array>
#include <cstring>

namespace kx {

// Command pool
class VkCommandPoolImpl final {
public:
    VkCommandPoolImpl() = default;
    ~VkCommandPoolImpl();

    VkCommandPoolImpl(const VkCommandPoolImpl&) = delete;
    VkCommandPoolImpl& operator=(const VkCommandPoolImpl&) = delete;

    VkCommandPoolImpl(VkCommandPoolImpl&& other) noexcept;
    VkCommandPoolImpl& operator=(VkCommandPoolImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxCommandPoolCreateInfo* create_info);

    VkCommandPool handle() const noexcept { return pool_; }
    uint32_t queue_family_index() const noexcept { return queue_family_index_; }

private:
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    uint32_t queue_family_index_ = 0;
};

// Command buffer
class VkCommandBufferImpl final {
public:
    VkCommandBufferImpl() = default;
    ~VkCommandBufferImpl();

    VkCommandBufferImpl(const VkCommandBufferImpl&) = delete;
    VkCommandBufferImpl& operator=(const VkCommandBufferImpl&) = delete;

    VkCommandBufferImpl(VkCommandBufferImpl&& other) noexcept;
    VkCommandBufferImpl& operator=(VkCommandBufferImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    VkCommandPool pool,
                    KxCommandBufferLevel level);

    VkCommandBuffer handle() const noexcept { return buffer_; }
    VkCommandPool pool() const noexcept { return pool_; }
    KxCommandBufferLevel level() const noexcept { return level_; }
    VkDevice device() const noexcept { return device_; }
    VkFunctionTable* functions() const noexcept { return functions_; }

    // Recording state
    bool is_recording() const noexcept { return recording_; }
    void set_recording(bool recording) noexcept { recording_ = recording; }
    bool is_inside_render_pass() const noexcept { return inside_render_pass_; }
    void set_inside_render_pass(bool inside) noexcept { inside_render_pass_ = inside; }

    // Bound pipeline state
    VkPipelineLayout current_pipeline_layout() const noexcept { return current_pipeline_layout_; }
    void set_current_pipeline_layout(VkPipelineLayout layout) noexcept { current_pipeline_layout_ = layout; }
    VkPipelineBindPoint current_pipeline_bind_point() const noexcept { return current_pipeline_bind_point_; }
    void set_current_pipeline_bind_point(VkPipelineBindPoint point) noexcept { current_pipeline_bind_point_ = point; }

    // Descriptor sets
    const std::vector<VkDescriptorSet>& current_descriptor_sets() const noexcept { return current_descriptor_sets_; }
    void set_current_descriptor_sets(const std::vector<VkDescriptorSet>& sets, uint32_t first_set) noexcept {
        current_descriptor_sets_ = sets;
        first_descriptor_set_ = first_set;
    }
    uint32_t first_descriptor_set() const noexcept { return first_descriptor_set_; }

    // Bound vertex buffers
    struct VertexBufferBinding {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
    };
    const std::vector<VertexBufferBinding>& vertex_buffers() const noexcept { return vertex_buffers_; }
    void bind_vertex_buffers(uint32_t first_binding, const std::vector<VertexBufferBinding>& buffers) noexcept {
        if (first_binding + buffers.size() > vertex_buffers_.size()) {
            vertex_buffers_.resize(first_binding + buffers.size());
        }
        for (size_t i = 0; i < buffers.size(); ++i) {
            vertex_buffers_[first_binding + i] = buffers[i];
        }
    }

    // Bound index buffer
    VkBuffer index_buffer() const noexcept { return index_buffer_; }
    VkDeviceSize index_buffer_offset() const noexcept { return index_buffer_offset_; }
    VkIndexType index_type() const noexcept { return index_type_; }
    void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type) noexcept {
        index_buffer_ = buffer;
        index_buffer_offset_ = offset;
        index_type_ = type;
    }

    // Push constants
    const std::vector<uint8_t>& push_constants() const noexcept { return push_constants_; }
    void set_push_constants(VkPipelineLayout layout, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) noexcept {
        if (offset + size > push_constants_.size()) {
            push_constants_.resize(offset + size);
        }
        std::memcpy(push_constants_.data() + offset, data, size);
    }

    // Dynamic state
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
    float line_width_ = 1.0f;
    float depth_bias_constant_factor_ = 0.0f;
    float depth_bias_clamp_ = 0.0f;
    float depth_bias_slope_factor_ = 0.0f;
    std::array<float, 4> blend_constants_ = {0, 0, 0, 0};
    uint32_t stencil_compare_mask_ = 0xFFFFFFFF;
    uint32_t stencil_write_mask_ = 0xFFFFFFFF;
    uint32_t stencil_reference_ = 0;

    // ============ RECORDING METHODS (llaman vkCmd*) ============
    
    // Pipeline binding
    void cmd_bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline);
    
    // Vertex/Index buffers
    void cmd_bind_vertex_buffers(uint32_t first_binding, uint32_t binding_count, const VkBuffer* buffers, const VkDeviceSize* offsets);
    void cmd_bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type);
    
    // Descriptor sets
    void cmd_bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, uint32_t first_set, uint32_t set_count, const VkDescriptorSet* sets, uint32_t dynamic_offset_count, const uint32_t* dynamic_offsets);
    
    // Push constants
    void cmd_push_constants(VkPipelineLayout layout, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data);
    
    // Draw commands
    void cmd_draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
    void cmd_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
    void cmd_draw_indirect(VkBuffer buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride);
    void cmd_draw_indexed_indirect(VkBuffer buffer, VkDeviceSize offset, uint32_t draw_count, uint32_t stride);
    
    // Dispatch commands
    void cmd_dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);
    void cmd_dispatch_indirect(VkBuffer buffer, VkDeviceSize offset);
    
    // Render pass
    void cmd_begin_render_pass(const VkRenderPassBeginInfo* render_pass_begin, VkSubpassContents contents);
    void cmd_begin_render_pass(VkRenderPass render_pass, VkFramebuffer framebuffer,
                               const VkRect2D& render_area, const VkClearValue* clear_values,
                               uint32_t clear_value_count, VkSubpassContents contents);
    void cmd_begin_render_pass(VkRenderPass render_pass, VkFramebuffer framebuffer,
                               const KxRect2D& render_area, const KxClearValue* clear_values,
                               uint32_t clear_value_count, KxSubpassContents contents);
    void cmd_end_render_pass();
    void cmd_next_subpass(VkSubpassContents contents);
    
    // Command buffer lifecycle
    bool begin(const KxCommandBufferBeginInfo* begin_info);
    bool end();
    bool reset();
    
    // Copy/Blit/Clear
    void cmd_copy_buffer(VkBuffer src, VkBuffer dst, uint32_t region_count, const VkBufferCopy* regions);
    void cmd_copy_image(VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkImageCopy* regions);
    void cmd_copy_buffer_to_image(VkBuffer src, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkBufferImageCopy* regions);
    void cmd_copy_image_to_buffer(VkImage src, VkImageLayout src_layout, VkBuffer dst, uint32_t region_count, const VkBufferImageCopy* regions);
    void cmd_blit_image(VkImage src, VkImageLayout src_layout, VkImage dst, VkImageLayout dst_layout, uint32_t region_count, const VkImageBlit* regions, VkFilter filter);
    void cmd_clear_color_image(VkImage image, VkImageLayout layout, const VkClearColorValue* color, uint32_t range_count, const VkImageSubresourceRange* ranges);
    void cmd_clear_depth_stencil_image(VkImage image, VkImageLayout layout, const VkClearDepthStencilValue* depth_stencil, uint32_t range_count, const VkImageSubresourceRange* ranges);
    void cmd_fill_buffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, uint32_t data);
    void cmd_update_buffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, const void* data);
    
    // Barriers
    void cmd_pipeline_barrier(VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage, VkDependencyFlags flags, uint32_t memory_barrier_count, const VkMemoryBarrier* memory_barriers, uint32_t buffer_memory_barrier_count, const VkBufferMemoryBarrier* buffer_memory_barriers, uint32_t image_memory_barrier_count, const VkImageMemoryBarrier* image_memory_barriers);
    
    // Dynamic state setters
    void cmd_set_viewport(uint32_t first_viewport, uint32_t viewport_count, const VkViewport* viewports);
    void cmd_set_scissor(uint32_t first_scissor, uint32_t scissor_count, const VkRect2D* scissors);
    void cmd_set_line_width(float line_width);
    void cmd_set_depth_bias(float constant_factor, float clamp, float slope_factor);
    void cmd_set_blend_constants(const float blend_constants[4]);
    void cmd_set_depth_bounds(float min_depth_bounds, float max_depth_bounds);
    void cmd_set_stencil_compare_mask(VkStencilFaceFlags face_mask, uint32_t compare_mask);
    void cmd_set_stencil_write_mask(VkStencilFaceFlags face_mask, uint32_t write_mask);
    void cmd_set_stencil_reference(VkStencilFaceFlags face_mask, uint32_t reference);
    
    // Extended dynamic state (VK_EXT_extended_dynamic_state)
    void cmd_set_cull_mode(VkCullModeFlags cull_mode);
    void cmd_set_front_face(VkFrontFace front_face);
    void cmd_set_primitive_topology(VkPrimitiveTopology primitive_topology);
    void cmd_set_viewport_with_count(uint32_t viewport_count, const VkViewport* viewports);
    void cmd_set_scissor_with_count(uint32_t scissor_count, const VkRect2D* scissors);
    void cmd_set_depth_test_enable(VkBool32 depth_test_enable);
    void cmd_set_depth_write_enable(VkBool32 depth_write_enable);
    void cmd_set_depth_compare_op(VkCompareOp depth_compare_op);
    void cmd_set_depth_bounds_test_enable(VkBool32 depth_bounds_test_enable);
    void cmd_set_stencil_test_enable(VkBool32 stencil_test_enable);
    void cmd_set_stencil_op(VkStencilFaceFlags face_mask, VkStencilOp fail_op, VkStencilOp pass_op, VkStencilOp depth_fail_op, VkCompareOp compare_op);
    void cmd_set_primitive_restart_enable(VkBool32 primitive_restart_enable);
    void cmd_set_rasterizer_discard_enable(VkBool32 rasterizer_discard_enable);
    void cmd_set_depth_bias_enable(VkBool32 depth_bias_enable);
    void cmd_set_logic_op(VkLogicOp logic_op);

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    void cmd_begin_rendering(const VkRenderingInfoKHR* rendering_info);
    void cmd_end_rendering();

    // Synchronization 2 (VK_KHR_synchronization2)
    void cmd_pipeline_barrier2(const VkDependencyInfoKHR* dependency_info);
    void cmd_write_timestamp2(VkPipelineStageFlags2KHR stage, VkQueryPool query_pool, uint32_t query);
    void cmd_set_event2(VkEvent event, const VkDependencyInfoKHR* dependency_info);
    void cmd_reset_event2(VkEvent event, VkPipelineStageFlags2KHR stage_mask);
    void cmd_wait_events2(uint32_t event_count, const VkEvent* events, const VkDependencyInfoKHR* dependency_infos);

    // Copy Commands v2 (VK_KHR_copy_commands2)
    void cmd_copy_buffer2(const VkCopyBufferInfo2KHR* copy_buffer_info);
    void cmd_copy_image2(const VkCopyImageInfo2KHR* copy_image_info);
    void cmd_blit_image2(const VkBlitImageInfo2KHR* blit_image_info);
    void cmd_copy_buffer_to_image2(const VkCopyBufferToImageInfo2KHR* copy_buffer_to_image_info);
    void cmd_copy_image_to_buffer2(const VkCopyImageToBufferInfo2KHR* copy_image_to_buffer_info);
    void cmd_resolve_image2(const VkResolveImageInfo2KHR* resolve_image_info);

    // Query
    void cmd_begin_query(VkQueryPool query_pool, uint32_t query, VkQueryControlFlags flags);
    void cmd_end_query(VkQueryPool query_pool, uint32_t query);
    void cmd_reset_query_pool(VkQueryPool query_pool, uint32_t first_query, uint32_t query_count);
    void cmd_write_timestamp(VkPipelineStageFlagBits pipeline_stage, VkQueryPool query_pool, uint32_t query);

    // Conditional rendering
    void cmd_begin_conditional_rendering(VkConditionalRenderingBeginInfoEXT* info);
    void cmd_end_conditional_rendering();

    // Execute commands
    void cmd_execute_commands(uint32_t command_buffer_count, const VkCommandBuffer* command_buffers);

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    void cmd_set_depth_clamp_enable(VkBool32 depth_clamp_enable);
    void cmd_set_polygon_mode(VkPolygonMode polygon_mode);
    void cmd_set_rasterization_samples(VkSampleCountFlagBits rasterization_samples);
    void cmd_set_sample_mask(VkSampleMask sample_mask);
    void cmd_set_alpha_to_coverage_enable(VkBool32 alpha_to_coverage_enable);
    void cmd_set_alpha_to_one_enable(VkBool32 alpha_to_one_enable);
    void cmd_set_logic_op_enable(VkBool32 logic_op_enable);
    void cmd_set_color_blend_enable(uint32_t first_attachment, uint32_t attachment_count, const VkBool32* color_blend_enables);
    void cmd_set_color_blend_equation(uint32_t first_attachment, uint32_t attachment_count, const VkColorBlendEquationEXT* color_blend_equations);
    void cmd_set_color_write_mask(uint32_t first_attachment, uint32_t attachment_count, const VkColorComponentFlags* color_write_masks);
    void cmd_set_tessellation_domain_origin(VkTessellationDomainOriginKHR domain_origin);
    void cmd_set_rasterization_stream(uint32_t rasterization_stream);
    void cmd_set_conservative_rasterization_mode(VkConservativeRasterizationModeEXT conservative_rasterization_mode);
    void cmd_set_extra_primitive_overestimation_size(float extra_primitive_overestimation_size);
    void cmd_set_depth_clip_enable(VkBool32 depth_clip_enable);
    void cmd_set_depth_clip_negative_one_to_one(VkBool32 negative_one_to_one);
    void cmd_set_viewport_w_scaling_enable_nv(VkBool32 viewport_w_scaling_enable);
    void cmd_set_viewport_swizzle_nv(uint32_t first_viewport, uint32_t viewport_count, const VkViewportSwizzleNV* viewport_swizzles);

private:
    VkCommandBuffer buffer_ = VK_NULL_HANDLE;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkFunctionTable* functions_ = nullptr;
    KxCommandBufferLevel level_ = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    bool recording_ = false;
    bool inside_render_pass_ = false;

    // State tracking
    VkPipelineLayout current_pipeline_layout_ = VK_NULL_HANDLE;
    VkPipelineBindPoint current_pipeline_bind_point_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkDescriptorSet> current_descriptor_sets_;
    uint32_t first_descriptor_set_ = 0;
    std::vector<VertexBufferBinding> vertex_buffers_;
    VkBuffer index_buffer_ = VK_NULL_HANDLE;
    VkDeviceSize index_buffer_offset_ = 0;
    VkIndexType index_type_ = VK_INDEX_TYPE_UINT16;
    std::vector<uint8_t> push_constants_;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKCOMMAND_H