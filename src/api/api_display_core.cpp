#include <algorithm>
/* api_display_core.cpp - Display extension core (KxDisplayEXT)
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
#include "KmsBackend.h"

#include <cstdio>
#include <vector>

namespace kx {

struct SoftwareDisplay {
    IDisplayBackend* backend = nullptr;
    uint32_t width = 0, height = 0;
    KxImage swapchain_image;
};

} // namespace kx

static std::vector<std::unique_ptr<kx::SoftwareDisplay>> g_displays;

const char* kx_format_to_string(KxFormat f) {
    static char b[32]; snprintf(b, sizeof(b), "FMT_%d", (int)f); return b;
}

extern "C" {

KxResult KX_CALL kxCreateDisplayEXT(KxInstance inst,
                                   const KxDisplayCreateInfoEXT* pCI,
                                   const KxAllocationCallbacks* PA,
                                   KxDisplayEXT* pD)
{
    if (!inst || !pCI || !pD) return KX_ERROR_INVALID_ARGUMENT;
    auto* d = new (std::nothrow) kx::SoftwareDisplay();
    if (!d) return KX_ERROR_OUT_OF_HOST_MEMORY;
    KxResult r = KX_ERROR_FEATURE_NOT_PRESENT;
    auto* ob = new (std::nothrow) kx::OffscreenBackend();
    if (ob && ob->initialize(pCI) == KX_SUCCESS) {
        d->backend = ob; d->width = 800; d->height = 600;
    } else { delete ob; }
    if (!d->backend) { delete d; return KX_ERROR_FEATURE_NOT_PRESENT; }
    g_displays.push_back(std::unique_ptr<kx::SoftwareDisplay>(d));
    *pD = reinterpret_cast<KxDisplayEXT>(d);
    printf("[kromax] DisplayEXT: %s %ux%u\n", d->backend->name(), d->width, d->height);
    return KX_SUCCESS;
}

void KX_CALL kxDestroyDisplayEXT(KxInstance, KxDisplayEXT d, const KxAllocationCallbacks*) {
    if (!d) return;
    auto it = std::find_if(g_displays.begin(), g_displays.end(),
        [d](const std::unique_ptr<kx::SoftwareDisplay>& v) {
            return reinterpret_cast<KxDisplayEXT>(v.get()) == d; });
    if (it != g_displays.end()) g_displays.erase(it);
}

// Export kx_format_to_string for use in other files
const char* kx_get_format_string(KxFormat f) {
    return kx_format_to_string(f);
}

} // extern "C"
