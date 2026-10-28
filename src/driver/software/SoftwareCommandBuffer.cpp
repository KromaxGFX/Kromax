#include "SoftwareCommandBuffer.h"

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

void SoftwareCommandBuffer::record_draw(uint32_t vertex_count,
                                        uint32_t instance_count,
                                        uint32_t first_vertex,
                                        uint32_t first_instance) {
    commands_.push_back(
        CmdDraw{vertex_count, instance_count, first_vertex, first_instance});
}

void SoftwareCommandBuffer::record_dispatch(uint32_t gx, uint32_t gy,
                                            uint32_t gz) {
    commands_.push_back(CmdDispatch{gx, gy, gz});
}

} /* namespace kx */