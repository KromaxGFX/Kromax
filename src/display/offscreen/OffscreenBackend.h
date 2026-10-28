#ifndef KROMAX_SRC_DISPLAY_OFFSCREEN_OFFSCREENBACKEND_H
#define KROMAX_SRC_DISPLAY_OFFSCREEN_OFFSCREENBACKEND_H

/*
 * OffscreenBackend.h
 *
 * Offscreen display backend: writes rendered frames to PPM files.
 * Default backend for CI and desktop development (no DRM master needed).
 */

#include <cstdint>
#include <string>

#include "IDisplayBackend.h"

namespace kx {

class OffscreenBackend : public IDisplayBackend {
public:
    OffscreenBackend() = default;
    ~OffscreenBackend() override = default;

    const char* name() const noexcept override { return "offscreen-ppm"; }

    KxResult initialize(const void* /*pCreateInfo*/) override;
    KxResult create_framebuffer(uint32_t width, uint32_t height,
                                KxFormat format,
                                BackendFramebuffer* out) override;
    void destroy_framebuffer(BackendFramebuffer* fb) override;
    KxResult present(const BackendFramebuffer* fb) override;
    KxResult wait_vblank() override;
    void shutdown() override;

    void set_output_path(std::string path) { output_path_ = std::move(path); }
    const std::string& output_path() const noexcept { return output_path_; }

private:
    std::string output_path_ = "./kromax_frame.ppm";
    uint32_t frame_index_ = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DISPLAY_OFFSCREEN_OFFSCREENBACKEND_H */