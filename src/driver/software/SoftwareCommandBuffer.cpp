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

} /* namespace kx */