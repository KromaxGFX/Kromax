/*
 * SoftwareDeviceImage.cpp
 *
 * Image creation, destruction, image views, and binding.
 */

#include "SoftwareDevice.h"

#include "SoftwareImage.h"
#include "SoftwareImageView.h"

namespace kx {

KxResult SoftwareDevice::create_image(
    const KxImageCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxImage* image) {
    if (!image) return KX_ERROR_INVALID_ARGUMENT;
    *image = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->width == 0 || create_info->height == 0)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->imageType == KX_IMAGE_TYPE_3D && create_info->depth == 0)
        return KX_ERROR_INVALID_ARGUMENT;

    // Calculate image size based on format and dimensions
    uint32_t pixel_size = format_pixel_size(create_info->format);

    uint64_t image_size = static_cast<uint64_t>(create_info->width) *
                          static_cast<uint64_t>(create_info->height) *
                          static_cast<uint64_t>(create_info->depth) *
                          static_cast<uint64_t>(create_info->mipLevels) *
                          static_cast<uint64_t>(create_info->arrayLayers) *
                          pixel_size;

    // Allocate memory for the image
    KxMemoryAllocateInfo mem_info{};
    mem_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = image_size;
    mem_info.memoryTypeIndex = 0;

    KxDeviceMemory memory = nullptr;
    KxResult result = allocate_memory(&mem_info, allocator, &memory);
    if (result != KX_SUCCESS) return result;

    // Create and register the image
    auto img = std::make_unique<SoftwareImage>(*create_info, allocator);
    *image = register_image(std::move(img));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_image(KxImage image,
                                   const KxAllocationCallbacks* allocator) {
    (void)allocator;
    auto* img = find_image(image);
    if (img) {
        // Free the underlying memory - we need to track which memory belongs to the image
        // For now, we'll just unregister; memory tracking would need enhancement
        unregister_image(image);
    }
}

KxResult SoftwareDevice::create_image_view(
    const KxImageViewCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxImageView* image_view) {
    (void)allocator;
    if (!image_view) return KX_ERROR_INVALID_ARGUMENT;
    *image_view = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    // Find the parent image
    auto* image = find_image(create_info->image);
    if (!image) return KX_ERROR_INVALID_ARGUMENT;

    // Validate the view against the image
    if (!image->create_info().imageType || !image->create_info().format) {
        return KX_ERROR_INVALID_ARGUMENT;
    }

    // Check view type compatibility
    KxImageType img_type = image->create_info().imageType;
    KxImageViewType view_type = create_info->viewType;
    bool compatible = false;
    switch (img_type) {
    case KX_IMAGE_TYPE_1D:
        compatible = (view_type == KX_IMAGE_VIEW_TYPE_1D || view_type == KX_IMAGE_VIEW_TYPE_1D_ARRAY);
        break;
    case KX_IMAGE_TYPE_2D:
        compatible = (view_type == KX_IMAGE_VIEW_TYPE_2D || view_type == KX_IMAGE_VIEW_TYPE_2D_ARRAY ||
                      view_type == KX_IMAGE_VIEW_TYPE_CUBE || view_type == KX_IMAGE_VIEW_TYPE_CUBE_ARRAY);
        break;
    case KX_IMAGE_TYPE_3D:
        compatible = (view_type == KX_IMAGE_VIEW_TYPE_3D);
        break;
    default:
        compatible = false;
    }
    if (!compatible) return KX_ERROR_INVALID_ARGUMENT;

    // Validate subresource range
    if (create_info->subresourceRange.baseMipLevel + create_info->subresourceRange.levelCount > image->create_info().mipLevels)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->subresourceRange.baseArrayLayer + create_info->subresourceRange.layerCount > image->create_info().arrayLayers)
        return KX_ERROR_INVALID_ARGUMENT;

    // Validate format
    if (create_info->format != image->create_info().format)
        return KX_ERROR_INVALID_ARGUMENT;

    // Validate aspect mask
    if (create_info->subresourceRange.aspectMask == 0)
        return KX_ERROR_INVALID_ARGUMENT;

    // Create and register the image view
    auto view = std::make_unique<SoftwareImageView>(create_info, image);
    *image_view = register_image_view(std::move(view));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_image_view(KxImageView image_view,
                                        const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_image_view(image_view);
}

} /* namespace kx */