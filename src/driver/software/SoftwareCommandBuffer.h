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