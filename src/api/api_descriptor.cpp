/*
 * api_descriptor.cpp
 *
 * Descriptor set layout, descriptor pool, descriptor set, sampler, and texture sampling C API implementation.
 */

#include "api.h"
#include "api_internal.h"

#include "Device.h"
#include <cmath>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Descriptor Set Layout
 * ======================================================================= */

KxResult KX_CALL kxCreateDescriptorSetLayout(
    KxDevice device,
    const KxDescriptorSetLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDescriptorSetLayout* pLayout) {
    if (!pLayout) return KX_ERROR_INVALID_ARGUMENT;
    *pLayout = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_descriptor_set_layout)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_descriptor_set_layout(device, pCreateInfo, pAllocator, pLayout);
}

void KX_CALL kxDestroyDescriptorSetLayout(
    KxDevice device,
    KxDescriptorSetLayout layout,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_descriptor_set_layout) return;
    dev->dispatch()->destroy_descriptor_set_layout(device, layout, pAllocator);
}

/* =======================================================================
 * Descriptor Pool
 * ======================================================================= */

KxResult KX_CALL kxCreateDescriptorPool(
    KxDevice device,
    const KxDescriptorPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDescriptorPool* pPool) {
    if (!pPool) return KX_ERROR_INVALID_ARGUMENT;
    *pPool = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_descriptor_pool)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_descriptor_pool(device, pCreateInfo, pAllocator, pPool);
}

void KX_CALL kxDestroyDescriptorPool(
    KxDevice device,
    KxDescriptorPool pool,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_descriptor_pool) return;
    dev->dispatch()->destroy_descriptor_pool(device, pool, pAllocator);
}

KxResult KX_CALL kxResetDescriptorPool(
    KxDevice device,
    KxDescriptorPool pool,
    KxDescriptorPoolResetFlags flags) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->reset_descriptor_pool)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->reset_descriptor_pool(device, pool, flags);
}

/* =======================================================================
 * Descriptor Sets
 * ======================================================================= */

KxResult KX_CALL kxAllocateDescriptorSets(
    KxDevice device,
    const KxDescriptorSetAllocateInfo* pAllocateInfo,
    KxDescriptorSet* pDescriptorSets) {
    if (!pDescriptorSets) return KX_ERROR_INVALID_ARGUMENT;
    for (uint32_t i = 0; i < pAllocateInfo->descriptorSetCount; ++i) {
        pDescriptorSets[i] = nullptr;
    }
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->allocate_descriptor_sets)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->allocate_descriptor_sets(device, pAllocateInfo, pDescriptorSets);
}

KxResult KX_CALL kxFreeDescriptorSets(
    KxDevice device,
    KxDescriptorPool descriptorPool,
    uint32_t descriptorSetCount,
    const KxDescriptorSet* pDescriptorSets) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->free_descriptor_sets)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->free_descriptor_sets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void KX_CALL kxUpdateDescriptorSets(
    KxDevice device,
    uint32_t descriptorWriteCount,
    const KxWriteDescriptorSet* pDescriptorWrites,
    uint32_t descriptorCopyCount,
    const KxCopyDescriptorSet* pDescriptorCopies) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->update_descriptor_sets) return;
    dev->dispatch()->update_descriptor_sets(device, descriptorWriteCount, pDescriptorWrites,
                                            descriptorCopyCount, pDescriptorCopies);
}

/* =======================================================================
 * Sampler
 * ======================================================================= */

KxResult KX_CALL kxCreateSampler(
    KxDevice device,
    const KxSamplerCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSampler* pSampler) {
    if (!pSampler) return KX_ERROR_INVALID_ARGUMENT;
    *pSampler = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_sampler)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_sampler(device, pCreateInfo, pAllocator, pSampler);
}

void KX_CALL kxDestroySampler(
    KxDevice device,
    KxSampler sampler,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_sampler) return;
    dev->dispatch()->destroy_sampler(device, sampler, pAllocator);
}

/* =======================================================================
 * Texture Sampling
 * ======================================================================= */

