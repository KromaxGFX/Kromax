#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREFRAMEBUFFER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREFRAMEBUFFER_H

/*
 * SoftwareFramebuffer.h
 *
 * Software driver implementation of KxFramebuffer.
 */

#include "kx_core.h"
#include "SoftwareImageView.h"
#include "SoftwareRenderPass.h"
#include "SoftwareImage.h"
#include <vector>

namespace kx {

class SoftwareFramebuffer {
public:
    SoftwareFramebuffer(const KxFramebufferCreateInfo* create_info,
                        const std::vector<SoftwareImageView*>& attachments,
                        SoftwareRenderPass* render_pass);

    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    uint32_t layers() const { return layers_; }
    SoftwareRenderPass* render_pass() const { return render_pass_; }
    uint32_t attachment_count() const { return attachments_.size(); }
    SoftwareImageView* attachment(uint32_t index) const {
        return (index < attachments_.size()) ? attachments_[index] : nullptr;
    }

    // Get color attachment images (for rendering)
    std::vector<SoftwareImage*> color_attachments() const {
        std::vector<SoftwareImage*> result;
        for (auto* view : attachments_) {
            if (view && view->aspect_mask() & KX_IMAGE_ASPECT_COLOR_BIT) {
                result.push_back(view->image());
            }
        }
        return result;
    }

    // Get depth/stencil attachment image
    SoftwareImage* depth_attachment() const {
        for (auto* view : attachments_) {
            if (view && (view->aspect_mask() & (KX_IMAGE_ASPECT_DEPTH_BIT | KX_IMAGE_ASPECT_STENCIL_BIT))) {
                return view->image();
            }
        }
        return nullptr;
    }

private:
    uint32_t width_;
    uint32_t height_;
    uint32_t layers_;
    SoftwareRenderPass* render_pass_;
    std::vector<SoftwareImageView*> attachments_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREFRAMEBUFFER_H */