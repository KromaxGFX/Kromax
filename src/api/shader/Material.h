#ifndef KROMAX_SRC_API_SHADER_MATERIAL_H
#define KROMAX_SRC_API_SHADER_MATERIAL_H

/*
 * Material.h
 * Material creation, parameters, textures, and descriptor set updates.
 */

#include "kromax.h"
#include "kx_core.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Material lifecycle
KxResult KX_CALL kxCreateMaterial(
    KxDevice device,
    const KxMaterialCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial);

void KX_CALL kxDestroyMaterial(
    KxDevice device,
    KxMaterial material,
    const KxAllocationCallbacks* pAllocator);

// Create default PBR material
KxResult KX_CALL kxCreateDefaultMaterial(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial);

// Texture slot management
KxResult KX_CALL kxMaterialSetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView imageView,
    KxSampler sampler);

KxResult KX_CALL kxMaterialGetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView* pImageView,
    KxSampler* pSampler);

// Parameter get/set by name (float/vec types)
KxResult KX_CALL kxMaterialSetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    const void* pValue,
    size_t size);

KxResult KX_CALL kxMaterialGetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    void* pValue,
    size_t size);

// Update descriptor set from material
KxResult KX_CALL kxMaterialUpdateDescriptorSet(
    KxDevice device,
    KxMaterial material,
    KxDescriptorSet descriptorSet,
    uint32_t firstBinding);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // KROMAX_SRC_API_SHADER_MATERIAL_H