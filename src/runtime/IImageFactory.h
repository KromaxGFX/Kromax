#ifndef KROMAX_SRC_RUNTIME_IIMAGEFACTORY_H
#define KROMAX_SRC_RUNTIME_IIMAGEFACTORY_H

/*
 * IImageFactory.h
 * Image and ImageView creation, destruction, and memory requirements.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Image;
class ImageView;

struct ImageCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    const void* pNext = nullptr;
    KxImageCreateFlags flags = 0;
    KxImageType imageType = KX_IMAGE_TYPE_2D;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxExtent3D extent = {1, 1, 1};
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    KxSampleCountFlagBits samples = KX_SAMPLE_COUNT_1_BIT;
    KxImageTiling tiling = KX_IMAGE_TILING_OPTIMAL;
    KxImageUsageFlags usage = 0;
    KxSharingMode sharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    KxImageLayout initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
};

struct ImageViewCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    const void* pNext = nullptr;
    KxImageViewCreateFlags flags = 0;
    Image image = nullptr;
    KxImageViewType viewType = KX_IMAGE_VIEW_TYPE_2D;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxComponentMapping components{};
    KxImageSubresourceRange subresourceRange{};
};

class IImageFactory {
public:
    virtual ~IImageFactory() = default;

    virtual KxResult createImage(const ImageCreateInfo& info, Image** outImage) = 0;
    virtual void destroyImage(Image* image) = 0;
    virtual KxResult getImageMemoryRequirements(Image* image, KxMemoryRequirements* pRequirements) = 0;
    virtual KxResult createImageView(const ImageViewCreateInfo& info, ImageView** outView) = 0;
    virtual void destroyImageView(ImageView* view) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IIMAGEFACTORY_H