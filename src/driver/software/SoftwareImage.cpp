#include "SoftwareImage.h"

#include <cstdint>
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

static uint8_t unorm_byte(float c){ if(c<0.0f)c=0.0f; if(c>1.0f)c=1.0f; return (uint8_t)(c*255.0f);}
static int8_t snorm_byte(float c){ if(c<-1.0f)c=-1.0f; if(c>1.0f)c=1.0f; return (int8_t)(c*127.0f);}
static uint8_t uint_byte(float c){ if(c<0.0f)c=0.0f; if(c>255.0f)c=255.0f; return (uint8_t)c;}
static int8_t sint_byte(float c){ if(c<-128.0f)c=-128.0f; if(c>127.0f)c=127.0f; return (int8_t)c;}
static uint16_t unorm16(float c){ if(c<0.0f)c=0.0f; if(c>1.0f)c=1.0f; return (uint16_t)(c*65535.0f);}
static int16_t snorm16(float c){ if(c<-1.0f)c=-1.0f; if(c>1.0f)c=1.0f; return (int16_t)(c*32767.0f);}
static uint16_t uint16v(float c){ if(c<0.0f)c=0.0f; if(c>65535.0f)c=65535.0f; return (uint16_t)c;}
static int16_t sint16v(float c){ if(c<-32768.0f)c=-32768.0f; if(c>32767.0f)c=32767.0f; return (int16_t)c;}

static uint16_t float_to_half(float value) {
    uint32_t f;
    std::memcpy(&f, &value, sizeof(f));
    uint32_t s = (f >> 31) & 1u, e = (f >> 23) & 0xFFu, m = f & 0x7FFFFFu;
    uint16_t h = (uint16_t)(s << 15);
    if (e == 0)
        h |= (uint16_t)(m >> 13);
    else if (e == 0xFF)
        h |= (uint16_t)(0x7C00u | (m ? 1u : 0u));
    else {
        int ne = (int)e - 127 + 15;
        if (ne >= 31)
            h |= 0x7C00u;
        else if (ne <= 0)
            h |= (uint16_t)((m | 0x800000u) >> (1 - ne));
        else
            h |= (uint16_t)(ne << 10 | m >> 13);
    }
    return h;
}

SoftwareImage::SoftwareImage(KxImageCreateInfo create_info,
                             const KxAllocationCallbacks* /*allocator*/) noexcept
    : create_info_(create_info) {
    uint32_t pixel_size = format_pixel_size(create_info.format);
    row_pitch_ = create_info.width * pixel_size;
}

void SoftwareImage::clear_color(float r, float g, float b, float a) {
    if (!memory_) return;
    KxFormat format = create_info_.format;
    uint32_t ps = format_pixel_size(format);
    size_t pitch = row_pitch_;
    if (pitch == 0) pitch = static_cast<size_t>(create_info_.width) * ps;
    uint8_t* data = memory_->data();
    for (uint32_t y = 0; y < create_info_.height; ++y) {
        uint8_t* row = data + y * pitch;
        for (uint32_t x = 0; x < create_info_.width; ++x) {
            uint8_t* px = row + x * ps;
            switch (format) {
            case KX_FORMAT_R8_UNORM:
            case KX_FORMAT_R8_SNORM:
            case KX_FORMAT_R8_USCALED:
            case KX_FORMAT_R8_SSCALED:
            case KX_FORMAT_R8_UINT:
            case KX_FORMAT_R8_SINT:
                px[0] = unorm_byte(r);
                break;
            case KX_FORMAT_R16_UNORM:
            case KX_FORMAT_R16_USCALED:
                *reinterpret_cast<uint16_t*>(px) = unorm16(r);
                break;
            case KX_FORMAT_R16_SNORM:
            case KX_FORMAT_R16_SSCALED:
                *reinterpret_cast<int16_t*>(px) = snorm16(r);
                break;
            case KX_FORMAT_R16_UINT:
                *reinterpret_cast<uint16_t*>(px) = uint16v(r);
                break;
            case KX_FORMAT_R16_SINT:
                *reinterpret_cast<int16_t*>(px) = sint16v(r);
                break;
            case KX_FORMAT_R16_SFLOAT:
                *reinterpret_cast<uint16_t*>(px) = float_to_half(r);
                break;
            case KX_FORMAT_R32_SFLOAT:
                *reinterpret_cast<float*>(px) = r;
                break;
            case KX_FORMAT_R8G8_UNORM:
            case KX_FORMAT_R8G8_SNORM:
            case KX_FORMAT_R8G8_UINT:
            case KX_FORMAT_R8G8_SINT:
                px[0] = unorm_byte(r);
                px[1] = unorm_byte(g);
                break;
            case KX_FORMAT_R32G32_SFLOAT:
                reinterpret_cast<float*>(px)[0] = r;
                reinterpret_cast<float*>(px)[1] = g;
                break;
            case KX_FORMAT_R32G32B32_SFLOAT:
                reinterpret_cast<float*>(px)[0] = r;
                reinterpret_cast<float*>(px)[1] = g;
                reinterpret_cast<float*>(px)[2] = b;
                break;
            case KX_FORMAT_R32G32B32A32_SFLOAT:
                reinterpret_cast<float*>(px)[0] = r;
                reinterpret_cast<float*>(px)[1] = g;
                reinterpret_cast<float*>(px)[2] = b;
                reinterpret_cast<float*>(px)[3] = a;
                break;
            case KX_FORMAT_B8G8R8A8_UNORM:
            case KX_FORMAT_B8G8R8A8_SRGB:
                px[0] = unorm_byte(b);
                px[1] = unorm_byte(g);
                px[2] = unorm_byte(r);
                px[3] = unorm_byte(a);
                break;
            case KX_FORMAT_R8G8B8A8_UNORM:
            case KX_FORMAT_R8G8B8A8_SRGB:
                px[0] = unorm_byte(r);
                px[1] = unorm_byte(g);
                px[2] = unorm_byte(b);
                px[3] = unorm_byte(a);
                break;
            default:
                px[0] = unorm_byte(r);
                px[1] = unorm_byte(g);
                px[2] = unorm_byte(b);
                px[3] = unorm_byte(a);
                break;
            }
        }
    }
}

} /* namespace kx */
