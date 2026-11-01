/*
 * VkRenderPass.cpp
 *
 * Vulkan render pass and framebuffer implementation.
 */

#include "VkRenderPass.h"

#include <vector>

namespace kx {

// ============================================================================
// VkRenderPassImpl
// ============================================================================

VkRenderPassImpl::~VkRenderPassImpl() {
    if (render_pass_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkRenderPassImpl::VkRenderPassImpl(VkRenderPassImpl&& other) noexcept
    : render_pass_(other.render_pass_)
    , device_(other.device_)
    , attachments_(std::move(other.attachments_))
    , subpasses_(std::move(other.subpasses_))
    , dependencies_(std::move(other.dependencies_)) {
    other.render_pass_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkRenderPassImpl& VkRenderPassImpl::operator=(VkRenderPassImpl&& other) noexcept {
    if (this != &other) {
        render_pass_ = other.render_pass_;
        device_ = other.device_;
        attachments_ = std::move(other.attachments_);
        subpasses_ = std::move(other.subpasses_);
        dependencies_ = std::move(other.dependencies_);
        other.render_pass_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkRenderPassImpl::initialize(VkFunctionTable& functions,
                                  VkDevice device,
                                  const KxRenderPassCreateInfo* create_info) {
    device_ = device;

    // Convert attachments
    attachments_.reserve(create_info->attachmentCount);
    for (uint32_t i = 0; i < create_info->attachmentCount; ++i) {
        const KxAttachmentDescription& kx_att = create_info->pAttachments[i];
        VkAttachmentDescription vk_att{};
        vk_att.flags = static_cast<VkAttachmentDescriptionFlags>(kx_att.flags);
        vk_att.format = static_cast<VkFormat>(kx_att.format);
        vk_att.samples = static_cast<VkSampleCountFlagBits>(kx_att.samples);
        vk_att.loadOp = static_cast<VkAttachmentLoadOp>(kx_att.loadOp);
        vk_att.storeOp = static_cast<VkAttachmentStoreOp>(kx_att.storeOp);
        vk_att.stencilLoadOp = static_cast<VkAttachmentLoadOp>(kx_att.stencilLoadOp);
        vk_att.stencilStoreOp = static_cast<VkAttachmentStoreOp>(kx_att.stencilStoreOp);
        vk_att.initialLayout = static_cast<VkImageLayout>(kx_att.initialLayout);
        vk_att.finalLayout = static_cast<VkImageLayout>(kx_att.finalLayout);
        attachments_.push_back(vk_att);
    }

    // Convert subpasses
    subpasses_.reserve(create_info->subpassCount);
    for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
        const KxSubpassDescription& kx_sub = create_info->pSubpasses[i];
        VkSubpassDescription vk_sub{};
        vk_sub.flags = static_cast<VkSubpassDescriptionFlags>(kx_sub.flags);
        vk_sub.pipelineBindPoint = static_cast<VkPipelineBindPoint>(kx_sub.pipelineBindPoint);
        vk_sub.inputAttachmentCount = kx_sub.inputAttachmentCount;
        // Note: pInputAttachments, pColorAttachments, pResolveAttachments, pDepthStencilAttachment, pPreserveAttachments
        // would need to be converted - simplified for now
        subpasses_.push_back(vk_sub);
    }

    // Convert dependencies
    dependencies_.reserve(create_info->dependencyCount);
    for (uint32_t i = 0; i < create_info->dependencyCount; ++i) {
        const KxSubpassDependency& kx_dep = create_info->pDependencies[i];
        VkSubpassDependency vk_dep{};
        vk_dep.srcSubpass = kx_dep.srcSubpass;
        vk_dep.dstSubpass = kx_dep.dstSubpass;
        vk_dep.srcStageMask = static_cast<VkPipelineStageFlags>(kx_dep.srcStageMask);
        vk_dep.dstStageMask = static_cast<VkPipelineStageFlags>(kx_dep.dstStageMask);
        vk_dep.srcAccessMask = static_cast<VkAccessFlags>(kx_dep.srcAccessMask);
        vk_dep.dstAccessMask = static_cast<VkAccessFlags>(kx_dep.dstAccessMask);
        vk_dep.dependencyFlags = static_cast<VkDependencyFlags>(kx_dep.dependencyFlags);
        dependencies_.push_back(vk_dep);
    }

    VkRenderPassCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vk_info.flags = static_cast<VkRenderPassCreateFlags>(create_info->flags);
    vk_info.attachmentCount = static_cast<uint32_t>(attachments_.size());
    vk_info.pAttachments = attachments_.data();
    vk_info.subpassCount = static_cast<uint32_t>(subpasses_.size());
    vk_info.pSubpasses = subpasses_.data();
    vk_info.dependencyCount = static_cast<uint32_t>(dependencies_.size());
    vk_info.pDependencies = dependencies_.data();

    VkResult result = functions.vkCreateRenderPass(device, &vk_info, nullptr, &render_pass_);
    return result == VK_SUCCESS;
}

VkAttachmentReference VkRenderPassImpl::get_attachment_reference(uint32_t subpass, uint32_t attachment) const {
    VkAttachmentReference ref{};
    ref.attachment = attachment;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    return ref;
}

// ============================================================================
// VkFramebufferImpl
// ============================================================================

VkFramebufferImpl::~VkFramebufferImpl() {
    if (framebuffer_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkFramebufferImpl::VkFramebufferImpl(VkFramebufferImpl&& other) noexcept
    : framebuffer_(other.framebuffer_)
    , device_(other.device_)
    , render_pass_(other.render_pass_)
    , width_(other.width_)
    , height_(other.height_)
    , layers_(other.layers_)
    , attachments_(std::move(other.attachments_)) {
    other.framebuffer_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.render_pass_ = VK_NULL_HANDLE;
    other.width_ = 0;
    other.height_ = 0;
    other.layers_ = 1;
}

VkFramebufferImpl& VkFramebufferImpl::operator=(VkFramebufferImpl&& other) noexcept {
    if (this != &other) {
        framebuffer_ = other.framebuffer_;
        device_ = other.device_;
        render_pass_ = other.render_pass_;
        width_ = other.width_;
        height_ = other.height_;
        layers_ = other.layers_;
        attachments_ = std::move(other.attachments_);
        other.framebuffer_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.render_pass_ = VK_NULL_HANDLE;
        other.width_ = 0;
        other.height_ = 0;
        other.layers_ = 1;
    }
    return *this;
}

bool VkFramebufferImpl::initialize(VkFunctionTable& functions,
                                   VkDevice device,
                                   const KxFramebufferCreateInfo* create_info) {
    device_ = device;
    render_pass_ = static_cast<VkRenderPass>(create_info->renderPass);
    width_ = create_info->width;
    height_ = create_info->height;
    layers_ = create_info->layers;

    // Convert attachments
    attachments_.reserve(create_info->attachmentCount);
    for (uint32_t i = 0; i < create_info->attachmentCount; ++i) {
        attachments_.push_back(static_cast<VkImageView>(create_info->pAttachments[i]));
    }

    VkFramebufferCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vk_info.flags = static_cast<VkFramebufferCreateFlags>(create_info->flags);
    vk_info.renderPass = render_pass_;
    vk_info.attachmentCount = static_cast<uint32_t>(attachments_.size());
    vk_info.pAttachments = attachments_.data();
    vk_info.width = width_;
    vk_info.height = height_;
    vk_info.layers = layers_;

    VkResult result = functions.vkCreateFramebuffer(device, &vk_info, nullptr, &framebuffer_);
    return result == VK_SUCCESS;
}

} // namespace kx