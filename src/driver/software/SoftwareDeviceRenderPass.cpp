/*
 * SoftwareDeviceRenderPass.cpp
 *
 * Render pass execution (begin/end/next subpass).
 */

#include "SoftwareDevice.h"

#include <cstdint>
#include <cstring>

#include "SoftwareCommandBuffer.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"

namespace kx {

static uint32_t format_pixel_size(KxFormat format) {
    switch (format) {
    case KX_FORMAT_R8_UNORM: case KX_FORMAT_R8_SNORM: case KX_FORMAT_R8_USCALED: case KX_FORMAT_R8_SSCALED: case KX_FORMAT_R8_UINT: case KX_FORMAT_R8_SINT: return 1;
    case KX_FORMAT_R16_UNORM: case KX_FORMAT_R16_SNORM: case KX_FORMAT_R16_USCALED: case KX_FORMAT_R16_SSCALED: case KX_FORMAT_R16_UINT: case KX_FORMAT_R16_SINT: case KX_FORMAT_R16_SFLOAT: return 2;
    case KX_FORMAT_R32_SFLOAT: case KX_FORMAT_D32_SFLOAT: return 4;
    case KX_FORMAT_R8G8_UNORM: case KX_FORMAT_R8G8_SNORM: case KX_FORMAT_R8G8_UINT: case KX_FORMAT_R8G8_SINT: return 2;
    case KX_FORMAT_R32G32_SFLOAT: return 8;
    case KX_FORMAT_R32G32B32_SFLOAT: return 12;
    case KX_FORMAT_R32G32B32A32_SFLOAT: return 16;
    case KX_FORMAT_B8G8R8A8_UNORM: case KX_FORMAT_B8G8R8A8_SRGB: case KX_FORMAT_R8G8B8A8_UNORM: case KX_FORMAT_R8G8B8A8_SRGB: return 4;
    case KX_FORMAT_D16_UNORM: return 2;
    case KX_FORMAT_D24_UNORM_S8_UINT: return 4;
    default: return 4;
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
    if (e == 0) h |= (uint16_t)(m >> 13);
    else if (e == 0xFF) h |= (uint16_t)(0x7C00u | (m ? 1u : 0u));
    else { int ne = (int)e - 127 + 15; if (ne >= 31) h |= 0x7C00u; else if (ne <= 0) h |= (uint16_t)((m | 0x800000u) >> (1 - ne)); else h |= (uint16_t)(ne << 10 | m >> 13); }
    return h;
}

static void write_colour_pixel(uint8_t* px, KxFormat f, const float c[4]) {
    float r = c[0], g = c[1], b = c[2], a = c[3];
    switch (f) {
    case KX_FORMAT_R8_UNORM:
    case KX_FORMAT_R8_SNORM:
    case KX_FORMAT_R8_USCALED:
    case KX_FORMAT_R8_SSCALED:
    case KX_FORMAT_R8_UINT:
    case KX_FORMAT_R8_SINT:
        px[0] = unorm_byte(r); return;
    case KX_FORMAT_R16_UNORM:
    case KX_FORMAT_R16_USCALED:
        *reinterpret_cast<uint16_t*>(px) = unorm16(r); return;
    case KX_FORMAT_R16_SNORM:
    case KX_FORMAT_R16_SSCALED:
        *reinterpret_cast<int16_t*>(px) = snorm16(r); return;
    case KX_FORMAT_R16_UINT:
        *reinterpret_cast<uint16_t*>(px) = uint16v(r); return;
    case KX_FORMAT_R16_SINT:
        *reinterpret_cast<int16_t*>(px) = sint16v(r); return;
    case KX_FORMAT_R16_SFLOAT:
        *reinterpret_cast<uint16_t*>(px) = float_to_half(r); return;
    case KX_FORMAT_R32_SFLOAT:
        *reinterpret_cast<float*>(px) = r; return;
    case KX_FORMAT_R8G8_UNORM:
    case KX_FORMAT_R8G8_SNORM:
    case KX_FORMAT_R8G8_UINT:
    case KX_FORMAT_R8G8_SINT:
        px[0] = unorm_byte(r); px[1] = unorm_byte(g); return;
    case KX_FORMAT_R32G32_SFLOAT:
        reinterpret_cast<float*>(px)[0] = r; reinterpret_cast<float*>(px)[1] = g; return;
    case KX_FORMAT_R32G32B32_SFLOAT:
        reinterpret_cast<float*>(px)[0] = r; reinterpret_cast<float*>(px)[1] = g; reinterpret_cast<float*>(px)[2] = b; return;
    case KX_FORMAT_R32G32B32A32_SFLOAT:
        reinterpret_cast<float*>(px)[0] = r; reinterpret_cast<float*>(px)[1] = g; reinterpret_cast<float*>(px)[2] = b; reinterpret_cast<float*>(px)[3] = a; return;
    case KX_FORMAT_B8G8R8A8_UNORM:
    case KX_FORMAT_B8G8R8A8_SRGB:
        px[0] = unorm_byte(b); px[1] = unorm_byte(g); px[2] = unorm_byte(r); px[3] = unorm_byte(a); return;
    case KX_FORMAT_R8G8B8A8_UNORM:
    case KX_FORMAT_R8G8B8A8_SRGB:
        px[0] = unorm_byte(r); px[1] = unorm_byte(g); px[2] = unorm_byte(b); px[3] = unorm_byte(a); return;
    default:
        px[0] = unorm_byte(r); px[1] = unorm_byte(g); px[2] = unorm_byte(b); px[3] = unorm_byte(a); return;
    }
}

void SoftwareDevice::execute_begin_render_pass(const CmdBeginRenderPass& cmd) {
    current_render_pass_ = cmd.render_pass;
    current_framebuffer_ = cmd.framebuffer;
    current_subpass_ = 0;

    if (cmd.framebuffer && cmd.framebuffer->color_attachments().size() > 0) {
        const std::vector<SoftwareImage*>& colours = cmd.framebuffer->color_attachments();
        size_t colour_count = colours.size();
        for (size_t i = 0; i < colour_count && i < cmd.clear_values.size(); ++i) {
            auto* img = colours[i];
            if (!img) continue;
            const auto& clear_val = cmd.clear_values[i];
            uint8_t* data = img->data();
            if (!data) continue;
            uint32_t width = img->width();
            uint32_t height = img->height();
            KxFormat format = img->format();
            uint32_t ps = format_pixel_size(format);
            size_t pitch = img->row_pitch();
            if (pitch == 0) pitch = static_cast<size_t>(width) * ps;
            for (uint32_t y = 0; y < height; ++y) {
                uint8_t* row = data + y * pitch;
                for (uint32_t x = 0; x < width; ++x)
                    write_colour_pixel(row + x * ps, format, clear_val.color);
            }
        }
    }

    if (cmd.framebuffer && cmd.framebuffer->depth_attachment()) {
        auto* img = cmd.framebuffer->depth_attachment();
        size_t colour_count = cmd.framebuffer->color_attachments().size();
        if (!img || cmd.clear_values.size() <= colour_count) return;
        const auto& clear_val = cmd.clear_values[colour_count];
        uint8_t* data = img->data();
        if (!data) return;
        uint32_t width = img->width();
        uint32_t height = img->height();
        KxFormat format = img->format();
        uint32_t ps = format_pixel_size(format);
        size_t pitch = img->row_pitch();
        if (pitch == 0) pitch = static_cast<size_t>(width) * ps;
        float depth = clear_val.depthStencil.depth;
        uint32_t stencil = clear_val.depthStencil.stencil;
        for (uint32_t y = 0; y < height; ++y) {
            uint8_t* row = data + y * pitch;
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t* px = row + x * ps;
                switch (format) {
                case KX_FORMAT_D32_SFLOAT:
                    *reinterpret_cast<float*>(px) = depth;
                    break;
                case KX_FORMAT_D16_UNORM: {
                    float v = depth;
                    if (v < 0.0f) v = 0.0f;
                    if (v > 1.0f) v = 1.0f;
                    *reinterpret_cast<uint16_t*>(px) = (uint16_t)(v * 65535.0f);
                    break;
                }
                case KX_FORMAT_D24_UNORM_S8_UINT: {
                    float v = depth;
                    if (v < 0.0f) v = 0.0f;
                    if (v > 1.0f) v = 1.0f;
                    uint32_t depth_bits = (uint32_t)(v * 16777215.0f);
                    uint32_t word = ((stencil & 0xFFu) << 24) | (depth_bits & 0xFFFFFFu);
                    px[0] = (uint8_t)(word & 0xFFu);
                    px[1] = (uint8_t)((word >> 8) & 0xFFu);
                    px[2] = (uint8_t)((word >> 16) & 0xFFu);
                    px[3] = (uint8_t)((word >> 24) & 0xFFu);
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

void SoftwareDevice::execute_end_render_pass(const CmdEndRenderPass& cmd) {
    (void)cmd;
    current_render_pass_ = nullptr;
    current_framebuffer_ = nullptr;
    current_subpass_ = 0;
}

void SoftwareDevice::execute_next_subpass(const CmdNextSubpass& cmd) {
    (void)cmd;
}

} /* namespace kx */
