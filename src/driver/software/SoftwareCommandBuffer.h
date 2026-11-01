#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H

/*
 * SoftwareCommandBuffer.h
 *
 * Software command buffer: records commands into a linear buffer and
 * replays them on queue submit.
 */

#include "SoftwareCommandBufferCommands.h"

namespace kx {

class SoftwareCommandBuffer {
public:
    SoftwareCommandBuffer() = default;
    ~SoftwareCommandBuffer() = default;

    SoftwareCommandBuffer(const SoftwareCommandBuffer&) = delete;
    SoftwareCommandBuffer& operator=(const SoftwareCommandBuffer&) = delete;

    void record_clear_image(SoftwareImage* image, KxImageLayout layout,
                            float r, float g, float b, float a);
    void record_copy_buffer(SoftwareBuffer* src, SoftwareBuffer* dst,
                            uint64_t src_offset, uint64_t dst_offset,
                            uint64_t size);
    void record_copy_buffer_to_image(SoftwareBuffer* src, SoftwareImage* dst,
                                     uint64_t src_offset,
                                     uint32_t dst_width, uint32_t dst_height);
    void record_bind_pipeline(KxPipelineBindPoint bind_point,
                              void* pipeline);
    void record_bind_vertex_buffers(uint32_t first_binding, uint32_t binding_count,
                                    SoftwareBuffer* const* buffers, const uint64_t* offsets);
    void record_bind_index_buffer(SoftwareBuffer* buffer, uint64_t offset, KxIndexType index_type);
    void record_draw(uint32_t vertex_count, uint32_t instance_count,
                     uint32_t first_vertex, uint32_t first_instance);
    void record_draw_indexed(uint32_t index_count, uint32_t instance_count,
                             uint32_t first_index, int32_t vertex_offset,
                             uint32_t first_instance);
    void record_push_constants(KxPipelineLayout layout, KxShaderStageFlags stage_flags,
                               uint32_t offset, uint32_t size, const void* values);
    void record_bind_descriptor_sets(KxPipelineBindPoint bind_point,
                                     KxPipelineLayout layout,
                                     uint32_t first_set,
                                     uint32_t descriptor_set_count,
                                     const KxDescriptorSet* sets,
                                     uint32_t dynamic_offset_count,
                                     const uint32_t* dynamic_offsets);
    void record_dispatch(uint32_t gx, uint32_t gy, uint32_t gz);

    // Render pass commands
    void record_begin_render_pass(SoftwareRenderPass* render_pass,
                                  SoftwareFramebuffer* framebuffer,
                                  const KxRect2D& render_area,
                                  const std::vector<KxClearValue>& clear_values,
                                  KxSubpassContents contents);
    void record_end_render_pass();
    void record_next_subpass(KxSubpassContents contents);

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    void record_begin_rendering(const KxRenderingInfoKHR* pRenderingInfo);
    void record_end_rendering();

    // Synchronization 2 (VK_KHR_synchronization2)
    void record_pipeline_barrier2(const KxDependencyInfoKHR* pDependencyInfo);
    void record_write_timestamp2(KxPipelineStageFlags2KHR stage, KxQueryPool queryPool, uint32_t query);
    void record_set_event2(KxEvent event, const KxDependencyInfoKHR* pDependencyInfo);
    void record_reset_event2(KxEvent event, KxPipelineStageFlags2KHR stageMask);
    void record_wait_events2(uint32_t eventCount, const KxEvent* pEvents, const KxDependencyInfoKHR* pDependencyInfos);

    // Copy Commands v2 (VK_KHR_copy_commands2)
    void record_copy_buffer2(const void* pCopyBufferInfo);
    void record_copy_image2(const void* pCopyImageInfo);
    void record_blit_image2(const void* pBlitImageInfo);
    void record_copy_buffer_to_image2(const void* pCopyBufferToImageInfo);
    void record_copy_image_to_buffer2(const void* pCopyImageToBufferInfo);
    void record_resolve_image2(const void* pResolveImageInfo);

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    void record_set_depth_clamp_enable(KxBool32 depthClampEnable);
    void record_set_polygon_mode(KxPolygonMode polygonMode);
    void record_set_rasterization_samples(KxSampleCountFlagBits rasterizationSamples);
    void record_set_sample_mask(KxSampleMask sampleMask);
    void record_set_alpha_to_coverage_enable(KxBool32 alphaToCoverageEnable);
    void record_set_alpha_to_one_enable(KxBool32 alphaToOneEnable);
    void record_set_logic_op_enable(KxBool32 logicOpEnable);
    void record_set_color_blend_enable(uint32_t firstAttachment, uint32_t attachmentCount, const KxBool32* pColorBlendEnables);
    void record_set_color_blend_equation(uint32_t firstAttachment, uint32_t attachmentCount, const void* pColorBlendEquations);
    void record_set_color_write_mask(uint32_t firstAttachment, uint32_t attachmentCount, const KxColorComponentFlags* pColorWriteMasks);
    void record_set_tessellation_domain_origin(KxTessellationDomainOriginKHR domainOrigin);
    void record_set_rasterization_stream(uint32_t rasterizationStream);
    void record_set_conservative_rasterization_mode(KxConservativeRasterizationModeEXT conservativeRasterizationMode);
    void record_set_extra_primitive_overestimation_size(float extraPrimitiveOverestimationSize);
    void record_set_depth_clip_enable(KxBool32 depthClipEnable);
    void record_set_depth_clip_negative_one_to_one(KxBool32 negativeOneToOne);
    void record_set_viewport_w_scaling_enable_nv(KxBool32 viewportWScalingEnable);
    void record_set_viewport_swizzle_nv(uint32_t firstViewport, uint32_t viewportCount, const void* pViewportSwizzles);

    // Render pass state
    bool in_render_pass() const noexcept { return in_render_pass_; }
    uint32_t current_subpass() const noexcept { return current_subpass_; }
    SoftwareRenderPass* active_render_pass() const noexcept { return active_render_pass_; }
    SoftwareFramebuffer* active_framebuffer() const noexcept { return active_framebuffer_; }

    const std::vector<CommandVariant>& commands() const noexcept {
        return commands_;
    }

    bool is_recording() const noexcept { return recording_; }
    void begin() { recording_ = true; commands_.clear(); in_render_pass_ = false; current_subpass_ = 0; active_render_pass_ = nullptr; active_framebuffer_ = nullptr; }
    void end() { recording_ = false; }

private:
    std::vector<CommandVariant> commands_;
    bool recording_ = false;

    // Render pass state
    bool in_render_pass_ = false;
    uint32_t current_subpass_ = 0;
    SoftwareRenderPass* active_render_pass_ = nullptr;
    SoftwareFramebuffer* active_framebuffer_ = nullptr;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H */