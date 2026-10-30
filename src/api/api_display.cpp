#include <algorithm>
/* api_display.cpp
 * Display extension (KxDisplayEXT / KxSwapchainEXT) C API implementation.
 * Backed by the existing offscreen PPM backend and KMS DRM backend.
 */

#include "api.h"
#include "api_internal.h"
#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareImage.h"
#include "SoftwareFramebuffer.h"
#include "SoftwareRenderPass.h"
#include "IDisplayBackend.h"
#include "OffscreenBackend.h"
#include "KmsBackend.h"

#include <cstdio>
#include <vector>

namespace kx {

struct SoftwareDisplay {
    IDisplayBackend* backend = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    // Swapchain image stored separately
    KxImage swapchain_image;
};

// Swapchain state - tracks swapchain creation info
struct SwapchainState {
    uint32_t width = 0;
    uint32_t height = 0;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxPresentModeKHR presentMode = KX_PRESENT_MODE_MAX_ENUM_KHR;
    bool clipped = false;
    uint32_t imageCount = 0;
    KxImage image; // Associated image
};

} // namespace kx

static std::vector<std::unique_ptr<kx::SoftwareDisplay>> g_displays;
static std::vector<kx::SwapchainState> g_swapchain_states;

const char* kx_format_to_string(KxFormat format) {
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "FMT_%d", (int)format);
    return buffer;
}

