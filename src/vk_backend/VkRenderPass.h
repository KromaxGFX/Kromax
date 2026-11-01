#ifndef KROMAX_SRC_VK_BACKEND_VKRENDERPASS_H
#define KROMAX_SRC_VK_BACKEND_VKRENDERPASS_H

/*
 * VkRenderPass.h
 *
 * Vulkan render pass and framebuffer wrappers.
 */

#include "VkLoader.h"
#include <kx_core.h>

#include <vector>

namespace kx {

// Render pass
class VkRenderPassImpl final {
public:
    VkRenderPassImpl() = default;
    ~VkRenderPassImpl();

    VkRenderPassImpl(const VkRenderPassImpl&) = delete;
    VkRenderPassImpl& operator=(const VkRenderPassImpl&) = delete;

    VkRenderPassImpl(VkRenderPassImpl&& other) noexcept;
    VkRenderPassImpl& operator=(VkRenderPassImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxRenderPassCreateInfo* create_info);

    VkRenderPass handle() const noexcept { return render_pass_; }
    uint32_t attachment_count() const noexcept { return static_cast<uint32_t>(attachments_.size()); }
    uint32_t subpass_count() const noexcept { return static_cast<uint32_t>(subpasses_.size()); }

    // Get attachment reference for a given subpass and attachment index
    VkAttachmentReference get_attachment_reference(uint32_t subpass, uint32_t attachment) const;

private:
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<VkAttachmentDescription> attachments_;
    std::vector<VkSubpassDescription> subpasses_;
    std::vector<VkSubpassDependency> dependencies_;
};

// Framebuffer
class VkFramebufferImpl final {
public:
    VkFramebufferImpl() = default;
    ~VkFramebufferImpl();

    VkFramebufferImpl(const VkFramebufferImpl&) = delete;
    VkFramebufferImpl& operator=(const VkFramebufferImpl&) = delete;

    VkFramebufferImpl(VkFramebufferImpl&& other) noexcept;
    VkFramebufferImpl& operator=(VkFramebufferImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxFramebufferCreateInfo* create_info);

    VkFramebuffer handle() const noexcept { return framebuffer_; }
    VkRenderPass render_pass() const noexcept { return render_pass_; }
    uint32_t width() const noexcept { return width_; }
    uint32_t height() const noexcept { return height_; }
    uint32_t layers() const noexcept { return layers_; }

private:
    VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    uint32_t layers_ = 1;
    std::vector<VkImageView> attachments_;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKRENDERPASS_H