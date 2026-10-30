/*
 * api_pipeline_commands.cpp
 *
 * Commands recordable in a command buffer C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareBuffer.h"
#include "SoftwareImage.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Commands recordable in a command buffer
 * ======================================================================= */

void KX_CALL kxCmdBindPipeline(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipeline pipeline) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    auto* pipe = dev->driver_device()->find_pipeline(pipeline);
    if (pipe) {
        cmd->record_bind_pipeline(pipelineBindPoint, pipe);
    }
}

void KX_CALL kxCmdBindVertexBuffers(
    KxCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const KxBuffer* pBuffers,
    const uint64_t* pOffsets) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    
    std::vector<kx::SoftwareBuffer*> buffers;
    buffers.reserve(bindingCount);
    for (uint32_t i = 0; i < bindingCount; ++i) {
        buffers.push_back(dev->driver_device()->find_buffer(pBuffers[i]));
    }
    
    cmd->record_bind_vertex_buffers(firstBinding, bindingCount, buffers.data(), pOffsets);
}

void KX_CALL kxCmdPushConstants(
    KxCommandBuffer commandBuffer,
    KxPipelineLayout layout,
    KxShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    const void* pValues) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_push_constants(layout, stageFlags, offset, size, pValues);
}

void KX_CALL kxCmdDraw(
    KxCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void KX_CALL kxCmdDrawIndexed(
    KxCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_draw_indexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void KX_CALL kxCmdDispatch(
    KxCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) {
    (void)commandBuffer; (void)groupCountX; (void)groupCountY;
    (void)groupCountZ;
}

/* =======================================================================
 * Commands - Copy/Clear/Barrier
 * ======================================================================= */

void KX_CALL kxCmdCopyBuffer(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxBuffer dstBuffer,
    uint32_t regionCount,
    const void* pRegions) {
    (void)regionCount; (void)pRegions;
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    auto* src = dev->driver_device()->find_buffer(srcBuffer);
    auto* dst = dev->driver_device()->find_buffer(dstBuffer);
    if (src && dst) {
        // Simplified: assume single region covering entire buffer
        cmd->record_copy_buffer(src, dst, 0, 0, dst->size());
    }
}

void KX_CALL kxCmdCopyBufferToImage(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxImage dstImage,
    KxImageLayout dstImageLayout,
    uint32_t regionCount,
    const void* pRegions) {
    (void)regionCount; (void)pRegions; (void)dstImageLayout;
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    auto* src = dev->driver_device()->find_buffer(srcBuffer);
    auto* dst = dev->driver_device()->find_image(dstImage);
    if (src && dst) {
        // Simplified: assume single region covering entire image
        cmd->record_copy_buffer_to_image(src, dst, 0, dst->width(), dst->height());
    }
}

void KX_CALL kxCmdClearImage(
    KxCommandBuffer commandBuffer,
    KxImage image,
    KxImageLayout imageLayout,
    const void* pColor,
    const void* pDepthStencil,
    uint32_t rangeCount,
    const void* pRanges) {
    (void)rangeCount; (void)pRanges; (void)imageLayout; (void)pDepthStencil;
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    auto* img = dev->driver_device()->find_image(image);
    if (img && pColor) {
        const float* color = static_cast<const float*>(pColor);
        cmd->record_clear_image(img, imageLayout, color[0], color[1], color[2], color[3]);
    }
}

void KX_CALL kxCmdPipelineBarrier(
    KxCommandBuffer commandBuffer,
    KxPipelineStageFlags srcStageMask,
    KxPipelineStageFlags dstStageMask,
    KxDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const void* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const void* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const void* pImageMemoryBarriers) {
    (void)commandBuffer; (void)srcStageMask; (void)dstStageMask;
    (void)dependencyFlags; (void)memoryBarrierCount; (void)pMemoryBarriers;
    (void)bufferMemoryBarrierCount; (void)pBufferMemoryBarriers;
    (void)imageMemoryBarrierCount; (void)pImageMemoryBarriers;
}

#ifdef __cplusplus
} /* extern "C" */
#endif