extern "C" {

KxResult KX_CALL kxCreateDisplayEXT(KxInstance instance,
                                   const KxDisplayCreateInfoEXT* pCreateInfo,
                                   const KxAllocationCallbacks* pAllocator,
                                   KxDisplayEXT* pDisplay)
{
    if (!instance || !pCreateInfo || !pDisplay) return KX_ERROR_INVALID_ARGUMENT;

    auto* disp = new (std::nothrow) kx::SoftwareDisplay();
    if (!disp) return KX_ERROR_OUT_OF_HOST_MEMORY;

    KxResult result = KX_ERROR_FEATURE_NOT_PRESENT;

    // Try offscreen PPM backend first (always available)
    auto* ob = new (std::nothrow) kx::OffscreenBackend();
    if (ob) {
        result = ob->initialize(pCreateInfo);
        if (result == KX_SUCCESS) {
            disp->backend = ob;
            disp->width = 800;
            disp->height = 600;
        } else {
            delete ob;
        }
    }

    // Try KMS backend if offscreen failed
    if (!disp->backend) {
        auto* kb = new (std::nothrow) kx::KmsBackend();
        if (kb) {
            result = kb->initialize(pCreateInfo);
            if (result == KX_SUCCESS) {
                disp->backend = kb;
                disp->width = 800;
                disp->height = 600;
            } else {
                delete kb;
            }
        }
    }

    if (!disp->backend) {
        delete disp;
        return KX_ERROR_FEATURE_NOT_PRESENT;
    }

    g_displays.push_back(std::unique_ptr<kx::SoftwareDisplay>(disp));

    *pDisplay = reinterpret_cast<KxDisplayEXT>(disp);

    std::printf("[kromax][INFO] DisplayEXT created: %s (%ux%u)\n",
        disp->backend->name(), disp->width, disp->height);
    return KX_SUCCESS;
}

void KX_CALL kxDestroyDisplayEXT(KxInstance instance, KxDisplayEXT display,
                                  const KxAllocationCallbacks* pAllocator)
{
    (void)instance; (void)pAllocator;
    if (!display) return;

    auto it = std::find_if(g_displays.begin(), g_displays.end(),
        [display](const std::unique_ptr<kx::SoftwareDisplay>& d) {
            return reinterpret_cast<KxDisplayEXT>(d.get()) == display;
        });

    if (it != g_displays.end()) {
        g_displays.erase(it);
    }
}

KxResult KX_CALL kxCreateSwapchainEXT(KxDevice device,
                                       const KxSwapchainCreateInfoEXT* pCreateInfo,
                                       const KxAllocationCallbacks* pAllocator,
                                       KxSwapchainEXT* pSwapchain)
{
    (void)device; (void)pAllocator;
    if (!pCreateInfo || !pSwapchain) return KX_ERROR_INVALID_ARGUMENT;

    // Find or create a display
    kx::SoftwareDisplay* disp = nullptr;
    if (!g_displays.empty()) {
        disp = g_displays[0].get();
    }

    if (!disp) return KX_ERROR_FEATURE_NOT_PRESENT;

    // Create the swapchain image
    KxImageCreateInfo image_create_info{};
    image_create_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.format = pCreateInfo->imageFormat;
    image_create_info.imageType = KX_IMAGE_TYPE_2D;
    image_create_info.tiling = KX_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.width = pCreateInfo->width;
    image_create_info.height = pCreateInfo->height;
    image_create_info.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = (KxSampleCountFlags)1;
    image_create_info.usage = pCreateInfo->imageUsage;
    image_create_info.sharingModeExclusive = KX_TRUE;
    image_create_info.pQueueFamilyIndices = nullptr;

    KxResult result = kxCreateImage(device, &image_create_info, pAllocator, &disp->swapchain_image);
    if (result != KX_SUCCESS) return result;

    // Store swapchain state
    kx::SwapchainState state{};
    state.width = pCreateInfo->width;
    state.height = pCreateInfo->height;
    state.format = pCreateInfo->imageFormat;
    state.presentMode = pCreateInfo->presentMode;
    state.clipped = pCreateInfo->clipped;
    state.imageCount = 1;
    state.image = disp->swapchain_image;

    g_swapchain_states.push_back(state);

    // Return a "handle" - since KxSwapchainEXT is a handle type,
    // we just return the index into our storage
    *pSwapchain = reinterpret_cast<KxSwapchainEXT>(&g_swapchain_states.back());

    std::printf("[kromax][INFO] SwapchainEXT created: %ux%u, format=%s, presentMode=%u\n",
        pCreateInfo->width, pCreateInfo->height,
        kx_format_to_string(pCreateInfo->imageFormat),
        (unsigned)pCreateInfo->presentMode);

    return KX_SUCCESS;
}

void KX_CALL kxDestroySwapchainEXT(KxDevice device, KxSwapchainEXT swapchain,
                                    const KxAllocationCallbacks* pAllocator)
{
    (void)device; (void)pAllocator;
    if (!swapchain) return;

    // Find and remove the swapchain state
    uintptr_t handle = reinterpret_cast<uintptr_t>(swapchain);
    auto it = std::find_if(g_swapchain_states.begin(), g_swapchain_states.end(),
        [handle](const kx::SwapchainState& s) {
            return reinterpret_cast<uintptr_t>(&s) == handle;
        });

    if (it != g_swapchain_states.end()) {
        g_swapchain_states.erase(it);
    }
}

KxResult KX_CALL kxGetSwapchainImagesEXT(KxDevice device,
                                          KxSwapchainEXT swapchain,
                                          uint32_t* pImageCount,
                                          KxImage* pImages)
{
    if (!device || !pImageCount || !pImages || !swapchain) return KX_ERROR_INVALID_ARGUMENT;

    // Find the swapchain state
    uintptr_t handle = reinterpret_cast<uintptr_t>(swapchain);
    auto it = std::find_if(g_swapchain_states.begin(), g_swapchain_states.end(),
        [handle](const kx::SwapchainState& s) {
            return reinterpret_cast<uintptr_t>(&s) == handle;
        });

    if (it != g_swapchain_states.end()) {
        *pImageCount = it->imageCount;
        if (pImages && *pImageCount > 0) {
            pImages[0] = it->image;
        }
    } else {
        *pImageCount = 0;
    }

    return KX_SUCCESS;
}

KxResult KX_CALL kxAcquireNextImageEXT(KxDevice device,
                                       KxSwapchainEXT swapchain,
                                       uint64_t timeout,
                                       KxSemaphore semaphore,
                                       KxFence fence,
                                       uint32_t* pImageIndex)
{
    (void)device; (void)timeout; (void)semaphore; (void)fence;

    if (!swapchain || !pImageIndex) return KX_ERROR_INVALID_ARGUMENT;

    *pImageIndex = 0;

    return KX_SUCCESS;
}

KxResult KX_CALL kxQueuePresentEXT(KxQueue queue,
                                   const KxPresentInfoEXT* pPresentInfo)
{
    if (!queue || !pPresentInfo) return KX_ERROR_INVALID_ARGUMENT;

    if (g_displays.empty()) {
        return KX_ERROR_FEATURE_NOT_PRESENT;
    }

    kx::SoftwareDisplay* disp = g_displays[0].get();
    if (!disp || !disp->backend) return KX_ERROR_FEATURE_NOT_PRESENT;

    std::printf("[kromax][INFO] QueuePresentEXT: presenting frame\n");

    // Present using the offscreen PPM backend
    kx::BackendFramebuffer fb{};
    // Use the first swapchain state's image
    if (!g_swapchain_states.empty()) {
        fb.image = g_swapchain_states[0].image;
    } else {
        fb.image = nullptr;
    }
    fb.width = disp->width ? disp->width : 640;
    fb.height = disp->height ? disp->height : 480;
    fb.pitch = fb.width * 4; // RGBA8
    fb.data = nullptr;
    fb.data_size = 0;

    return disp->backend->present(&fb);
}

} // extern "C"

// End of display extension API implementation
