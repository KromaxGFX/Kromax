#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H

/*
 * SoftwareCommandBuffer.h
 *
 * Software command buffer: records commands into a linear buffer and
 * replays them on queue submit.
 */

#include <cstdint>
#include <variant>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareDevice;
class SoftwareImage;
class SoftwareBuffer;

struct CmdClearImage {
    SoftwareImage* image;
    KxImageLayout layout;
    float color[4];   /* RGBA */
    float depth;
    uint8_t stencil;
};

struct CmdCopyBuffer {
    SoftwareBuffer* src;
    SoftwareBuffer* dst;
    uint64_t src_offset;
    uint64_t dst_offset;
    uint64_t size;
};

struct CmdCopyBufferToImage {
    SoftwareBuffer* src;
    SoftwareImage* dst;
    uint64_t src_offset;
    uint32_t dst_width;
    uint32_t dst_height;
};

struct CmdBindPipeline {
    KxPipelineBindPoint bind_point;
    void* pipeline;  /* opaque software pipeline pointer */
};

struct CmdDraw {
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
};

struct CmdDispatch {
    uint32_t group_x;
    uint32_t group_y;
    uint32_t group_z;
};

using CommandVariant = std::variant<
    CmdClearImage,
    CmdCopyBuffer,
    CmdCopyBufferToImage,
    CmdBindPipeline,
    CmdDraw,
    CmdDispatch>;

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
    void record_draw(uint32_t vertex_count, uint32_t instance_count,
                     uint32_t first_vertex, uint32_t first_instance);
    void record_dispatch(uint32_t gx, uint32_t gy, uint32_t gz);

    const std::vector<CommandVariant>& commands() const noexcept {
        return commands_;
    }

    bool is_recording() const noexcept { return recording_; }
    void begin() { recording_ = true; commands_.clear(); }
    void end() { recording_ = false; }

private:
    std::vector<CommandVariant> commands_;
    bool recording_ = false;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFER_H */