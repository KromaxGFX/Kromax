#ifndef KX_SWAPCHAIN_H
#define KX_SWAPCHAIN_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Swapchain Extension (KX_KHR_swapchain)
 * ======================================================================= */

#define KX_KHR_SWAPCHAIN_EXTENSION_NAME "KX_KHR_swapchain"
#define KX_KHR_SWAPCHAIN_SPEC_VERSION 1

/* Forward declaration */
typedef struct KxSwapchain_T* KxSwapchainKHR;
typedef struct KxSurface_T* KxSurfaceKHR;

/* Swapchain create flags */
typedef uint32_t KxSwapchainCreateFlagsKHR;
#define KX_SWAPCHAIN_CREATE_NONE_KHR 0u
#define KX_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR 0x00000001u
#define KX_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR 0x00000002u
#define KX_SWAPCHAIN_CREATE_MAX_ENUM_KHR 0x7FFFFFFFu

/* Surface transform */
typedef uint32_t KxSurfaceTransformFlagsKHR;
#define KX_SURFACE_TRANSFORM_IDENTITY_BIT_KHR 0x00000001u
#define KX_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR 0x00000002u
#define KX_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR 0x00000004u
#define KX_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR 0x00000008u
#define KX_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR 0x00000010u
#define KX_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR 0x00000020u
#define KX_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR 0x00000040u
#define KX_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR 0x00000080u
#define KX_SURFACE_TRANSFORM_INHERIT_BIT_KHR 0x00000100u
#define KX_SURFACE_TRANSFORM_MAX_ENUM_KHR 0x7FFFFFFFu

typedef uint32_t KxSurfaceTransformFlagBitsKHR;

/* Composite alpha */
typedef uint32_t KxCompositeAlphaFlagsKHR;
#define KX_COMPOSITE_ALPHA_OPAQUE_BIT_KHR 0x00000001u
#define KX_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR 0x00000002u
#define KX_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR 0x00000004u
#define KX_COMPOSITE_ALPHA_INHERIT_BIT_KHR 0x00000008u
#define KX_COMPOSITE_ALPHA_MAX_ENUM_KHR 0x7FFFFFFFu

typedef uint32_t KxCompositeAlphaFlagBitsKHR;

/* Present mode - defined in kx_enums.h */
/* typedef enum KxPresentModeKHR { ... } KxPresentModeKHR; */

/* Surface capabilities */
typedef struct KxSurfaceCapabilitiesKHR {
    KxStructureType sType;
    void* pNext;
    uint32_t minImageCount;
    uint32_t maxImageCount;
    KxExtent2D currentExtent;
    KxExtent2D minImageExtent;
    KxExtent2D maxImageExtent;
    uint32_t maxImageArrayLayers;
    KxSurfaceTransformFlagsKHR supportedTransforms;
    KxSurfaceTransformFlagBitsKHR currentTransform;
    KxCompositeAlphaFlagsKHR supportedCompositeAlpha;
    KxImageUsageFlags supportedUsageFlags;
} KxSurfaceCapabilitiesKHR;

/* Surface format */
typedef struct KxSurfaceFormatKHR {
    KxStructureType sType;
    void* pNext;
    KxFormat format;
    KxColorSpaceKHR colorSpace;
} KxSurfaceFormatKHR;

/* Swapchain create info */
typedef struct KxSwapchainCreateInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxSwapchainCreateFlagsKHR flags;
    KxSurfaceKHR surface;
    uint32_t minImageCount;
    KxFormat imageFormat;
    KxColorSpaceKHR imageColorSpace;
    KxExtent2D imageExtent;
    uint32_t imageArrayLayers;
    KxImageUsageFlags imageUsage;
    KxSharingMode imageSharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxSurfaceTransformFlagBitsKHR preTransform;
    KxCompositeAlphaFlagBitsKHR compositeAlpha;
    KxPresentModeKHR presentMode;
    KxBool32 clipped;
    KxSwapchainKHR oldSwapchain;
} KxSwapchainCreateInfoKHR;

/* Present info */
typedef struct KxPresentInfoKHR {
    KxStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    const KxSemaphore* pWaitSemaphores;
    uint32_t swapchainCount;
    const KxSwapchainKHR* pSwapchains;
    const uint32_t* pImageIndices;
    const KxResult* pResults;
} KxPresentInfoKHR;

/* =======================================================================
 * Surface creation (platform-specific)
 * ======================================================================= */

typedef struct KxWin32SurfaceCreateInfoKHR {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    void* hinstance;
    void* hwnd;
} KxWin32SurfaceCreateInfoKHR;

typedef struct KxXcbSurfaceCreateInfoKHR {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    void* connection;
    uint32_t window;
} KxXcbSurfaceCreateInfoKHR;

typedef struct KxWaylandSurfaceCreateInfoKHR {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    void* display;
    void* surface;
} KxWaylandSurfaceCreateInfoKHR;

