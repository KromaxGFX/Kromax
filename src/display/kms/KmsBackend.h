#ifndef KROMAX_SRC_DISPLAY_KMS_KMSBACKEND_H
#define KROMAX_SRC_DISPLAY_KMS_KMSBACKEND_H

/*
 * KmsBackend.h
 *
 * DRM/KMS display backend. Uses dumb buffers, modesetting, page flips and
 * vblank. Requires DRM master (run from a TTY without an active
 * compositor). Disabled unless libdrm is found.
 */

#ifdef KX_HAVE_LIBDRM

#include <xf86drm.h>
#include <xf86drmMode.h>

#include "IDisplayBackend.h"

namespace kx {

class KmsBackend : public IDisplayBackend {
public:
    KmsBackend() = default;
    ~KmsBackend() override = default;

    const char* name() const noexcept override { return "drm-kms"; }

    KxResult initialize(const void* /*pCreateInfo*/) override;
    KxResult create_framebuffer(uint32_t width, uint32_t height,
                                KxFormat format,
                                BackendFramebuffer* out) override;
    void destroy_framebuffer(BackendFramebuffer* fb) override;
    KxResult present(const BackendFramebuffer* fb) override;
    KxResult wait_vblank() override;
    void shutdown() override;

private:
    int drm_fd_ = -1;
    uint32_t connector_id_ = 0;
    uint32_t crtc_id_ = 0;
    uint32_t fb_id_ = 0;
    uint32_t bo_handle_ = 0;
    uint8_t* bo_map_ = nullptr;
    std::size_t bo_size_ = 0;
};

} /* namespace kx */

#endif /* KX_HAVE_LIBDRM */

#endif /* KROMAX_SRC_DISPLAY_KMS_KMSBACKEND_H */