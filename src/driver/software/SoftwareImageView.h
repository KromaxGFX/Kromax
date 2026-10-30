#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGEVIEW_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGEVIEW_H

/*
 * SoftwareImageView.h
 *
 * Software driver implementation of KxImageView.
 */

#include "kx_core.h"
#include "SoftwareImage.h"

namespace kx {

class SoftwareImageView {
public:
    SoftwareImageView(const KxImageViewCreateInfo* create_info, SoftwareImage* image);

    KxImageViewType view_type() const { return create_info_.viewType; }
    KxFormat format() const { return create_info_.format; }
    KxImageAspectFlags aspect_mask() const { return create_info_.subresourceRange.aspectMask; }
    uint32_t base_mip_level() const { return create_info_.subresourceRange.baseMipLevel; }
    uint32_t level_count() const { return create_info_.subresourceRange.levelCount; }
    uint32_t base_array_layer() const { return create_info_.subresourceRange.baseArrayLayer; }
    uint32_t layer_count() const { return create_info_.subresourceRange.layerCount; }
    SoftwareImage* image() const { return image_; }

    // Calculate subresource range for the view
    bool is_compatible_with_image(const KxImageCreateInfo& image_info) const;

private:
    KxImageViewCreateInfo create_info_;
    SoftwareImage* image_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGEVIEW_H */