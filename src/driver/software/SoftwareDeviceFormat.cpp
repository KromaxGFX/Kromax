/*
 * SoftwareDeviceFormat.cpp
 *
 * Format utilities.
 */

#include "SoftwareDevice.h"

namespace kx {

uint32_t SoftwareDevice::format_pixel_size(KxFormat format) {
    uint32_t pixel_size = 4; // default RGBA8
    switch (format) {
    case KX_FORMAT_R8_UNORM:
    case KX_FORMAT_R8_SNORM:
    case KX_FORMAT_R8_USCALED:
    case KX_FORMAT_R8_SSCALED:
    case KX_FORMAT_R8_UINT:
    case KX_FORMAT_R8_SINT:
        pixel_size = 1;
        break;
    case KX_FORMAT_R16_UNORM:
    case KX_FORMAT_R16_SNORM:
    case KX_FORMAT_R16_USCALED:
    case KX_FORMAT_R16_SSCALED:
    case KX_FORMAT_R16_UINT:
    case KX_FORMAT_R16_SINT:
    case KX_FORMAT_R16_SFLOAT:
        pixel_size = 2;
        break;
    case KX_FORMAT_R32_SFLOAT:
    case KX_FORMAT_D32_SFLOAT:
        pixel_size = 4;
        break;
    case KX_FORMAT_R8G8_UNORM:
    case KX_FORMAT_R8G8_SNORM:
    case KX_FORMAT_R8G8_UINT:
    case KX_FORMAT_R8G8_SINT:
        pixel_size = 2;
        break;
    case KX_FORMAT_R32G32_SFLOAT:
        pixel_size = 8;
        break;
    case KX_FORMAT_R32G32B32_SFLOAT:
        pixel_size = 12;
        break;
    case KX_FORMAT_R32G32B32A32_SFLOAT:
    case KX_FORMAT_B8G8R8A8_UNORM:
    case KX_FORMAT_B8G8R8A8_SRGB:
    case KX_FORMAT_R8G8B8A8_UNORM:
    case KX_FORMAT_R8G8B8A8_SRGB:
        pixel_size = 4;
        break;
    default:
        pixel_size = 4;
    }
    return pixel_size;
}

} /* namespace kx */