#include "SoftwareRenderPass.h"

#include <algorithm>

namespace kx {

SoftwareRenderPass::SoftwareRenderPass(const KxRenderPassCreateInfo* create_info) {
    // Copy attachments
    if (create_info->attachmentCount > 0 && create_info->pAttachments) {
        attachments_.reserve(create_info->attachmentCount);
        for (uint32_t i = 0; i < create_info->attachmentCount; ++i) {
            attachments_.push_back(create_info->pAttachments[i]);
        }
    }

    // Copy subpasses
    if (create_info->subpassCount > 0 && create_info->pSubpasses) {
        subpasses_.reserve(create_info->subpassCount);
        for (uint32_t i = 0; i < create_info->subpassCount; ++i) {
            subpasses_.push_back(create_info->pSubpasses[i]);
        }
    }

    // Copy dependencies
    if (create_info->dependencyCount > 0 && create_info->pDependencies) {
        dependencies_.reserve(create_info->dependencyCount);
        for (uint32_t i = 0; i < create_info->dependencyCount; ++i) {
            dependencies_.push_back(create_info->pDependencies[i]);
        }
    }

    // Validate subpass attachment references
    for (uint32_t i = 0; i < subpasses_.size(); ++i) {
        const auto& subpass = subpasses_[i];
        if (subpass.colorAttachmentCount > 0 && subpass.pColorAttachments) {
            for (uint32_t j = 0; j < subpass.colorAttachmentCount; ++j) {
                if (subpass.pColorAttachments[j].attachment >= attachments_.size()) {
                    // Invalid attachment reference - leave invalid state
                    // The create_render_pass method will check this
                }
            }
        }
        if (subpass.pDepthStencilAttachment) {
            if (subpass.pDepthStencilAttachment->attachment >= attachments_.size()) {
                // Invalid depth/stencil attachment reference
            }
        }
    }
}

int32_t SoftwareRenderPass::depth_stencil_attachment(uint32_t subpass_index) const {
    if (subpass_index >= subpasses_.size()) return -1;
    const auto& subpass = subpasses_[subpass_index];
    if (subpass.pDepthStencilAttachment) {
        return static_cast<int32_t>(subpass.pDepthStencilAttachment->attachment);
    }
    return -1;
}

uint32_t SoftwareRenderPass::color_attachment_count(uint32_t subpass_index) const {
    if (subpass_index >= subpasses_.size()) return 0;
    return subpasses_[subpass_index].colorAttachmentCount;
}

const KxAttachmentReference* SoftwareRenderPass::color_attachment_ref(
    uint32_t subpass_index, uint32_t color_idx) const {
    if (subpass_index >= subpasses_.size()) return nullptr;
    const auto& subpass = subpasses_[subpass_index];
    if (color_idx >= subpass.colorAttachmentCount) return nullptr;
    return &subpass.pColorAttachments[color_idx];
}

} /* namespace kx */