/* =======================================================================
 * Swapchain functions
 * ======================================================================= */

/* Create a swapchain for a surface */
KX_API KxResult KX_CALL kxCreateSwapchainKHR(
    KxDevice device,
    const KxSwapchainCreateInfoKHR* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSwapchainKHR* pSwapchain);

/* Destroy a swapchain */
KX_API void KX_CALL kxDestroySwapchainKHR(
    KxDevice device,
    KxSwapchainKHR swapchain,
    const KxAllocationCallbacks* pAllocator);

/* Get swapchain images */
KX_API KxResult KX_CALL kxGetSwapchainImagesKHR(
    KxDevice device,
    KxSwapchainKHR swapchain,
    uint32_t* pImageCount,
    KxImage* pImages);

/* Acquire next image */
KX_API KxResult KX_CALL kxAcquireNextImageKHR(
    KxDevice device,
    KxSwapchainKHR swapchain,
    uint64_t timeout,
    KxSemaphore semaphore,
    KxFence fence,
    uint32_t* pImageIndex);

/* Queue present */
KX_API KxResult KX_CALL kxQueuePresentKHR(
    KxQueue queue,
    const KxPresentInfoKHR* pPresentInfo);

/* Get device group present capabilities (for multi-GPU) */
KX_API KxResult KX_CALL kxGetDeviceGroupPresentCapabilitiesKHR(
    KxDevice device,
    void* pDeviceGroupPresentCapabilities);

/* Get device group surface present modes */
KX_API KxResult KX_CALL kxGetDeviceGroupSurfacePresentModesKHR(
    KxDevice device,
    KxSurfaceKHR surface,
    void* pSurfacePresentModes);

/* Get physical device present rects */
KX_API KxResult KX_CALL kxGetPhysicalDevicePresentRectanglesKHR(
    KxPhysicalDevice physicalDevice,
    KxSurfaceKHR surface,
    uint32_t* pRectCount,
    KxRect2D* pRects);

/* Acquire next image 2 (with more options) */
KX_API KxResult KX_CALL kxAcquireNextImage2KHR(
    KxDevice device,
    const void* pAcquireInfo,
    uint32_t* pImageIndex);

/* =======================================================================
 * Surface functions
 * ======================================================================= */

/* Create a Win32 surface */
KX_API KxResult KX_CALL kxCreateWin32SurfaceKHR(
    KxInstance instance,
    const KxWin32SurfaceCreateInfoKHR* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSurfaceKHR* pSurface);

/* Create an XCB surface */
KX_API KxResult KX_CALL kxCreateXcbSurfaceKHR(
    KxInstance instance,
    const KxXcbSurfaceCreateInfoKHR* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSurfaceKHR* pSurface);

/* Create a Wayland surface */
KX_API KxResult KX_CALL kxCreateWaylandSurfaceKHR(
    KxInstance instance,
    const KxWaylandSurfaceCreateInfoKHR* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSurfaceKHR* pSurface);

/* Destroy a surface */
KX_API void KX_CALL kxDestroySurfaceKHR(
    KxInstance instance,
    KxSurfaceKHR surface,
    const KxAllocationCallbacks* pAllocator);

/* Get surface capabilities */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfaceCapabilitiesKHR(
    KxPhysicalDevice physicalDevice,
    KxSurfaceKHR surface,
    KxSurfaceCapabilitiesKHR* pSurfaceCapabilities);

/* Get surface formats */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfaceFormatsKHR(
    KxPhysicalDevice physicalDevice,
    KxSurfaceKHR surface,
    uint32_t* pSurfaceFormatCount,
    KxSurfaceFormatKHR* pSurfaceFormats);

/* Get surface present modes */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfacePresentModesKHR(
    KxPhysicalDevice physicalDevice,
    KxSurfaceKHR surface,
    uint32_t* pPresentModeCount,
    KxPresentModeKHR* pPresentModes);

/* Get surface support */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfaceSupportKHR(
    KxPhysicalDevice physicalDevice,
    uint32_t queueFamilyIndex,
    KxSurfaceKHR surface,
    KxBool32* pSupported);

/* Get surface capabilities 2 */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfaceCapabilities2KHR(
    KxPhysicalDevice physicalDevice,
    const void* pSurfaceInfo,
    KxSurfaceCapabilitiesKHR* pSurfaceCapabilities);

/* Get surface formats 2 */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfaceFormats2KHR(
    KxPhysicalDevice physicalDevice,
    const void* pSurfaceInfo,
    uint32_t* pSurfaceFormatCount,
    KxSurfaceFormatKHR* pSurfaceFormats);

/* Get physical device surface support for presentation */
KX_API KxResult KX_CALL kxGetPhysicalDeviceSurfacePresentModes2EXT(
    KxPhysicalDevice physicalDevice,
    const void* pSurfaceInfo,
    uint32_t* pPresentModeCount,
    KxPresentModeKHR* pPresentModes);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_SWAPCHAIN_H */