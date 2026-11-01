#ifndef KROMAX_SRC_RUNTIME_ISWAPCHAINFACTORY_H
#define KROMAX_SRC_RUNTIME_ISWAPCHAINFACTORY_H

/*
 * ISwapchainFactory.h
 * Swapchain management.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Swapchain;
class Image;
class Semaphore;
class Fence;

struct SwapchainCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO;
    const void* pNext = nullptr;
    KxSwapchainCreateFlags flags = 0;
    void* surface = nullptr;
    uint32_t minImageCount = 2;
    KxFormat imageFormat = KX_FORMAT_B8G8R8A8_UNORM;
    KxColorSpaceKHR imageColorSpace = KX_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    KxExtent2D imageExtent = {0, 0};
    uint32_t imageArrayLayers = 1;
    KxImageUsageFlags imageUsage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    KxSharingMode imageSharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    KxSurfaceTransformFlagBitsKHR preTransform = KX_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    KxCompositeAlphaFlagBitsKHR compositeAlpha = KX_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    KxPresentModeKHR presentMode = KX_PRESENT_MODE_FIFO_KHR;
    KxBool32 clipped = true;
    Swapchain oldSwapchain = nullptr;
};

class ISwapchainFactory {
public:
    virtual ~ISwapchainFactory() = default;

    virtual KxResult createSwapchain(const SwapchainCreateInfo& info, Swapchain** outSwapchain) = 0;
    virtual void destroySwapchain(Swapchain swapchain) = 0;
    virtual KxResult getSwapchainImages(Swapchain swapchain, uint32_t* pImageCount, Image* pImages) = 0;
    virtual KxResult acquireNextImage(Swapchain swapchain, uint64_t timeout, Semaphore semaphore, Fence fence, uint32_t* pImageIndex) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISWAPCHAINFACTORY_H