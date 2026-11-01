#ifndef KROMAX_SRC_API_SHADER_TEXTUREHELPERS_H
#define KROMAX_SRC_API_SHADER_TEXTUREHELPERS_H

/*
 * TextureHelpers.h
 * Texture creation, mipmap generation, and sampler utilities.
 */

#include "kromax.h"
#include "kx_core.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Get image info (dimensions, format, layout, etc.)
KxResult KX_CALL kxGetImageInfo(
    KxDevice device,
    KxImage image,
    KxTextureInfo* pInfo);

// Create 2D texture with optional pixel data
KxResult KX_CALL kxCreateTexture2D(
    KxDevice device,
    uint32_t width,
    uint32_t height,
    KxFormat format,
    const void* pPixels,
    uint32_t mipLevels,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage);

// Generate mipmaps for an image
KxResult KX_CALL kxGenerateMipmaps(
    KxDevice device,
    KxImage image);

// Get default sampler state
KxSamplerState KX_CALL kxDefaultSamplerState(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // KROMAX_SRC_API_SHADER_TEXTUREHELPERS_H