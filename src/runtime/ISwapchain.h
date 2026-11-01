#ifndef KROMAX_SRC_RUNTIME_ISWAPCHAIN_H
#define KROMAX_SRC_RUNTIME_ISWAPCHAIN_H

/*
 * ISwapchain.h
 *
 * Backend-agnostic swapchain interface.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class Image;
class Semaphore;
class Fence;

struct SurfaceCapabilities {
    uint32_t minImageCount = 1;
    uint32_t maxImageCount = 0;  // 0 = no limit
    KxExtent2D currentExtent = {0, 0};
    KxExtent2D minImageExtent = {1, 1};
    KxExtent2D maxImageExtent = {0xFFFFFFFF, 0xFFFFFFFF};
    uint32_t maxImageArrayLayers = 1;
    KxSurfaceTransformFlagsKHR supportedTransforms = 0;
    KxSurfaceTransformFlagBitsKHR currentTransform = KX_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    KxCompositeAlphaFlagsKHR supportedCompositeAlpha = 0;
    KxImageUsageFlags supportedUsageFlags = 0;
};

struct SurfaceFormat {
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxColorSpaceKHR colorSpace = KX_COLOR_SPACE_SRGB_NONLINEAR_KHR;
};

struct PresentMode {
    KxPresentModeKHR mode = KX_PRESENT_MODE_FIFO_KHR;
};

class ISwapchain {
public:
    virtual ~ISwapchain() = default;

    virtual Result getImages(uint32_t* pImageCount, Image* pImages) = 0;
    virtual Result acquireNextImage(uint64_t timeout, Semaphore semaphore, Fence fence, uint32_t* pImageIndex) = 0;
    virtual KxExtent2D getExtent() const noexcept = 0;
    virtual KxFormat getFormat() const noexcept = 0;
    virtual uint32_t getImageCount() const noexcept = 0;
};

class ISurface {
public:
    virtual ~ISurface() = default;

    virtual Result getCapabilities(SurfaceCapabilities* pCapabilities) = 0;
    virtual Result getFormats(uint32_t* pFormatCount, SurfaceFormat* pFormats) = 0;
    virtual Result getPresentModes(uint32_t* pPresentModeCount, PresentMode* pPresentModes) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISWAPCHAIN_H