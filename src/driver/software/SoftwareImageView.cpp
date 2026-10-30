#include "SoftwareImageView.h"

#include <algorithm>

namespace kx {

SoftwareImageView::SoftwareImageView(const KxImageViewCreateInfo* create_info,
                                     SoftwareImage* image)
    : create_info_(*create_info), image_(image) {}

bool SoftwareImageView::is_compatible_with_image(
    const KxImageCreateInfo& image_info) const {
    // Validate view type compatibility with image type
    switch (image_info.imageType) {
    case KX_IMAGE_TYPE_1D:
        if (create_info_.viewType != KX_IMAGE_VIEW_TYPE_1D &&
            create_info_.viewType != KX_IMAGE_VIEW_TYPE_1D_ARRAY)
            return false;
        break;
    case KX_IMAGE_TYPE_2D:
        if (create_info_.viewType != KX_IMAGE_VIEW_TYPE_2D &&
            create_info_.viewType != KX_IMAGE_VIEW_TYPE_2D_ARRAY &&
            create_info_.viewType != KX_IMAGE_VIEW_TYPE_CUBE &&
            create_info_.viewType != KX_IMAGE_VIEW_TYPE_CUBE_ARRAY)
            return false;
        break;
    case KX_IMAGE_TYPE_3D:
        if (create_info_.viewType != KX_IMAGE_VIEW_TYPE_3D)
            return false;
        break;
    default:
        return false;
    }

    // Validate mip levels
    if (create_info_.subresourceRange.baseMipLevel + create_info_.subresourceRange.levelCount > image_info.mipLevels)
        return false;

    // Validate array layers
    if (create_info_.subresourceRange.baseArrayLayer + create_info_.subresourceRange.layerCount > image_info.arrayLayers)
        return false;

    // Validate format compatibility (simplified: must match exactly for now)
    if (create_info_.format != image_info.format)
        return false;

    // Validate aspect mask
    if (create_info_.subresourceRange.aspectMask == 0)
        return false;

    // For depth/stencil formats, aspect must include depth/stencil
    bool is_depth_stencil = (image_info.usage & KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0;
    if (is_depth_stencil) {
        if ((create_info_.subresourceRange.aspectMask & (KX_IMAGE_ASPECT_DEPTH_BIT | KX_IMAGE_ASPECT_STENCIL_BIT)) == 0)
            return false;
    } else {
        // Color image - aspect must include color
        if ((create_info_.subresourceRange.aspectMask & KX_IMAGE_ASPECT_COLOR_BIT) == 0)
            return false;
    }

    return true;
}

} /* namespace kx */