#include "OffscreenBackend.h"
#include "PpmWriter.h"

#include <cstdio>
#include <cstring>

namespace kx {

KxResult OffscreenBackend::initialize(const void* /*pCreateInfo*/) {
    log_info("Offscreen (PPM) display backend initialized");
    return KX_SUCCESS;
}

KxResult OffscreenBackend::create_framebuffer(uint32_t width, uint32_t height,
                                              KxFormat format,
                                              BackendFramebuffer* out) {
    if (!out) return KX_ERROR_INVALID_ARGUMENT;
    uint32_t pixel_size = 4; /* RGBA8 */
    uint32_t pitch = width * pixel_size;
    std::size_t data_size = static_cast<std::size_t>(pitch) * height;

    out->data = std::malloc(data_size);
    if (!out->data) return KX_ERROR_OUT_OF_HOST_MEMORY;
    std::memset(out->data, 0, data_size);

    out->image = reinterpret_cast<KxImage>(out->data);
    out->width = width;
    out->height = height;
    out->pitch = pitch;
    out->data_size = data_size;
    return KX_SUCCESS;
}

void OffscreenBackend::destroy_framebuffer(BackendFramebuffer* fb) {
    if (!fb) return;
    std::free(fb->data);
    fb->data = nullptr;
    fb->data_size = 0;
}

KxResult OffscreenBackend::present(const BackendFramebuffer* fb) {
    if (!fb || !fb->data) return KX_ERROR_INVALID_ARGUMENT;

    char path[256];
    std::snprintf(path, sizeof(path), "%s", output_path_.c_str());

    /* Write PPM */
    FILE* f = std::fopen(path, "wb");
    if (!f) return KX_ERROR_INITIALIZATION_FAILED;
    std::fprintf(f, "P6\n%u %u\n255\n", fb->width, fb->height);
    /* PPM is RGB; our buffer is RGBA, skip alpha. */
    const uint8_t* src = static_cast<const uint8_t*>(fb->data);
    for (uint32_t y = 0; y < fb->height; ++y) {
        const uint8_t* row = src + y * fb->pitch;
        for (uint32_t x = 0; x < fb->width; ++x) {
            const uint8_t* px = row + x * 4;
            uint8_t rgb[3] = {px[0], px[1], px[2]};
            std::fwrite(rgb, 1, 3, f);
        }
    }
    std::fclose(f);
    ++frame_index_;
    log_info("Offscreen backend: frame written to PPM");
    return KX_SUCCESS;
}

KxResult OffscreenBackend::wait_vblank() {
    return KX_SUCCESS;
}

void OffscreenBackend::shutdown() {}

} /* namespace kx */