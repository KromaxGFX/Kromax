#include "SoftwareImage.h"

#include <cstring>

namespace kx {

static uint32_t format_pixel_size(KxFormat format) {
    switch (format) {
    case KX_FORMAT_R8_UNORM:
    case KX_FORMAT_R8_SNORM:
    case KX_FORMAT_R8_USCALED:
    case KX_FORMAT_R8_SSCALED:
    case KX_FORMAT_R8_UINT:
    case KX_FORMAT_R8_SINT:
        return 1;
    case KX_FORMAT_R16_UNORM:
    case KX_FORMAT_R16_SNORM:
    case KX_FORMAT_R16_USCALED:
    case KX_FORMAT_R16_SSCALED:
    case KX_FORMAT_R16_UINT:
    case KX_FORMAT_R16_SINT:
    case KX_FORMAT_R16_SFLOAT:
        return 2;
    case KX_FORMAT_R32_SFLOAT:
    case KX_FORMAT_D32_SFLOAT:
        return 4;
    case KX_FORMAT_R8G8_UNORM:
    case KX_FORMAT_R8G8_SNORM:
    case KX_FORMAT_R8G8_UINT:
    case KX_FORMAT_R8G8_SINT:
        return 2;
    case KX_FORMAT_R32G32_SFLOAT:
        return 8;
    case KX_FORMAT_R32G32B32_SFLOAT:
        return 12;
    case KX_FORMAT_R32G32B32A32_SFLOAT:
        return 16;
    case KX_FORMAT_B8G8R8A8_UNORM:
    case KX_FORMAT_B8G8R8A8_SRGB:
    case KX_FORMAT_R8G8B8A8_UNORM:
    case KX_FORMAT_R8G8B8A8_SRGB:
        return 4;
    case KX_FORMAT_D16_UNORM:
        return 2;
    case KX_FORMAT_D24_UNORM_S8_UINT:
        return 4;
    default:
        return 4;
    }
}

SoftwareImage::SoftwareImage(KxImageCreateInfo create_info,
                             const KxAllocationCallbacks* /*allocator*/) noexcept
    : create_info_(create_info) {
    uint32_t pixel_size = format_pixel_size(create_info.format);
    row_pitch_ = create_info.width * pixel_size;
    std::size_t data_size =
        static_cast<std::size_t>(row_pitch_) * create_info.height *
        create_info.arrayLayers;
    data_.assign(data_size, 0);
}

void SoftwareImage::clear_color(float r, float g, float b, float a) {
    uint8_t r8 = static_cast<uint8_t>(r * 255.0f);
    uint8_t g8 = static_cast<uint8_t>(g * 255.0f);
    uint8_t b8 = static_cast<uint8_t>(b * 255.0f);
    uint8_t a8 = static_cast<uint8_t>(a * 255.0f);

    /* Assume RGBA8 for now */
    uint32_t pixel_size = format_pixel_size(create_info_.format);
    for (uint32_t y = 0; y < create_info_.height; ++y) {
        uint8_t* row = data_.data() + y * row_pitch_;
        for (uint32_t x = 0; x < create_info_.width; ++x) {
            uint8_t* px = row + x * pixel_size;
            if (pixel_size >= 4) {
                px[0] = r8;
                px[1] = g8;
                px[2] = b8;
                px[3] = a8;
            } else if (pixel_size == 1) {
                px[0] = r8;
            }
        }
    }
}

} /* namespace kx */