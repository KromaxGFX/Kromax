#ifndef KROMAX_SRC_RUNTIME_ICOMMANDBUFFER_H
#define KROMAX_SRC_RUNTIME_ICOMMANDBUFFER_H

/*
 * ICommandBuffer.h
 *
 * Backend-agnostic command buffer recording interface.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class Buffer;
class Image;
class ImageView;
class Pipeline;
class PipelineLayout;
class DescriptorSet;
class RenderPass;
class Framebuffer;

struct ImageMemoryBarrier {
    KxStructureType sType = KX_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    const void* pNext = nullptr;
    KxAccessFlags srcAccessMask = 0;
    KxAccessFlags dstAccessMask = 0;
    KxImageLayout oldLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    KxImageLayout newLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    uint32_t srcQueueFamilyIndex = KX_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = KX_QUEUE_FAMILY_IGNORED;
    Image image = nullptr;
    KxImageSubresourceRange subresourceRange{};
};

struct BufferMemoryBarrier {
    KxStructureType sType = KX_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    const void* pNext = nullptr;
    KxAccessFlags srcAccessMask = 0;
    KxAccessFlags dstAccessMask = 0;
    uint32_t srcQueueFamilyIndex = KX_QUEUE_FAMILY_IGNORED;
    uint32_t dstQueueFamilyIndex = KX_QUEUE_FAMILY_IGNORED;
    Buffer buffer = nullptr;
    KxDeviceSize offset = 0;
    KxDeviceSize size = KX_WHOLE_SIZE;
};

struct MemoryBarrier {
    KxStructureType sType = KX_STRUCTURE_TYPE_MEMORY_BARRIER;
    const void* pNext = nullptr;
    KxAccessFlags srcAccessMask = 0;
    KxAccessFlags dstAccessMask = 0;
};

struct RenderPassBeginInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    const void* pNext = nullptr;
    RenderPass renderPass = nullptr;
    Framebuffer framebuffer = nullptr;
    KxRect2D renderArea{};
    uint32_t clearValueCount = 0;
    const KxClearValue* pClearValues = nullptr;
};

struct DispatchIndirectCommand {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct DrawIndirectCommand {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct DrawIndexedIndirectCommand {
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t vertexOffset;
    uint32_t firstInstance;
};

class ICommandBuffer {
public:
    virtual ~ICommandBuffer() = default;

    // Lifecycle
    virtual Result begin(const KxCommandBufferBeginInfo& info) = 0;
    virtual Result end() = 0;
    virtual Result reset(KxCommandBufferResetFlags flags = 0) = 0;

    // Pipeline binding
    virtual void cmdBindPipeline(KxPipelineBindPoint pipelineBindPoint, Pipeline pipeline) = 0;

    // Descriptor sets
    virtual void cmdBindDescriptorSets(KxPipelineBindPoint pipelineBindPoint,
                                        PipelineLayout layout,
                                        uint32_t firstSet,
                                        uint32_t descriptorSetCount,
                                        const DescriptorSet* pDescriptorSets,
                                        uint32_t dynamicOffsetCount,
                                        const uint32_t* pDynamicOffsets) = 0;

    // Push constants
    virtual void cmdPushConstants(PipelineLayout layout,
                                   KxShaderStageFlags stageFlags,
                                   uint32_t offset,
                                   uint32_t size,
                                   const void* pValues) = 0;

    // Vertex/index buffers
    virtual void cmdBindVertexBuffers(uint32_t firstBinding,
                                       uint32_t bindingCount,
                                       const Buffer* const* pBuffers,
                                       const KxDeviceSize* pOffsets) = 0;
    virtual void cmdBindIndexBuffer(Buffer buffer, KxDeviceSize offset, KxIndexType indexType) = 0;

    // Draw commands
    virtual void cmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void cmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
    virtual void cmdDrawIndirect(Buffer buffer, KxDeviceSize offset, uint32_t drawCount, uint32_t stride) = 0;
    virtual void cmdDrawIndexedIndirect(Buffer buffer, KxDeviceSize offset, uint32_t drawCount, uint32_t stride) = 0;

    // Dispatch commands
    virtual void cmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void cmdDispatchIndirect(Buffer buffer, KxDeviceSize offset) = 0;

    // Render pass
    virtual void cmdBeginRenderPass(const RenderPassBeginInfo& info, KxSubpassContents contents) = 0;
    virtual void cmdNextSubpass(KxSubpassContents contents) = 0;
    virtual void cmdEndRenderPass() = 0;

    // Barriers
    virtual void cmdPipelineBarrier(KxPipelineStageFlags srcStageMask,
                                     KxPipelineStageFlags dstStageMask,
                                     KxDependencyFlags dependencyFlags,
                                     uint32_t memoryBarrierCount,
                                     const MemoryBarrier* pMemoryBarriers,
                                     uint32_t bufferMemoryBarrierCount,
                                     const BufferMemoryBarrier* pBufferMemoryBarriers,
                                     uint32_t imageMemoryBarrierCount,
                                     const ImageMemoryBarrier* pImageMemoryBarriers) = 0;

    // Copy commands
    virtual void cmdCopyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint32_t regionCount, const KxBufferCopy* pRegions) = 0;
    virtual void cmdCopyImage(Image srcImage, KxImageLayout srcLayout, Image dstImage, KxImageLayout dstLayout, uint32_t regionCount, const KxImageCopy* pRegions) = 0;
    virtual void cmdCopyBufferToImage(Buffer srcBuffer, Image dstImage, KxImageLayout dstLayout, uint32_t regionCount, const KxBufferImageCopy* pRegions) = 0;
    virtual void cmdCopyImageToBuffer(Image srcImage, KxImageLayout srcLayout, Buffer dstBuffer, uint32_t regionCount, const KxBufferImageCopy* pRegions) = 0;
    virtual void cmdBlitImage(Image srcImage, KxImageLayout srcLayout, Image dstImage, KxImageLayout dstLayout, uint32_t regionCount, const KxImageBlit* pRegions, KxFilter filter) = 0;

    // Image layout transitions
    virtual void cmdImageMemoryBarrier(uint32_t barrierCount, const ImageMemoryBarrier* pBarriers) = 0;

    // Viewport / scissor
    virtual void cmdSetViewport(uint32_t firstViewport, uint32_t viewportCount, const KxViewport* pViewports) = 0;
    virtual void cmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, const KxRect2D* pScissors) = 0;

    // Dynamic state
    virtual void cmdSetLineWidth(float lineWidth) = 0;
    virtual void cmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) = 0;
    virtual void cmdSetBlendConstants(const float blendConstants[4]) = 0;
    virtual void cmdSetDepthBounds(float minDepthBounds, float maxDepthBounds) = 0;
    virtual void cmdSetStencilCompareMask(KxStencilFaceFlags faceMask, uint32_t compareMask) = 0;
    virtual void cmdSetStencilWriteMask(KxStencilFaceFlags faceMask, uint32_t writeMask) = 0;
    virtual void cmdSetStencilReference(KxStencilFaceFlags faceMask, uint32_t reference) = 0;

    // Debug markers
    virtual void cmdDebugMarkerBegin(const char* name, const float color[4]) = 0;
    virtual void cmdDebugMarkerEnd() = 0;
    virtual void cmdDebugMarkerInsert(const char* name, const float color[4]) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ICOMMANDBUFFER_H