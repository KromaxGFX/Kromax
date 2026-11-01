/*
 * api_renderpass.cpp
 *
 * RenderPass, Framebuffer, ImageView C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Image views
 * ======================================================================= */

KxResult KX_CALL kxCreateImageView(
    KxDevice device,
    const KxImageViewCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImageView* pImageView) {
    if (!pImageView) return KX_ERROR_INVALID_ARGUMENT;
    *pImageView = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_image_view)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_image_view(device, pCreateInfo, pAllocator,
                                              pImageView);
}

void KX_CALL kxDestroyImageView(
    KxDevice device,
    KxImageView imageView,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_image_view) return;
    dev->dispatch()->destroy_image_view(device, imageView, pAllocator);
}

/* =======================================================================
 * RenderPass
 * ======================================================================= */

KxResult KX_CALL kxCreateRenderPass(
    KxDevice device,
    const KxRenderPassCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxRenderPass* pRenderPass) {
    if (!pRenderPass) return KX_ERROR_INVALID_ARGUMENT;
    *pRenderPass = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_render_pass)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_render_pass(device, pCreateInfo, pAllocator,
                                               pRenderPass);
}

void KX_CALL kxDestroyRenderPass(
    KxDevice device,
    KxRenderPass renderPass,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_render_pass) return;
    dev->dispatch()->destroy_render_pass(device, renderPass, pAllocator);
}

/* =======================================================================
 * Framebuffer
 * ======================================================================= */

KxResult KX_CALL kxCreateFramebuffer(
    KxDevice device,
    const KxFramebufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxFramebuffer* pFramebuffer) {
    if (!pFramebuffer) return KX_ERROR_INVALID_ARGUMENT;
    *pFramebuffer = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_framebuffer)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_framebuffer(device, pCreateInfo, pAllocator,
                                               pFramebuffer);
}

void KX_CALL kxDestroyFramebuffer(
    KxDevice device,
    KxFramebuffer framebuffer,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_framebuffer) return;
    dev->dispatch()->destroy_framebuffer(device, framebuffer, pAllocator);
}

/* =======================================================================
 * Render pass commands
 * ======================================================================= */

void KX_CALL kxCmdBeginRenderPass(
    KxCommandBuffer commandBuffer,
    const KxRenderPassBeginInfo* pRenderPassBegin,
    KxSubpassContents contents) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;

    auto* render_pass = dev->find_render_pass(pRenderPassBegin->renderPass);
    auto* framebuffer = dev->find_framebuffer(pRenderPassBegin->framebuffer);
    if (!render_pass || !framebuffer) return;

    std::vector<KxClearValue> clear_values;
    if (pRenderPassBegin->clearValueCount > 0 && pRenderPassBegin->pClearValues) {
        clear_values.assign(pRenderPassBegin->pClearValues,
                           pRenderPassBegin->pClearValues + pRenderPassBegin->clearValueCount);
    }

    cmd->record_begin_render_pass(render_pass, framebuffer,
                                  pRenderPassBegin->renderArea, clear_values, contents);
}

void KX_CALL kxCmdEndRenderPass(
    KxCommandBuffer commandBuffer) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_end_render_pass();
}

void KX_CALL kxCmdNextSubpass(
    KxCommandBuffer commandBuffer,
    KxSubpassContents contents) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_next_subpass(contents);
}

/* =======================================================================
 * Dynamic Rendering (VK_KHR_dynamic_rendering)
 * ======================================================================= */

void KX_CALL kxCmdBeginRendering(
    KxCommandBuffer commandBuffer,
    const KxRenderingInfoKHR* pRenderingInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_begin_rendering(pRenderingInfo);
}

void KX_CALL kxCmdEndRendering(
    KxCommandBuffer commandBuffer) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_end_rendering();
}

/* =======================================================================
 * Synchronization 2 (VK_KHR_synchronization2)
 * ======================================================================= */

void KX_CALL kxCmdPipelineBarrier2(
    KxCommandBuffer commandBuffer,
    const KxDependencyInfoKHR* pDependencyInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_pipeline_barrier2(pDependencyInfo);
}

void KX_CALL kxCmdWriteTimestamp2(
    KxCommandBuffer commandBuffer,
    KxPipelineStageFlags2KHR stage,
    KxQueryPool queryPool,
    uint32_t query) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_write_timestamp2(stage, queryPool, query);
}

void KX_CALL kxCmdSetEvent2(
    KxCommandBuffer commandBuffer,
    KxEvent event,
    const KxDependencyInfoKHR* pDependencyInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_event2(event, pDependencyInfo);
}

void KX_CALL kxCmdResetEvent2(
    KxCommandBuffer commandBuffer,
    KxEvent event,
    KxPipelineStageFlags2KHR stageMask) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_reset_event2(event, stageMask);
}

void KX_CALL kxCmdWaitEvents2(
    KxCommandBuffer commandBuffer,
    uint32_t eventCount,
    const KxEvent* pEvents,
    const KxDependencyInfoKHR* pDependencyInfos) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_wait_events2(eventCount, pEvents, pDependencyInfos);
}

