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

#ifdef __cplusplus
} /* extern "C" */
#endif