KxResult KX_CALL kxSampleTexture(
    const KxBoundTexture* pTexture,
    float u,
    float v,
    float outColor[4]) {
    if (!pTexture || !outColor) return KX_ERROR_INVALID_ARGUMENT;

    // Validate texture
    if (!pTexture->pTexels || pTexture->width == 0 || pTexture->height == 0) {
        outColor[0] = outColor[1] = outColor[2] = outColor[3] = 0.0f;
        return KX_ERROR_FEATURE_NOT_PRESENT;
    }

    // Get sampler parameters from resolved sampler state
    KxSamplerState sampler_state = pTexture->samplerState;
    // Default values if not set
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

    // Clamp or wrap coordinates based on address mode
    auto wrap_coord = [](float coord, uint32_t size, KxSamplerAddressMode mode) -> float {
        if (mode == KX_SAMPLER_ADDRESS_MODE_REPEAT) {
            coord = coord - floorf(coord);
        } else if (mode == KX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT) {
            float f = floorf(coord);
            coord = (static_cast<int>(f) % 2 == 0) ? (coord - f) : (1.0f - (coord - f));
        } else if (mode == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) {
            // For clamp to border, we let it go out of bounds and handle in sampling
        } else { // CLAMP_TO_EDGE, CLAMP_TO_BORDER
            coord = fmaxf(0.0f, fminf(1.0f, coord));
        }
        return coord;
    };

    float u_wrapped = wrap_coord(u, pTexture->width, sampler_state.addressModeU);
    float v_wrapped = wrap_coord(v, pTexture->height, sampler_state.addressModeV);

    // Apply mip LOD bias
    float lod = sampler_state.mipLodBias;
    // For simplicity, we use level 0 (base level)
    // Full mipmap support would select level based on derivatives

    // Determine if we need bilinear filtering
    bool use_bilinear = (sampler_state.minFilter == KX_FILTER_LINEAR ||
                         sampler_state.magFilter == KX_FILTER_LINEAR);

    // Convert to texel coordinates
    float tx, ty;
    if (sampler_state.unnormalizedCoordinates) {
        tx = u_wrapped;
        ty = v_wrapped;
    } else {
        tx = u_wrapped * static_cast<float>(pTexture->width);
        ty = v_wrapped * static_cast<float>(pTexture->height);
    }

    // Sample
    auto sample_nearest = [&](int ix, int iy) -> void {
        // Clamp to texture bounds
        ix = (ix < 0) ? 0 : (ix >= static_cast<int>(pTexture->width) ? pTexture->width - 1 : ix);
        iy = (iy < 0) ? 0 : (iy >= static_cast<int>(pTexture->height) ? pTexture->height - 1 : iy);

        uint32_t row_pitch = pTexture->rowPitch ? pTexture->rowPitch : pTexture->width * 4;
        const uint8_t* texels = static_cast<const uint8_t*>(pTexture->pTexels);
        const uint8_t* pixel = texels + iy * row_pitch + ix * 4;

        // Decode based on format
        switch (pTexture->format) {
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
                if (sampler_state.srgbDecode) {
                    auto srgb_to_linear = [](float c) {
                        return (c <= 0.04045f) ? (c / 12.92f) : std::powf((c + 0.055f) / 1.055f, 2.4f);
                    };
                    r = srgb_to_linear(r);
                    g = srgb_to_linear(g);
                    b = srgb_to_linear(b);
                }
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
    };

    if (use_bilinear) {
        // Bilinear filtering
        float fx = tx - 0.5f;
        float fy = ty - 0.5f;
        int ix0 = static_cast<int>(floorf(fx));
        int iy0 = static_cast<int>(floorf(fy));
        int ix1 = ix0 + 1;
        int iy1 = iy0 + 1;

        float wx1 = fx - ix0;
        float wy1 = fy - iy0;
        float wx0 = 1.0f - wx1;
        float wy0 = 1.0f - wy1;

        float c00[4], c10[4], c01[4], c11[4];

        auto sample_to_array = [&](int ix, int iy, float* out) {
            if (ix < 0 || iy < 0 || ix >= static_cast<int>(pTexture->width) || iy >= static_cast<int>(pTexture->height)) {
                if (sampler_state.addressModeU == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
                    sampler_state.addressModeV == KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) {
                    // Border color
                    out[0] = out[1] = out[2] = out[3] = 0.0f;
                } else {
                    // Clamped - handled by wrap_coord
                    ix = (ix < 0) ? 0 : (ix >= static_cast<int>(pTexture->width) ? pTexture->width - 1 : ix);
                    iy = (iy < 0) ? 0 : (iy >= static_cast<int>(pTexture->height) ? pTexture->height - 1 : iy);
                }
            }
            uint32_t row_pitch = pTexture->rowPitch ? pTexture->rowPitch : pTexture->width * 4;
            const uint8_t* texels = static_cast<const uint8_t*>(pTexture->pTexels);
            const uint8_t* pixel = texels + iy * row_pitch + ix * 4;

            // Decode
            out[0] = pixel[0] / 255.0f;
            out[1] = pixel[1] / 255.0f;
            out[2] = pixel[2] / 255.0f;
            out[3] = pixel[3] / 255.0f;
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
    } else {
        // Nearest neighbor
        int ix = static_cast<int>(floorf(tx));
        int iy = static_cast<int>(floorf(ty));
        sample_nearest(ix, iy);
    }

    return KX_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif