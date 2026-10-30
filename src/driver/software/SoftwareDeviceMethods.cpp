/*
 * SoftwareDeviceMethods.cpp
 *
 * IDriverDevice virtual method implementations - render pass, framebuffer, command pool.
 */

#include "SoftwareDevice.h"

#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"
#include "SoftwareCommandPool.h"
#include "SoftwareCommandBuffer.h"

namespace kx {

/* Render Pass */

KxResult SoftwareDevice::create_render_pass(
    const KxRenderPassCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxRenderPass* render_pass) {
    (void)allocator;
    if (!render_pass) return KX_ERROR_INVALID_ARGUMENT;
    *render_pass = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    // Validate subpass attachment references
    for (uint32_t s = 0; s < create_info->subpassCount; ++s) {
        const auto& subpass = create_info->pSubpasses[s];
        for (uint32_t c = 0; c < subpass.colorAttachmentCount; ++c) {
            if (subpass.pColorAttachments[c].attachment >= create_info->attachmentCount) {
                return KX_ERROR_INVALID_ARGUMENT;
            }
        }
        if (subpass.pDepthStencilAttachment) {
            if (subpass.pDepthStencilAttachment->attachment >= create_info->attachmentCount) {
                return KX_ERROR_INVALID_ARGUMENT;
            }
        }
        for (uint32_t i = 0; i < subpass.inputAttachmentCount; ++i) {
            if (subpass.pInputAttachments[i].attachment >= create_info->attachmentCount) {
                return KX_ERROR_INVALID_ARGUMENT;
            }
        }
        for (uint32_t r = 0; r < subpass.preserveAttachmentCount; ++r) {
            if (subpass.pPreserveAttachments[r] >= create_info->attachmentCount) {
                return KX_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    auto rp = std::make_unique<SoftwareRenderPass>(create_info);
    *render_pass = register_render_pass(std::move(rp));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_render_pass(KxRenderPass render_pass,
                                         const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_render_pass(render_pass);
}

/* Framebuffer */

KxResult SoftwareDevice::create_framebuffer(
    const KxFramebufferCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxFramebuffer* framebuffer) {
    (void)allocator;
    if (!framebuffer) return KX_ERROR_INVALID_ARGUMENT;
    *framebuffer = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    // Find render pass
    auto* render_pass = find_render_pass(create_info->renderPass);
    if (!render_pass) return KX_ERROR_INVALID_ARGUMENT;

    // Validate attachment count matches render pass
    if (create_info->attachmentCount != render_pass->attachment_count()) {
        return KX_ERROR_INVALID_ARGUMENT;
    }

    // Find attachment views
    std::vector<SoftwareImageView*> attachments;
    attachments.reserve(create_info->attachmentCount);
    for (uint32_t i = 0; i < create_info->attachmentCount; ++i) {
        auto* view = find_image_view(create_info->pAttachments[i]);
        if (!view) return KX_ERROR_INVALID_ARGUMENT;
        attachments.push_back(view);
    }

    // Validate dimensions match first attachment
    if (create_info->attachmentCount > 0) {
        auto* first_image = attachments[0]->image();
        if (first_image) {
            if (create_info->width != first_image->width() ||
                create_info->height != first_image->height()) {
                return KX_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    auto fb = std::make_unique<SoftwareFramebuffer>(create_info, attachments, render_pass);
    *framebuffer = register_framebuffer(std::move(fb));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_framebuffer(KxFramebuffer framebuffer,
                                         const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_framebuffer(framebuffer);
}

/* Command Pool */

KxResult SoftwareDevice::create_command_pool(
    const KxCommandPoolCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxCommandPool* command_pool) {
    (void)allocator;
    if (!command_pool) return KX_ERROR_INVALID_ARGUMENT;
    *command_pool = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    auto pool = std::make_unique<SoftwareCommandPool>(create_info);
    *command_pool = register_command_pool(std::move(pool));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_command_pool(KxCommandPool command_pool,
                                          const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_command_pool(command_pool);
}

/* Command Buffer */

KxResult SoftwareDevice::allocate_command_buffers(
    const KxCommandBufferAllocateInfo* allocate_info,
    KxCommandBuffer* command_buffers) {
    (void)allocate_info;
    if (!command_buffers) return KX_ERROR_INVALID_ARGUMENT;
    
    // For simplicity, allocate one command buffer
    auto cmd = std::make_unique<SoftwareCommandBuffer>();
    command_buffers[0] = register_command_buffer(std::move(cmd));
    return KX_SUCCESS;
}

void SoftwareDevice::free_command_buffers(KxCommandPool command_pool,
                                          uint32_t count,
                                          const KxCommandBuffer* command_buffers) {
    (void)command_pool;
    for (uint32_t i = 0; i < count; ++i) {
        unregister_command_buffer(command_buffers[i]);
    }
}

} /* namespace kx */