#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGE_H

/*
 * SoftwareImage.h
 *
 * Software image: a 2D pixel buffer (RGBA8 or depth). Used by the
 * software rasterizer and the offscreen display backend.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareImage {
public:
    SoftwareImage(KxImageCreateInfo create_info,
                  const KxAllocationCallbacks* allocator) noexcept;
    ~SoftwareImage() = default;

    SoftwareImage(const SoftwareImage&) = delete;
    SoftwareImage& operator=(const SoftwareImage&) = delete;

    KxImageCreateInfo create_info() const noexcept { return create_info_; }
    uint32_t width() const noexcept { return create_info_.width; }
    uint32_t height() const noexcept { return create_info_.height; }
    KxFormat format() const noexcept { return create_info_.format; }
    KxImageLayout layout() const noexcept { return layout_; }
    void set_layout(KxImageLayout layout) noexcept { layout_ = layout; }

    /* Raw pixel access (format dependent) */
    uint8_t* data() noexcept { return data_.data(); }
    const uint8_t* data() const noexcept { return data_.data(); }
    std::size_t data_size() const noexcept { return data_.size(); }
    std::size_t row_pitch() const noexcept { return row_pitch_; }

    /* Clear the whole image to a color (RGBA8) */
    void clear_color(float r, float g, float b, float a);

private:
    KxImageCreateInfo create_info_;
    KxImageLayout layout_ = KX_IMAGE_LAYOUT_UNDEFINED;
    std::vector<uint8_t> data_;
    std::size_t row_pitch_ = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREIMAGE_H */