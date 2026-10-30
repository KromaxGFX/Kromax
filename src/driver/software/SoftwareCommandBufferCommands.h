#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFERCOMMANDS_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFERCOMMANDS_H

/*
 * SoftwareCommandBufferCommands.h
 *
 * Command structs and variant for the software command buffer.
 * Separated to keep SoftwareCommandBuffer.h under 200 lines.
 */

#include <cstdint>
#include <variant>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareDevice;
class SoftwareImage;
class SoftwareBuffer;
class SoftwareRenderPass;
class SoftwareFramebuffer;

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

struct CmdBindVertexBuffers {
    uint32_t first_binding;
    uint32_t binding_count;
    SoftwareBuffer* buffers[16];  // Max 16 vertex buffers
    uint64_t offsets[16];
};

struct CmdBindIndexBuffer {
    SoftwareBuffer* buffer;
    uint64_t offset;
    KxIndexType index_type;
};

struct CmdDraw {
    uint32_t vertex_count;
    uint32_t instance_count;
    uint32_t first_vertex;
    uint32_t first_instance;
};

struct CmdDrawIndexed {
    uint32_t index_count;
    uint32_t instance_count;
    uint32_t first_index;
    int32_t vertex_offset;
    uint32_t first_instance;
};

struct CmdPushConstants {
    KxPipelineLayout layout;
    KxShaderStageFlags stage_flags;
    uint32_t offset;
    uint32_t size;
    std::vector<uint8_t> values;  // Stored push constant data
};

struct CmdDispatch {
    uint32_t group_x;
    uint32_t group_y;
    uint32_t group_z;
};

struct CmdBeginRenderPass {
    SoftwareRenderPass* render_pass;
    SoftwareFramebuffer* framebuffer;
    KxRect2D render_area;
    std::vector<KxClearValue> clear_values;
    KxSubpassContents contents;
};

struct CmdEndRenderPass {
    // No additional data needed
};

struct CmdNextSubpass {
    KxSubpassContents contents;
};

using CommandVariant = std::variant<
    CmdClearImage,
    CmdCopyBuffer,
    CmdCopyBufferToImage,
    CmdBindPipeline,
    CmdBindVertexBuffers,
    CmdBindIndexBuffer,
    CmdPushConstants,
    CmdDraw,
    CmdDrawIndexed,
    CmdDispatch,
    CmdBeginRenderPass,
    CmdEndRenderPass,
    CmdNextSubpass>;

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFERCOMMANDS_H */