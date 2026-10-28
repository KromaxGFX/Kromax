#ifndef KX_DISPLAY_H
#define KX_DISPLAY_H

/*
 * kx_display.h
 *
 * KX_EXT display extension: KxDisplayEXT, KxSwapchainEXT and related
 * creation/presentation functions. Declared here; implementations live in
 * src/display/. Functions return KX_ERROR_FEATURE_NOT_PRESENT until the
 * corresponding display backend is wired up.
 */

#include "kx_core.h"

#ifdef __cplusplus
extern "C" {
#endif

KX_DEFINE_HANDLE(KxDisplayEXT)
KX_DEFINE_HANDLE(KxSwapchainEXT)

typedef enum KxDisplayCreateInfoFlagsEXT {
    KX_DISPLAY_CREATE_NONE_EXT = 0u
} KxDisplayCreateInfoFlagsEXT;

typedef struct KxDisplayCreateInfoEXT {
    KxStructureType sType;
    const void* pNext;
    KxDisplayCreateInfoFlagsEXT flags;
    /* native display handle (platform-specific, passed via pNext chain) */
    const void* pNativeDisplay;
} KxDisplayCreateInfoEXT;

typedef enum KxSwapchainCreateInfoFlagsEXT {
    KX_SWAPCHAIN_CREATE_NONE_EXT = 0u
} KxSwapchainCreateInfoFlagsEXT;

typedef struct KxSwapchainCreateInfoEXT {
    KxStructureType sType;
    const void* pNext;
    KxSwapchainCreateInfoFlagsEXT flags;
    KxDisplayEXT display;
    uint32_t width;
    uint32_t height;
    KxFormat imageFormat;
    KxImageUsageFlags imageUsage;
    uint32_t minImageCount;
    KxPresentModeKHR presentMode;
    KxBool32 clipped;
} KxSwapchainCreateInfoEXT;

typedef struct KxPresentInfoEXT {
    KxStructureType sType;
    const void* pNext;
    KxSwapchainEXT swapchain;
    uint32_t imageIndex;
} KxPresentInfoEXT;

/* Display */
KX_API KxResult KX_CALL kxCreateDisplayEXT(
    KxInstance instance,
    const KxDisplayCreateInfoEXT* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDisplayEXT* pDisplay);

KX_API void KX_CALL kxDestroyDisplayEXT(
    KxInstance instance,
    KxDisplayEXT display,
    const KxAllocationCallbacks* pAllocator);

/* Swapchain */
KX_API KxResult KX_CALL kxCreateSwapchainEXT(
    KxDevice device,
    const KxSwapchainCreateInfoEXT* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSwapchainEXT* pSwapchain);

KX_API void KX_CALL kxDestroySwapchainEXT(
    KxDevice device,
    KxSwapchainEXT swapchain,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxGetSwapchainImagesEXT(
    KxDevice device,
    KxSwapchainEXT swapchain,
    uint32_t* pImageCount,
    KxImage* pImages);

KX_API KxResult KX_CALL kxAcquireNextImageEXT(
    KxDevice device,
    KxSwapchainEXT swapchain,
    uint64_t timeout,
    KxSemaphore semaphore,
    KxFence fence,
    uint32_t* pImageIndex);

KX_API KxResult KX_CALL kxQueuePresentEXT(
    KxQueue queue,
    const KxPresentInfoEXT* pPresentInfo);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_DISPLAY_H */