/* =======================================================================
 * Copy Commands v2 (VK_KHR_copy_commands2)
 * ======================================================================= */

void KX_CALL kxCmdCopyBuffer2(
    KxCommandBuffer commandBuffer,
    const void* pCopyBufferInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_copy_buffer2(pCopyBufferInfo);
}

void KX_CALL kxCmdCopyImage2(
    KxCommandBuffer commandBuffer,
    const void* pCopyImageInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_copy_image2(pCopyImageInfo);
}

void KX_CALL kxCmdBlitImage2(
    KxCommandBuffer commandBuffer,
    const void* pBlitImageInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_blit_image2(pBlitImageInfo);
}

void KX_CALL kxCmdCopyBufferToImage2(
    KxCommandBuffer commandBuffer,
    const void* pCopyBufferToImageInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_copy_buffer_to_image2(pCopyBufferToImageInfo);
}

void KX_CALL kxCmdCopyImageToBuffer2(
    KxCommandBuffer commandBuffer,
    const void* pCopyImageToBufferInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_copy_image_to_buffer2(pCopyImageToBufferInfo);
}

void KX_CALL kxCmdResolveImage2(
    KxCommandBuffer commandBuffer,
    const void* pResolveImageInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_resolve_image2(pResolveImageInfo);
}

/* =======================================================================
 * Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
 * ======================================================================= */

void KX_CALL kxCmdSetDepthClampEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 depthClampEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_depth_clamp_enable(depthClampEnable);
}

void KX_CALL kxCmdSetPolygonMode(
    KxCommandBuffer commandBuffer,
    KxPolygonMode polygonMode) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_polygon_mode(polygonMode);
}

void KX_CALL kxCmdSetRasterizationSamples(
    KxCommandBuffer commandBuffer,
    KxSampleCountFlagBits rasterizationSamples) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_rasterization_samples(rasterizationSamples);
}

void KX_CALL kxCmdSetSampleMask(
    KxCommandBuffer commandBuffer,
    KxSampleMask sampleMask) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_sample_mask(sampleMask);
}

void KX_CALL kxCmdSetAlphaToCoverageEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 alphaToCoverageEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_alpha_to_coverage_enable(alphaToCoverageEnable);
}

void KX_CALL kxCmdSetAlphaToOneEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 alphaToOneEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_alpha_to_one_enable(alphaToOneEnable);
}

void KX_CALL kxCmdSetLogicOpEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 logicOpEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_logic_op_enable(logicOpEnable);
}

void KX_CALL kxCmdSetColorBlendEnable(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const KxBool32* pColorBlendEnables) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_color_blend_enable(firstAttachment, attachmentCount, pColorBlendEnables);
}

void KX_CALL kxCmdSetColorBlendEquation(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const void* pColorBlendEquations) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_color_blend_equation(firstAttachment, attachmentCount, pColorBlendEquations);
}

void KX_CALL kxCmdSetColorWriteMask(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const KxColorComponentFlags* pColorWriteMasks) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_color_write_mask(firstAttachment, attachmentCount, pColorWriteMasks);
}

void KX_CALL kxCmdSetTessellationDomainOrigin(
    KxCommandBuffer commandBuffer,
    KxTessellationDomainOriginKHR domainOrigin) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_tessellation_domain_origin(domainOrigin);
}

void KX_CALL kxCmdSetRasterizationStream(
    KxCommandBuffer commandBuffer,
    uint32_t rasterizationStream) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_rasterization_stream(rasterizationStream);
}

void KX_CALL kxCmdSetConservativeRasterizationMode(
    KxCommandBuffer commandBuffer,
    KxConservativeRasterizationModeEXT conservativeRasterizationMode) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_conservative_rasterization_mode(conservativeRasterizationMode);
}

void KX_CALL kxCmdSetExtraPrimitiveOverestimationSize(
    KxCommandBuffer commandBuffer,
    float extraPrimitiveOverestimationSize) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_extra_primitive_overestimation_size(extraPrimitiveOverestimationSize);
}

void KX_CALL kxCmdSetDepthClipEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 depthClipEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_depth_clip_enable(depthClipEnable);
}

void KX_CALL kxCmdSetDepthClipNegativeOneToOne(
    KxCommandBuffer commandBuffer,
    KxBool32 negativeOneToOne) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_depth_clip_negative_one_to_one(negativeOneToOne);
}

void KX_CALL kxCmdSetViewportWScalingEnableNV(
    KxCommandBuffer commandBuffer,
    KxBool32 viewportWScalingEnable) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_viewport_w_scaling_enable_nv(viewportWScalingEnable);
}

void KX_CALL kxCmdSetViewportSwizzleNV(
    KxCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const void* pViewportSwizzles) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return;
    auto* cmd = dev->find_command_buffer(commandBuffer);
    if (!cmd) return;
    cmd->record_set_viewport_swizzle_nv(firstViewport, viewportCount, pViewportSwizzles);
}

#ifdef __cplusplus
} /* extern "C" */
#endif