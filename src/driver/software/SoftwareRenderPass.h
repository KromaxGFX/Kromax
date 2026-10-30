#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERENDERPASS_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERENDERPASS_H

/*
 * SoftwareRenderPass.h
 *
 * Software driver implementation of KxRenderPass.
 */

#include "kx_core.h"
#include <vector>

namespace kx {

class SoftwareRenderPass {
public:
    SoftwareRenderPass(const KxRenderPassCreateInfo* create_info);

    uint32_t attachment_count() const { return attachments_.size(); }
    uint32_t subpass_count() const { return subpasses_.size(); }
    uint32_t dependency_count() const { return dependencies_.size(); }

    const KxAttachmentDescription& attachment(uint32_t index) const {
        return attachments_[index];
    }

    const KxSubpassDescription& subpass(uint32_t index) const {
        return subpasses_[index];
    }

    const KxSubpassDependency& dependency(uint32_t index) const {
        return dependencies_[index];
    }

    // Find the depth/stencil attachment index for a subpass
    int32_t depth_stencil_attachment(uint32_t subpass_index) const;

    // Get color attachment count for a subpass
    uint32_t color_attachment_count(uint32_t subpass_index) const;

    // Get color attachment reference for a subpass
    const KxAttachmentReference* color_attachment_ref(uint32_t subpass_index, uint32_t color_idx) const;

private:
    std::vector<KxAttachmentDescription> attachments_;
    std::vector<KxSubpassDescription> subpasses_;
    std::vector<KxSubpassDependency> dependencies_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERENDERPASS_H */