#include "SoftwareCommandBuffer.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"

#include <cstring>

namespace kx {

void SoftwareCommandBuffer::record_clear_image(SoftwareImage* image,
                                               KxImageLayout layout,
                                               float r, float g, float b,
                                               float a) {
    commands_.push_back(CmdClearImage{image, layout, {r, g, b, a}, 0.0f, 0});
}

void SoftwareCommandBuffer::record_copy_buffer(SoftwareBuffer* src,
                                               SoftwareBuffer* dst,
                                               uint64_t src_offset,
                                               uint64_t dst_offset,
                                               uint64_t size) {
    commands_.push_back(CmdCopyBuffer{src, dst, src_offset, dst_offset, size});
}

void SoftwareCommandBuffer::record_copy_buffer_to_image(
    SoftwareBuffer* src, SoftwareImage* dst, uint64_t src_offset,
    uint32_t dst_width, uint32_t dst_height) {
    commands_.push_back(
        CmdCopyBufferToImage{src, dst, src_offset, dst_width, dst_height});
}

void SoftwareCommandBuffer::record_bind_pipeline(KxPipelineBindPoint bind_point,
                                                 void* pipeline) {
    commands_.push_back(CmdBindPipeline{bind_point, pipeline});
}

void SoftwareCommandBuffer::record_bind_vertex_buffers(uint32_t first_binding,
                                                       uint32_t binding_count,
                                                       SoftwareBuffer* const* buffers,
                                                       const uint64_t* offsets) {
    CmdBindVertexBuffers cmd;
    cmd.first_binding = first_binding;
    cmd.binding_count = binding_count;
    for (uint32_t i = 0; i < binding_count && i < 16; ++i) {
        cmd.buffers[i] = buffers[i];
        cmd.offsets[i] = offsets ? offsets[i] : 0;
    }
    commands_.push_back(cmd);
}

void SoftwareCommandBuffer::record_bind_index_buffer(SoftwareBuffer* buffer,
                                                     uint64_t offset,
                                                     KxIndexType index_type) {
    commands_.push_back(CmdBindIndexBuffer{buffer, offset, index_type});
}

void SoftwareCommandBuffer::record_draw(uint32_t vertex_count,
                                        uint32_t instance_count,
                                        uint32_t first_vertex,
                                        uint32_t first_instance) {
    commands_.push_back(
        CmdDraw{vertex_count, instance_count, first_vertex, first_instance});
}

void SoftwareCommandBuffer::record_draw_indexed(uint32_t index_count,
                                                uint32_t instance_count,
                                                uint32_t first_index,
                                                int32_t vertex_offset,
                                                uint32_t first_instance) {
    commands_.push_back(
        CmdDrawIndexed{index_count, instance_count, first_index, vertex_offset, first_instance});
}

void SoftwareCommandBuffer::record_push_constants(KxPipelineLayout layout,
                                                  KxShaderStageFlags stage_flags,
                                                  uint32_t offset,
                                                  uint32_t size,
                                                  const void* values) {
    CmdPushConstants cmd;
    cmd.layout = layout;
    cmd.stage_flags = stage_flags;
    cmd.offset = offset;
    cmd.size = size;
    if (values && size > 0) {
        cmd.values.resize(size);
        std::memcpy(cmd.values.data(), values, size);
    }
    commands_.push_back(cmd);
}

void SoftwareCommandBuffer::record_dispatch(uint32_t gx, uint32_t gy,
                                            uint32_t gz) {
    commands_.push_back(CmdDispatch{gx, gy, gz});
}

void SoftwareCommandBuffer::record_begin_render_pass(
    SoftwareRenderPass* render_pass,
    SoftwareFramebuffer* framebuffer,
    const KxRect2D& render_area,
    const std::vector<KxClearValue>& clear_values,
    KxSubpassContents contents) {
    commands_.push_back(CmdBeginRenderPass{
        render_pass, framebuffer, render_area, clear_values, contents
    });
    in_render_pass_ = true;
    current_subpass_ = 0;
    active_render_pass_ = render_pass;
    active_framebuffer_ = framebuffer;
}

void SoftwareCommandBuffer::record_end_render_pass() {
    commands_.push_back(CmdEndRenderPass{});
    in_render_pass_ = false;
    current_subpass_ = 0;
    active_render_pass_ = nullptr;
    active_framebuffer_ = nullptr;
}

void SoftwareCommandBuffer::record_next_subpass(KxSubpassContents contents) {
    commands_.push_back(CmdNextSubpass{contents});
    current_subpass_++;
}

void SoftwareCommandBuffer::record_bind_descriptor_sets(KxPipelineBindPoint bind_point,
                                                        KxPipelineLayout layout,
                                                        uint32_t first_set,
                                                        uint32_t descriptor_set_count,
                                                        const KxDescriptorSet* sets,
                                                        uint32_t dynamic_offset_count,
                                                        const uint32_t* dynamic_offsets) {
    CmdBindDescriptorSets cmd;
    cmd.bind_point = bind_point;
    cmd.layout = layout;
    cmd.first_set = first_set;
    cmd.descriptor_set_count = descriptor_set_count;
    for (uint32_t i = 0; i < descriptor_set_count && i < 16; ++i) {
        cmd.sets[i] = sets[i];
    }
    cmd.dynamic_offset_count = dynamic_offset_count;
    for (uint32_t i = 0; i < dynamic_offset_count && i < 16; ++i) {
        cmd.dynamic_offsets[i] = dynamic_offsets[i];
    }
    commands_.push_back(cmd);
}

// Dynamic Rendering (VK_KHR_dynamic_rendering)
void SoftwareCommandBuffer::record_begin_rendering(const KxRenderingInfoKHR* pRenderingInfo) {
    // Store rendering info for validation
    commands_.push_back(CmdBeginRendering{pRenderingInfo});
    in_render_pass_ = true;
}

void SoftwareCommandBuffer::record_end_rendering() {
    commands_.push_back(CmdEndRendering{});
    in_render_pass_ = false;
}

// Synchronization 2 (VK_KHR_synchronization2)
void SoftwareCommandBuffer::record_pipeline_barrier2(const KxDependencyInfoKHR* pDependencyInfo) {
    commands_.push_back(CmdPipelineBarrier2{pDependencyInfo});
}

void SoftwareCommandBuffer::record_write_timestamp2(KxPipelineStageFlags2KHR stage,
                                                    KxQueryPool queryPool,
                                                    uint32_t query) {
    commands_.push_back(CmdWriteTimestamp2{stage, queryPool, query});
}

void SoftwareCommandBuffer::record_set_event2(KxEvent event,
                                              const KxDependencyInfoKHR* pDependencyInfo) {
    commands_.push_back(CmdSetEvent2{event, pDependencyInfo});
}

void SoftwareCommandBuffer::record_reset_event2(KxEvent event,
                                                KxPipelineStageFlags2KHR stageMask) {
    commands_.push_back(CmdResetEvent2{event, stageMask});
}

void SoftwareCommandBuffer::record_wait_events2(uint32_t eventCount,
                                                const KxEvent* pEvents,
                                                const KxDependencyInfoKHR* pDependencyInfos) {
    commands_.push_back(CmdWaitEvents2{eventCount, pEvents, pDependencyInfos});
}

// Copy Commands v2 (VK_KHR_copy_commands2)
void SoftwareCommandBuffer::record_copy_buffer2(const void* pCopyBufferInfo) {
    commands_.push_back(CmdCopyBuffer2{pCopyBufferInfo});
}

void SoftwareCommandBuffer::record_copy_image2(const void* pCopyImageInfo) {
    commands_.push_back(CmdCopyImage2{pCopyImageInfo});
}

void SoftwareCommandBuffer::record_blit_image2(const void* pBlitImageInfo) {
    commands_.push_back(CmdBlitImage2{pBlitImageInfo});
}

void SoftwareCommandBuffer::record_copy_buffer_to_image2(const void* pCopyBufferToImageInfo) {
    commands_.push_back(CmdCopyBufferToImage2{pCopyBufferToImageInfo});
}

void SoftwareCommandBuffer::record_copy_image_to_buffer2(const void* pCopyImageToBufferInfo) {
    commands_.push_back(CmdCopyImageToBuffer2{pCopyImageToBufferInfo});
}

void SoftwareCommandBuffer::record_resolve_image2(const void* pResolveImageInfo) {
    commands_.push_back(CmdResolveImage2{pResolveImageInfo});
}

// Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
void SoftwareCommandBuffer::record_set_depth_clamp_enable(KxBool32 depthClampEnable) {
    commands_.push_back(CmdSetDepthClampEnable{depthClampEnable});
}

void SoftwareCommandBuffer::record_set_polygon_mode(KxPolygonMode polygonMode) {
    commands_.push_back(CmdSetPolygonMode{polygonMode});
}

void SoftwareCommandBuffer::record_set_rasterization_samples(KxSampleCountFlagBits rasterizationSamples) {
    commands_.push_back(CmdSetRasterizationSamples{rasterizationSamples});
}

void SoftwareCommandBuffer::record_set_sample_mask(KxSampleMask sampleMask) {
    commands_.push_back(CmdSetSampleMask{sampleMask});
}

void SoftwareCommandBuffer::record_set_alpha_to_coverage_enable(KxBool32 alphaToCoverageEnable) {
    commands_.push_back(CmdSetAlphaToCoverageEnable{alphaToCoverageEnable});
}

void SoftwareCommandBuffer::record_set_alpha_to_one_enable(KxBool32 alphaToOneEnable) {
    commands_.push_back(CmdSetAlphaToOneEnable{alphaToOneEnable});
}

void SoftwareCommandBuffer::record_set_logic_op_enable(KxBool32 logicOpEnable) {
    commands_.push_back(CmdSetLogicOpEnable{logicOpEnable});
}

void SoftwareCommandBuffer::record_set_color_blend_enable(uint32_t firstAttachment,
                                                          uint32_t attachmentCount,
                                                          const KxBool32* pColorBlendEnables) {
    commands_.push_back(CmdSetColorBlendEnable{firstAttachment, attachmentCount, pColorBlendEnables});
}

void SoftwareCommandBuffer::record_set_color_blend_equation(uint32_t firstAttachment,
                                                            uint32_t attachmentCount,
                                                            const void* pColorBlendEquations) {
    commands_.push_back(CmdSetColorBlendEquation{firstAttachment, attachmentCount, pColorBlendEquations});
}

void SoftwareCommandBuffer::record_set_color_write_mask(uint32_t firstAttachment,
                                                        uint32_t attachmentCount,
                                                        const KxColorComponentFlags* pColorWriteMasks) {
    commands_.push_back(CmdSetColorWriteMask{firstAttachment, attachmentCount, pColorWriteMasks});
}

void SoftwareCommandBuffer::record_set_tessellation_domain_origin(KxTessellationDomainOriginKHR domainOrigin) {
    commands_.push_back(CmdSetTessellationDomainOrigin{domainOrigin});
}

void SoftwareCommandBuffer::record_set_rasterization_stream(uint32_t rasterizationStream) {
    commands_.push_back(CmdSetRasterizationStream{rasterizationStream});
}

void SoftwareCommandBuffer::record_set_conservative_rasterization_mode(KxConservativeRasterizationModeEXT conservativeRasterizationMode) {
    commands_.push_back(CmdSetConservativeRasterizationMode{conservativeRasterizationMode});
}

void SoftwareCommandBuffer::record_set_extra_primitive_overestimation_size(float extraPrimitiveOverestimationSize) {
    commands_.push_back(CmdSetExtraPrimitiveOverestimationSize{extraPrimitiveOverestimationSize});
}

void SoftwareCommandBuffer::record_set_depth_clip_enable(KxBool32 depthClipEnable) {
    commands_.push_back(CmdSetDepthClipEnable{depthClipEnable});
}

void SoftwareCommandBuffer::record_set_depth_clip_negative_one_to_one(KxBool32 negativeOneToOne) {
    commands_.push_back(CmdSetDepthClipNegativeOneToOne{negativeOneToOne});
}

void SoftwareCommandBuffer::record_set_viewport_w_scaling_enable_nv(KxBool32 viewportWScalingEnable) {
    commands_.push_back(CmdSetViewportWScalingEnableNV{viewportWScalingEnable});
}

void SoftwareCommandBuffer::record_set_viewport_swizzle_nv(uint32_t firstViewport,
                                                           uint32_t viewportCount,
                                                           const void* pViewportSwizzles) {
    commands_.push_back(CmdSetViewportSwizzleNV{firstViewport, viewportCount, pViewportSwizzles});
}

} /* namespace kx */