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

    // Get sampler parameters (use defaults if no sampler)
    KxFilter mag_filter = KX_FILTER_NEAREST;
    (void)mag_filter; // Suppress unused warning for now; will be used when filtering is implemented
    KxSamplerAddressMode address_mode_u = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    KxSamplerAddressMode address_mode_v = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (pTexture->sampler) {
        // In a real implementation, we'd resolve the sampler handle and read its create info
        // For software driver, sampler is a SoftwareSampler* - but from C API it's opaque
        // We'll use defaults for now
    }

    // Clamp or wrap coordinates based on address mode
    auto wrap_coord = [](float coord, uint32_t /*size*/, KxSamplerAddressMode mode) -> float {
        if (mode == KX_SAMPLER_ADDRESS_MODE_REPEAT) {
            coord = coord - floorf(coord);
        } else if (mode == KX_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT) {
            float f = floorf(coord);
            coord = (static_cast<int>(f) % 2 == 0) ? (coord - f) : (1.0f - (coord - f));
        } else { // CLAMP_TO_EDGE, CLAMP_TO_BORDER
            coord = fmaxf(0.0f, fminf(1.0f, coord));
        }
        return coord;
    };

    float u_wrapped = wrap_coord(u, pTexture->width, address_mode_u);
    float v_wrapped = wrap_coord(v, pTexture->height, address_mode_v);

    // Convert to texel coordinates
    float tx = u_wrapped * static_cast<float>(pTexture->width);
    float ty = v_wrapped * static_cast<float>(pTexture->height);

    // Nearest neighbor sampling (simplified - no mipmap, no anisotropic)
    int ix = static_cast<int>(floorf(tx));
    int iy = static_cast<int>(floorf(ty));

    // Clamp to texture bounds
    ix = (ix < 0) ? 0 : (ix >= static_cast<int>(pTexture->width) ? pTexture->width - 1 : ix);
    iy = (iy < 0) ? 0 : (iy >= static_cast<int>(pTexture->height) ? pTexture->height - 1 : iy);

    // Calculate byte offset
    uint32_t row_pitch = pTexture->rowPitch ? pTexture->rowPitch : pTexture->width * 4; // Assume 4 bytes/pixel if not set
    const uint8_t* texels = static_cast<const uint8_t*>(pTexture->pTexels);
    const uint8_t* pixel = texels + iy * row_pitch + ix * 4;

    // Decode based on format (simplified - handle common formats)
    switch (pTexture->format) {
        case KX_FORMAT_R8G8B8A8_UNORM:
        case KX_FORMAT_R8G8B8A8_SRGB:
        case KX_FORMAT_B8G8R8A8_UNORM:
        case KX_FORMAT_B8G8R8A8_SRGB: {
            outColor[0] = pixel[0] / 255.0f;
            outColor[1] = pixel[1] / 255.0f;
            outColor[2] = pixel[2] / 255.0f;
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
            // Default to zero for unsupported formats
            outColor[0] = outColor[1] = outColor[2] = outColor[3] = 0.0f;
            return KX_ERROR_FORMAT_NOT_SUPPORTED;
        }
    }

    return KX_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif