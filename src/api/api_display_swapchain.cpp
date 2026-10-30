/* api_display_swapchain.cpp - Swapchain extension (KxSwapchainEXT)
 * Modularized: under 200 lines, single responsibility.
 */

#include "api.h"
#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareImage.h"
#include "SoftwareFramebuffer.h"
#include "SoftwareRenderPass.h"
#include "IDisplayBackend.h"
#include "OffscreenBackend.h"

#include <cstdio>
#include <algorithm>
#include <vector>

namespace kx {

struct SwapchainState {
    uint32_t width = 0, height = 0;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxPresentModeKHR presentMode = KX_PRESENT_MODE_MAX_ENUM_KHR;
    bool clipped = false;
    uint32_t imageCount = 0;
    KxImage image;
};

struct SoftwareDisplay {
    IDisplayBackend* backend = nullptr;
    uint32_t width = 0, height = 0;
    KxImage swapchain_image;
};

} // namespace kx

static std::vector<std::unique_ptr<kx::SoftwareDisplay>> g_displays;
static std::vector<kx::SwapchainState> g_swapchain_states;

const char* kx_format_to_string(KxFormat f) {
    static char b[32]; snprintf(b, sizeof(b), "FMT_%d", (int)f); return b;
}

extern "C" {

KxResult KX_CALL kxCreateSwapchainEXT(KxDevice dev,
                                     const KxSwapchainCreateInfoEXT* pCI,
                                     const KxAllocationCallbacks* PA,
                                     KxSwapchainEXT* pS)
{
    if (!dev || !pCI || !pS) return KX_ERROR_INVALID_ARGUMENT;
    kx::SoftwareDisplay* D = g_displays.empty() ? nullptr : g_displays[0].get();
    if (!D) return KX_ERROR_FEATURE_NOT_PRESENT;
    kx::SwapchainState S{};
    S.width = pCI->width; S.height = pCI->height;
    S.format = pCI->imageFormat; S.presentMode = pCI->presentMode;
    S.clipped = pCI->clipped; S.imageCount = 1;
    g_swapchain_states.push_back(S);
    // Create the swapchain image
    KxImageCreateInfo II{.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .format = pCI->imageFormat, .width = pCI->width, .height = pCI->height,
        .depth = 1, .mipLevels = 1, .arrayLayers = 1,
        .samples = (KxSampleCountFlags)1, .usage = pCI->imageUsage,
        .sharingModeExclusive = KX_TRUE};
    KxResult r = kxCreateImage(dev, &II, PA, &D->swapchain_image);
    if (r != KX_SUCCESS) { g_swapchain_states.pop_back(); return r; }
    S.image = D->swapchain_image;
    *pS = reinterpret_cast<KxSwapchainEXT>(&g_swapchain_states.back());
    printf("[kromax] SwapchainEXT created\n");
    return KX_SUCCESS;
}

void KX_CALL kxDestroySwapchainEXT(KxDevice, KxSwapchainEXT, const KxAllocationCallbacks*) {}

KxResult KX_CALL kxGetSwapchainImagesEXT(KxDevice dev,
                                          KxSwapchainEXT s,
                                          uint32_t* pIC, KxImage* pI)
{
    if (!dev || !pIC || !pI || !s) return KX_ERROR_INVALID_ARGUMENT;
    uintptr_t h = reinterpret_cast<uintptr_t>(s);
    auto it = std::find_if(g_swapchain_states.begin(), g_swapchain_states.end(),
        [h](const kx::SwapchainState& v) { return reinterpret_cast<uintptr_t>(&v) == h; });
    if (it != g_swapchain_states.end()) { *pIC = it->imageCount; if (pI) pI[0] = it->image; }
    else { *pIC = 0; }
    return KX_SUCCESS;
}

KxResult KX_CALL kxAcquireNextImageEXT(KxDevice dev,
                                       KxSwapchainEXT swapchain,
                                       uint64_t timeout,
                                       KxSemaphore semaphore,
                                       KxFence fence,
                                       uint32_t* pII)
{
    (void)dev; (void)timeout; (void)semaphore; (void)fence;
    if (!swapchain || !pII) return KX_ERROR_INVALID_ARGUMENT;
    *pII = 0;
    return KX_SUCCESS;
}

KxResult KX_CALL kxQueuePresentEXT(KxQueue queue,
                                   const KxPresentInfoEXT* PPI)
{
    if (!queue || !PPI) return KX_ERROR_INVALID_ARGUMENT;
    if (g_displays.empty()) return KX_ERROR_FEATURE_NOT_PRESENT;
    kx::SoftwareDisplay* D = g_displays[0].get();
    if (!D || !D->backend) return KX_ERROR_FEATURE_NOT_PRESENT;
    printf("[kromax] QueuePresentEXT: frame presented\n");
    kx::BackendFramebuffer FB{.image = D->swapchain_image,
        .width = D->width, .height = D->height,
        .pitch = D->width * 4, .data = nullptr, .data_size = 0};
    return D->backend->present(&FB);
}

// Export format string helper
const char* kx_get_format_string(KxFormat f) {
    return kx_format_to_string(f);
}

// Global state accessor
std::vector<kx::SwapchainState>& kx_get_swapchain_states() { return g_swapchain_states; }

} // extern "C"
