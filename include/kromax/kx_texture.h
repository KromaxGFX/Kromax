#ifndef KX_TEXTURE_H
#define KX_TEXTURE_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Texture helpers
 *
 * Convenience helpers on top of the core image / image view / sampler model.
 * ======================================================================= */

typedef struct KxTextureInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    KxFormat format;
    KxImageLayout layout;
    uint32_t rowPitch;
    uint32_t slicePitch;
    uint64_t dataSize;
} KxTextureInfo;

/* Query the properties of an image. */
KX_API KxResult KX_CALL kxGetImageInfo(
    KxDevice device,
    KxImage image,
    KxTextureInfo* pInfo);

/* Create a 2D image and upload the given pixels.
 *
 * The pixel data is tightly packed and must match `format`. The image is
 * created with mipLevels levels and the data of every level is uploaded, so
 * `pPixels` must contain the full mip chain (largest level first). Pass
 * mipLevels = 1 to upload a single level. */
KX_API KxResult KX_CALL kxCreateTexture2D(
    KxDevice device,
    uint32_t width,
    uint32_t height,
    KxFormat format,
    const void* pPixels,
    uint32_t mipLevels,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage);

/* Generate the mip chain of an image using a box filter. The image must have
 * been created with more than one mip level and must not be bound as a
 * render target. Existing mip levels are overwritten. */
KX_API KxResult KX_CALL kxGenerateMipmaps(
    KxDevice device,
    KxImage image);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_TEXTURE_H */
