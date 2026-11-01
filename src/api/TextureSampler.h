#ifndef KROMAX_SRC_API_TEXTURESAMPLER_H
#define KROMAX_SRC_API_TEXTURESAMPLER_H

/*
 * TextureSampler.h
 *
 * Texture sampling implementation (nearest, bilinear, trilinear).
 * Extracted from api_descriptor.cpp for reusability and testing.
 */

#include "kx_core.h"
#include "kx_structs.h"

#include <algorithm>
#include <cmath>

namespace kx {
namespace api {

/* -----------------------------------------------------------------------
 * Mipmap utilities
 * --------------------------------------------------------------------- */
inline float compute_lod_from_gradients(float dudx, float dvdx, float dudy, float dvdy,
                                         float tex_width, float tex_height) {
    float dx = std::sqrt(dudx * dudx + dudy * dudy) * tex_width;
    float dy = std::sqrt(dvdx * dvdx + dvdy * dvdy) * tex_height;
    float max_deriv = std::max(dx, dy);
    if (max_deriv <= 0.0f) return 0.0f;
    return std::log2(max_deriv);
}

/* -----------------------------------------------------------------------
 * Texture format decoding
 * --------------------------------------------------------------------- */
inline void decode_pixel_format(KxFormat format, const uint8_t* pixel, float outColor[4]) {
    switch (format) {
        case KX_FORMAT_R8G8B8A8_UNORM:
        case KX_FORMAT_B8G8R8A8_UNORM: {
            outColor[0] = pixel[0] / 255.0f;
            outColor[1] = pixel[1] / 255.0f;
            outColor[2] = pixel[2] / 255.0f;
            outColor[3] = pixel[3] / 255.0f;
            break;
        }
        case KX_FORMAT_R8G8B8A8_SRGB:
        case KX_FORMAT_B8G8R8A8_SRGB: {
            float r = pixel[0] / 255.0f;
            float g = pixel[1] / 255.0f;
            float b = pixel[2] / 255.0f;
            auto srgb_to_linear = [](float c) {
                return (c <= 0.04045f) ? (c / 12.92f) : std::powf((c + 0.055f) / 1.055f, 2.4f);
            };
            r = srgb_to_linear(r);
            g = srgb_to_linear(g);
            b = srgb_to_linear(b);
            outColor[0] = r;
            outColor[1] = g;
            outColor[2] = b;
            outColor[3] = pixel[3] / 255.0f;
            break;
        }
        case KX_FORMAT_R8_UNORM: {
            outColor[0] = pixel[0] / 255.0f;
            outColor[1] = outColor[2] = 0.0f;
            outColor[3] = 1.0f;
            break;
        }
        case KX_FORMAT_R16_SFLOAT: {
            const uint16_t* p16 = reinterpret_cast<const uint16_t*>(pixel);
            outColor[0] = __builtin_bit_cast(float, static_cast<uint32_t>(p16[0]) << 16);
            outColor[1] = outColor[2] = 0.0f;
            outColor[3] = 1.0f;
            break;
        }
        case KX_FORMAT_R32_SFLOAT: {
            const float* pf = reinterpret_cast<const float*>(pixel);
            outColor[0] = pf[0];
            outColor[1] = outColor[2] = 0.0f;
            outColor[3] = 1.0f;
            break;
        }
        case KX_FORMAT_R32G32B32A32_SFLOAT: {
            const float* pf = reinterpret_cast<const float*>(pixel);
            outColor[0] = pf[0];
            outColor[1] = pf[1];
            outColor[2] = pf[2];
            outColor[3] = pf[3];
            break;
        }
        default: {
            outColor[0] = outColor[1] = outColor[2] = outColor[3] = 0.0f;
        }
    }
}

/* -----------------------------------------------------------------------
 * Coordinate wrapping
 * --------------------------------------------------------------------- */
inline float wrap_coord(float coord, uint32_t size, KxSamplerAddressMode mode) {
    if (mode == KX_SAMPLER_ADDRESS_MODE_REPEAT) {
        coord = coord - std::floor(coord);
    } else if (mode == KX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT) {
        float f = std::floor(coord);
        coord = (static_cast<int>(f) % 2 == 0) ? (coord - f) : (1.0f - (coord - f));
    } else if (mode == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) {
        // Let it go out of bounds, handle in sampling
    } else { // CLAMP_TO_EDGE
        coord = std::fmaxf(0.0f, std::fminf(1.0f, coord));
    }
    return coord;
}

/* -----------------------------------------------------------------------
 * Mip level sampling
 * --------------------------------------------------------------------- */
inline void sample_mip_level(const KxBoundTexture* pTexture, int mip_level,
                              int ix, int iy, float outColor[4],
                              const KxSamplerState& sampler_state) {
    uint32_t mip_width = std::max<uint32_t>(1, pTexture->width >> mip_level);
    uint32_t mip_height = std::max<uint32_t>(1, pTexture->height >> mip_level);

    ix = (ix < 0) ? 0 : (ix >= static_cast<int>(mip_width) ? mip_width - 1 : ix);
    iy = (iy < 0) ? 0 : (iy >= static_cast<int>(mip_height) ? mip_height - 1 : iy);

    uint32_t row_pitch = pTexture->rowPitch ? pTexture->rowPitch : pTexture->width * 4;
    uint32_t mip_offset = 0;
    for (uint32_t l = 0; l < static_cast<uint32_t>(mip_level); ++l) {
        uint32_t mw = std::max<uint32_t>(1, pTexture->width >> l);
        mip_offset += mw * row_pitch;
    }

    const uint8_t* texels = static_cast<const uint8_t*>(pTexture->pTexels);
    const uint8_t* pixel = texels + mip_offset + iy * row_pitch + ix * 4;

    decode_pixel_format(pTexture->format, pixel, outColor);
}

/* -----------------------------------------------------------------------
 * Bilinear sampling at a specific mip level
 * --------------------------------------------------------------------- */
inline void sample_bilinear_mip(const KxBoundTexture* pTexture, int mip_level,
                                 float tx, float ty, float outColor[4],
                                 const KxSamplerState& sampler_state) {
    uint32_t mip_width = std::max<uint32_t>(1, pTexture->width >> mip_level);
    uint32_t mip_height = std::max<uint32_t>(1, pTexture->height >> mip_level);

    float fx = tx - 0.5f;
    float fy = ty - 0.5f;
    int ix0 = static_cast<int>(std::floor(fx));
    int iy0 = static_cast<int>(std::floor(fy));
    int ix1 = ix0 + 1;
    int iy1 = iy0 + 1;

    float wx1 = fx - ix0;
    float wy1 = fy - iy0;
    float wx0 = 1.0f - wx1;
    float wy0 = 1.0f - wy1;

    float c00[4], c10[4], c01[4], c11[4];

    bool clamp_to_border_u = (sampler_state.addressModeU == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    bool clamp_to_border_v = (sampler_state.addressModeV == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);

    auto sample_to_array = [&](int ix, int iy, float* out) {
        if (ix < 0 || iy < 0 || ix >= static_cast<int>(mip_width) || iy >= static_cast<int>(mip_height)) {
            if (clamp_to_border_u || clamp_to_border_v) {
                out[0] = out[1] = out[2] = out[3] = 0.0f;
                return;
            } else {
                ix = (ix < 0) ? 0 : (ix >= static_cast<int>(mip_width) ? mip_width - 1 : ix);
                iy = (iy < 0) ? 0 : (iy >= static_cast<int>(mip_height) ? mip_height - 1 : iy);
            }
        }
        sample_mip_level(pTexture, mip_level, ix, iy, out, sampler_state);
    };

    sample_to_array(ix0, iy0, c00);
    sample_to_array(ix1, iy0, c10);
    sample_to_array(ix0, iy1, c01);
    sample_to_array(ix1, iy1, c11);

    for (int c = 0; c < 4; ++c) {
        float v0 = c00[c] * wx0 + c10[c] * wx1;
        float v1 = c01[c] * wx0 + c11[c] * wx1;
        outColor[c] = v0 * wy0 + v1 * wy1;
    }
}

/* -----------------------------------------------------------------------
 * Main sampling function (used by kxSampleTexture, kxSampleTextureGrad, kxSampleTextureLod)
 * --------------------------------------------------------------------- */
inline KxResult sample_texture_impl(const KxBoundTexture* pTexture,
                                     float u, float v,
                                     float dudx, float dvdx, float dudy, float dvdy,
                                     float lod, bool use_gradients, bool use_explicit_lod,
                                     float outColor[4]) {
    if (!pTexture || !outColor) return KX_ERROR_INVALID_ARGUMENT;

    if (!pTexture->pTexels || pTexture->width == 0 || pTexture->height == 0) {
        outColor[0] = outColor[1] = outColor[2] = outColor[3] = 0.0f;
        return KX_ERROR_FEATURE_NOT_PRESENT;
    }

    KxSamplerState sampler_state = pTexture->samplerState;
    if (sampler_state.magFilter == 0 && sampler_state.minFilter == 0) {
        sampler_state.magFilter = KX_FILTER_NEAREST;
        sampler_state.minFilter = KX_FILTER_NEAREST;
        sampler_state.addressModeU = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_state.addressModeV = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_state.addressModeW = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_state.mipLodBias = 0.0f;
        sampler_state.anisotropyEnable = KX_FALSE;
        sampler_state.maxAnisotropy = 1.0f;
        sampler_state.compareEnable = KX_FALSE;
        sampler_state.compareOp = KX_COMPARE_OP_ALWAYS;
        sampler_state.minLod = 0.0f;
        sampler_state.maxLod = 1000.0f;
        sampler_state.borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        sampler_state.unnormalizedCoordinates = KX_FALSE;
        sampler_state.maxLodLevel = 0;
        sampler_state.srgbDecode = KX_FALSE;
    }

    float u_wrapped = wrap_coord(u, pTexture->width, sampler_state.addressModeU);
    float v_wrapped = wrap_coord(v, pTexture->height, sampler_state.addressModeV);

    if (use_gradients) {
        lod = compute_lod_from_gradients(dudx, dvdx, dudy, dvdy,
                                          static_cast<float>(pTexture->width),
                                          static_cast<float>(pTexture->height));
    }

    if (use_gradients || use_explicit_lod) {
        lod += sampler_state.mipLodBias;
        lod = std::fmaxf(sampler_state.minLod, std::fminf(sampler_state.maxLod, lod));
    }

    float tx, ty;
    if (sampler_state.unnormalizedCoordinates) {
        tx = u_wrapped;
        ty = v_wrapped;
    } else {
        tx = u_wrapped * static_cast<float>(pTexture->width);
        ty = v_wrapped * static_cast<float>(pTexture->height);
    }

    bool use_mipmap = (sampler_state.minFilter == KX_FILTER_LINEAR && pTexture->mipLevels > 1);
    bool use_bilinear = (sampler_state.minFilter == KX_FILTER_LINEAR || sampler_state.magFilter == KX_FILTER_LINEAR);

    if (use_mipmap && use_bilinear && (use_gradients || use_explicit_lod)) {
        // Trilinear filtering
        int lod_floor = static_cast<int>(std::floor(lod));
        int lod_ceil = lod_floor + 1;
        lod_floor = std::max(0, std::min(lod_floor, static_cast<int>(pTexture->mipLevels) - 1));
        lod_ceil = std::max(0, std::min(lod_ceil, static_cast<int>(pTexture->mipLevels) - 1));

        float t = lod - lod_floor;

        float color0[4], color1[4];
        sample_bilinear_mip(pTexture, lod_floor, tx, ty, color0, sampler_state);
        sample_bilinear_mip(pTexture, lod_ceil, tx, ty, color1, sampler_state);

        for (int c = 0; c < 4; ++c) {
            outColor[c] = color0[c] * (1.0f - t) + color1[c] * t;
        }
    } else if (use_bilinear) {
        // Bilinear at base level
        sample_bilinear_mip(pTexture, 0, tx, ty, outColor, sampler_state);
    } else {
        // Nearest neighbor
        int ix = static_cast<int>(std::floor(tx));
        int iy = static_cast<int>(std::floor(ty));
        sample_mip_level(pTexture, 0, ix, iy, outColor, sampler_state);
    }

    return KX_SUCCESS;
}

} // namespace api
} // namespace kx

#endif // KROMAX_SRC_API_TEXTURESAMPLER_H