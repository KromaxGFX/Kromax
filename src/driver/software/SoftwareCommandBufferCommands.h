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

struct CmdBindDescriptorSets {
    KxPipelineBindPoint bind_point;
    KxPipelineLayout layout;
    uint32_t first_set;
    uint32_t descriptor_set_count;
    KxDescriptorSet sets[16];  // Max 16 descriptor sets
    uint32_t dynamic_offset_count;
    uint32_t dynamic_offsets[16];
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

// Dynamic Rendering (VK_KHR_dynamic_rendering)
struct CmdBeginRendering {
    const KxRenderingInfoKHR* pRenderingInfo;
};

struct CmdEndRendering {
    // No additional data needed
};

// Synchronization 2 (VK_KHR_synchronization2)
struct CmdPipelineBarrier2 {
    const KxDependencyInfoKHR* pDependencyInfo;
};

struct CmdWriteTimestamp2 {
    KxPipelineStageFlags2KHR stage;
    KxQueryPool queryPool;
    uint32_t query;
};

struct CmdSetEvent2 {
    KxEvent event;
    const KxDependencyInfoKHR* pDependencyInfo;
};

struct CmdResetEvent2 {
    KxEvent event;
    KxPipelineStageFlags2KHR stageMask;
};

struct CmdWaitEvents2 {
    uint32_t eventCount;
    const KxEvent* pEvents;
    const KxDependencyInfoKHR* pDependencyInfos;
};

// Copy Commands v2 (VK_KHR_copy_commands2)
struct CmdCopyBuffer2 {
    const void* pCopyBufferInfo;
};

struct CmdCopyImage2 {
    const void* pCopyImageInfo;
};

struct CmdBlitImage2 {
    const void* pBlitImageInfo;
};

struct CmdCopyBufferToImage2 {
    const void* pCopyBufferToImageInfo;
};

struct CmdCopyImageToBuffer2 {
    const void* pCopyImageToBufferInfo;
};

struct CmdResolveImage2 {
    const void* pResolveImageInfo;
};

// Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
struct CmdSetDepthClampEnable {
    KxBool32 depthClampEnable;
};

struct CmdSetPolygonMode {
    KxPolygonMode polygonMode;
};

struct CmdSetRasterizationSamples {
    KxSampleCountFlagBits rasterizationSamples;
};

struct CmdSetSampleMask {
    KxSampleMask sampleMask;
};

struct CmdSetAlphaToCoverageEnable {
    KxBool32 alphaToCoverageEnable;
};

struct CmdSetAlphaToOneEnable {
    KxBool32 alphaToOneEnable;
};

struct CmdSetLogicOpEnable {
    KxBool32 logicOpEnable;
};

struct CmdSetColorBlendEnable {
    uint32_t firstAttachment;
    uint32_t attachmentCount;
    const KxBool32* pColorBlendEnables;
};

struct CmdSetColorBlendEquation {
    uint32_t firstAttachment;
    uint32_t attachmentCount;
    const void* pColorBlendEquations;
};

struct CmdSetColorWriteMask {
    uint32_t firstAttachment;
    uint32_t attachmentCount;
    const KxColorComponentFlags* pColorWriteMasks;
};

struct CmdSetTessellationDomainOrigin {
    KxTessellationDomainOriginKHR domainOrigin;
};

struct CmdSetRasterizationStream {
    uint32_t rasterizationStream;
};

struct CmdSetConservativeRasterizationMode {
    KxConservativeRasterizationModeEXT conservativeRasterizationMode;
};

struct CmdSetExtraPrimitiveOverestimationSize {
    float extraPrimitiveOverestimationSize;
};

struct CmdSetDepthClipEnable {
    KxBool32 depthClipEnable;
};

struct CmdSetDepthClipNegativeOneToOne {
    KxBool32 negativeOneToOne;
};

struct CmdSetViewportWScalingEnableNV {
    KxBool32 viewportWScalingEnable;
};

struct CmdSetViewportSwizzleNV {
    uint32_t firstViewport;
    uint32_t viewportCount;
    const void* pViewportSwizzles;
};

using CommandVariant = std::variant<
    CmdClearImage,
    CmdCopyBuffer,
    CmdCopyBufferToImage,
    CmdBindPipeline,
    CmdBindVertexBuffers,
    CmdBindIndexBuffer,
    CmdPushConstants,
    CmdBindDescriptorSets,
    CmdDraw,
    CmdDrawIndexed,
    CmdDispatch,
    CmdBeginRenderPass,
    CmdEndRenderPass,
    CmdNextSubpass,
    // Dynamic Rendering
    CmdBeginRendering,
    CmdEndRendering,
    // Synchronization 2
    CmdPipelineBarrier2,
    CmdWriteTimestamp2,
    CmdSetEvent2,
    CmdResetEvent2,
    CmdWaitEvents2,
    // Copy Commands v2
    CmdCopyBuffer2,
    CmdCopyImage2,
    CmdBlitImage2,
    CmdCopyBufferToImage2,
    CmdCopyImageToBuffer2,
    CmdResolveImage2,
    // Extended Dynamic State 3
    CmdSetDepthClampEnable,
    CmdSetPolygonMode,
    CmdSetRasterizationSamples,
    CmdSetSampleMask,
    CmdSetAlphaToCoverageEnable,
    CmdSetAlphaToOneEnable,
    CmdSetLogicOpEnable,
    CmdSetColorBlendEnable,
    CmdSetColorBlendEquation,
    CmdSetColorWriteMask,
    CmdSetTessellationDomainOrigin,
    CmdSetRasterizationStream,
    CmdSetConservativeRasterizationMode,
    CmdSetExtraPrimitiveOverestimationSize,
    CmdSetDepthClipEnable,
    CmdSetDepthClipNegativeOneToOne,
    CmdSetViewportWScalingEnableNV,
    CmdSetViewportSwizzleNV>;

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDBUFFERCOMMANDS_H */