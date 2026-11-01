/*
 * TextureHelpers.cpp
 * Texture creation, mipmap generation, and sampler utilities implementation.
 */

#include "TextureHelpers.h"

#include "api_internal.h"
#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareImage.h"

#include <cstring>
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

KxResult KX_CALL kxGetImageInfo(
    KxDevice device,
    KxImage image,
    KxTextureInfo* pInfo) {
    if (!pInfo) return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    pInfo->sType = KX_STRUCTURE_TYPE_MAX_ENUM; // Not defined yet
    pInfo->width = img->width();
    pInfo->height = img->height();
    pInfo->depth = img->depth();
    pInfo->mipLevels = img->mip_levels();
    pInfo->arrayLayers = img->array_layers();
    pInfo->format = img->format();
    pInfo->layout = img->layout();
    pInfo->rowPitch = static_cast<uint32_t>(img->row_pitch());
    pInfo->slicePitch = static_cast<uint32_t>(img->slice_pitch());
    pInfo->dataSize = img->data_size();

    return KX_SUCCESS;
}

KxResult KX_CALL kxCreateTexture2D(
    KxDevice device,
    uint32_t width,
    uint32_t height,
    KxFormat format,
    const void* pPixels,
    uint32_t mipLevels,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage) {
    if (!pImage) return KX_ERROR_INVALID_ARGUMENT;
    *pImage = nullptr;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_image)
        return KX_ERROR_FEATURE_NOT_PRESENT;

    KxImageCreateInfo create_info{};
    create_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = KX_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.width = width;
    create_info.height = height;
    create_info.depth = 1;
    create_info.mipLevels = mipLevels > 0 ? mipLevels : 1;
    create_info.arrayLayers = 1;
    create_info.tiling = KX_IMAGE_TILING_LINEAR;
    create_info.initialLayout = KX_IMAGE_LAYOUT_PREINITIALIZED;
    create_info.usage = KX_IMAGE_USAGE_SAMPLED_BIT | KX_IMAGE_USAGE_TRANSFER_DST_BIT;

    KxResult result = dev->dispatch()->create_image(device, &create_info, pAllocator, pImage);
    if (result != KX_SUCCESS) return result;

    if (pPixels && dev->dispatch()->bind_image_memory && dev->dispatch()->allocate_memory) {
        // Allocate and bind memory, then copy pixels
        // This is simplified - a full implementation would use staging buffers
    }

    return KX_SUCCESS;
}

KxResult KX_CALL kxGenerateMipmaps(
    KxDevice device,
    KxImage image) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    if (img->mip_levels() <= 1) return KX_SUCCESS;

    // Generate mipmaps using box filter
    uint32_t w = img->width();
    uint32_t h = img->height();
    uint32_t pixel_size = 4; // Assume RGBA8 for now

    for (uint32_t level = 1; level < img->mip_levels(); ++level) {
        uint32_t src_w = std::max<uint32_t>(1, w >> (level - 1));
        uint32_t src_h = std::max<uint32_t>(1, h >> (level - 1));
        uint32_t dst_w = std::max<uint32_t>(1, w >> level);
        uint32_t dst_h = std::max<uint32_t>(1, h >> level);

        // Simple box filter downsample
        uint8_t* dst = img->data() + (img->slice_pitch() * level);
        const uint8_t* src = img->data() + (img->slice_pitch() * (level - 1));

        for (uint32_t y = 0; y < dst_h; ++y) {
            for (uint32_t x = 0; x < dst_w; ++x) {
                uint32_t sx0 = x * 2;
                uint32_t sy0 = y * 2;
                uint32_t sx1 = std::min(sx0 + 1, src_w - 1);
                uint32_t sy1 = std::min(sy0 + 1, src_h - 1);

                float r = 0, g = 0, b = 0, a = 0;
                int count = 0;
                for (uint32_t sy = sy0; sy <= sy1; ++sy) {
                    for (uint32_t sx = sx0; sx <= sx1; ++sx) {
                        const uint8_t* p = src + sy * src_w * pixel_size + sx * pixel_size;
                        r += p[0] / 255.0f;
                        g += p[1] / 255.0f;
                        b += p[2] / 255.0f;
                        a += p[3] / 255.0f;
                        ++count;
                    }
                }
                uint8_t* dp = dst + y * dst_w * pixel_size + x * pixel_size;
                dp[0] = static_cast<uint8_t>(std::clamp(r / count, 0.0f, 1.0f) * 255.0f);
                dp[1] = static_cast<uint8_t>(std::clamp(g / count, 0.0f, 1.0f) * 255.0f);
                dp[2] = static_cast<uint8_t>(std::clamp(b / count, 0.0f, 1.0f) * 255.0f);
                dp[3] = static_cast<uint8_t>(std::clamp(a / count, 0.0f, 1.0f) * 255.0f);
            }
        }
    }

    return KX_SUCCESS;
}

KxSamplerState KX_CALL kxDefaultSamplerState(void) {
    KxSamplerState state{};
    state.magFilter = KX_FILTER_NEAREST;
    state.minFilter = KX_FILTER_NEAREST;
    state.addressModeU = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.addressModeV = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.addressModeW = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.mipLodBias = 0.0f;
    state.anisotropyEnable = KX_FALSE;
    state.maxAnisotropy = 1.0f;
    state.compareEnable = KX_FALSE;
    state.compareOp = KX_COMPARE_OP_ALWAYS;
    state.minLod = 0.0f;
    state.maxLod = 1000.0f;
    state.borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    state.unnormalizedCoordinates = KX_FALSE;
    state.maxLodLevel = 0;
    state.srgbDecode = KX_FALSE;
    return state;
}

#ifdef __cplusplus
} /* extern "C" */
#endif