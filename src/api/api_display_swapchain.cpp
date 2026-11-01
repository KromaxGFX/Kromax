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
    kx::Device* device = kx::find_device_locked(dev);
    if (!device) return KX_ERROR_INVALID_ARGUMENT;
    if (!device->dispatch() || !device->dispatch()->create_swapchain)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    // Note: KxSwapchainCreateInfoEXT needs to be converted to KxSwapchainCreateInfoKHR
    // For now, use the Vulkan-specific create_swapchain if available
    return device->dispatch()->create_swapchain(dev, 
        reinterpret_cast<const KxSwapchainCreateInfoKHR*>(pCI), PA, 
        reinterpret_cast<KxSwapchainKHR*>(pS));
}

void KX_CALL kxDestroySwapchainEXT(KxDevice dev, KxSwapchainEXT s, const KxAllocationCallbacks* PA)
{
    kx::Device* device = kx::find_device_locked(dev);
    if (!device) return;
    if (device->dispatch() && device->dispatch()->destroy_swapchain)
        device->dispatch()->destroy_swapchain(dev, reinterpret_cast<KxSwapchainKHR>(s), PA);
}

KxResult KX_CALL kxGetSwapchainImagesEXT(KxDevice dev,
                                          KxSwapchainEXT s,
                                          uint32_t* pIC, KxImage* pI)
{
    (void)dev; (void)s; (void)pIC; (void)pI;
    // For Vulkan, this is handled by the swapchain object directly
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxAcquireNextImageEXT(KxDevice dev,
                                       KxSwapchainEXT swapchain,
                                       uint64_t timeout,
                                       KxSemaphore semaphore,
                                       KxFence fence,
                                       uint32_t* pII)
{
    kx::Device* device = kx::find_device_locked(dev);
    if (!device) return KX_ERROR_INVALID_ARGUMENT;
    if (!device->dispatch() || !device->dispatch()->acquire_next_image)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return device->dispatch()->acquire_next_image(dev, 
        reinterpret_cast<KxSwapchainKHR>(swapchain), timeout,
        semaphore, fence, pII);
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
