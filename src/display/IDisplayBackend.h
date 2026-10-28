#ifndef KROMAX_SRC_DISPLAY_IDISPLAYBACKEND_H
#define KROMAX_SRC_DISPLAY_IDISPLAYBACKEND_H

/*
 * IDisplayBackend.h
 *
 * Abstract display backend interface. The Kromax display extension
 * (KxDisplayEXT / KxSwapchainEXT) is implemented by backends such as
 * DRM/KMS and offscreen (PPM).
 */

#include <cstdint>
#include <memory>
#include <vector>

#include "kx_core.h"
#include "kx_display.h"

namespace kx {

struct BackendFramebuffer {
    KxImage image;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    void* data;
    std::size_t data_size;
};

class IDisplayBackend {
public:
    virtual ~IDisplayBackend() = default;

    virtual const char* name() const noexcept = 0;

    /*
     * Initialize the backend. pCreateInfo is the KxDisplayCreateInfoEXT.
     */
    virtual KxResult initialize(const void* pCreateInfo) = 0;

    /*
     * Create a framebuffer (dumb buffer / PPM target) of the given size.
     */
    virtual KxResult create_framebuffer(uint32_t width, uint32_t height,
                                        KxFormat format,
                                        BackendFramebuffer* out) = 0;

    /*
     * Destroy a framebuffer.
     */
    virtual void destroy_framebuffer(BackendFramebuffer* fb) = 0;

    /*
     * Present a framebuffer to the display.
     */
    virtual KxResult present(const BackendFramebuffer* fb) = 0;

    /*
     * Wait for vblank / frame completion.
     */
    virtual KxResult wait_vblank() = 0;

    virtual void shutdown() = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DISPLAY_IDISPLAYBACKEND_H */