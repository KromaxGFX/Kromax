#ifndef KROMAX_SRC_DISPLAY_OFFSCREEN_PPMWRITER_H
#define KROMAX_SRC_DISPLAY_OFFSCREEN_PPMWRITER_H

/*
 * PpmWriter.h
 *
 * Tiny PPM (Portable Pixmap) writer used by the offscreen backend.
 * Header-only, no external dependencies.
 */

#include <cstdio>
#include <cstdint>

namespace kx {

/* Write an RGBA8 buffer as a PPM P6 file (alpha dropped). */
inline bool write_ppm(const char* path, uint32_t width, uint32_t height,
                      uint32_t pitch, const uint8_t* rgba) {
    FILE* f = std::fopen(path, "wb");
    if (!f) return false;
    std::fprintf(f, "P6\n%u %u\n255\n", width, height);
    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* row = rgba + y * pitch;
        for (uint32_t x = 0; x < width; ++x) {
            const uint8_t* px = row + x * 4;
            uint8_t rgb[3] = {px[0], px[1], px[2]};
            std::fwrite(rgb, 1, 3, f);
        }
    }
    std::fclose(f);
    return true;
}

} /* namespace kx */

#endif /* KROMAX_SRC_DISPLAY_OFFSCREEN_PPMWRITER_H */