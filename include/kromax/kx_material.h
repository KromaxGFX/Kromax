#ifndef KX_MATERIAL_H
#define KX_MATERIAL_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"
#include "kx_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Materials
 *
 * A material bundles the parameters and textures needed to shade a surface.
 * When no shader module is supplied, the built-in physically based shader is
 * used, so a material is useful without writing a single line of shader code.
 *
 * The built-in shader expects the following descriptor layout:
 *
 *   set 0, binding 0 : uniform block  { baseColor, metallicRoughness,
 *                                         emissive, normalScale,
 *                                         occlusionStrength, alphaCutoff }
 *   set 0, binding 1 : base colour texture
 *   set 0, binding 2 : normal texture
 *   set 0, binding 3 : metallic / roughness texture
 *   set 0, binding 4 : emissive texture
 *   set 0, binding 5 : ambient occlusion texture
 * ======================================================================= */

typedef struct KxMaterialTextureBinding {
    KxStructureType sType;
    const void* pNext;
    KxMaterialTextureSlot slot;
    KxImageView imageView;      /* may be NULL to leave the slot empty */
    KxSampler sampler;          /* may be NULL for the default sampler */
} KxMaterialTextureBinding;

typedef struct KxMaterialCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxShaderModule shaderModule;  /* NULL selects the built-in PBR shader */
    float baseColor[4];
    float metallic;
    float roughness;
    float emissive[3];
    float normalScale;
    float occlusionStrength;
    float alphaCutoff;
    uint32_t textureCount;
    const KxMaterialTextureBinding* pTextures;
} KxMaterialCreateInfo;

/* Create a material. */
KX_API KxResult KX_CALL kxCreateMaterial(
    KxDevice device,
    const KxMaterialCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial);

/* Destroy a material. */
KX_API void KX_CALL kxDestroyMaterial(
    KxDevice device,
    KxMaterial material,
    const KxAllocationCallbacks* pAllocator);

/* Convenience: create a material using the built-in PBR shader and default
 * parameters (white base colour, metallic 0, roughness 1). */
KX_API KxResult KX_CALL kxCreateDefaultMaterial(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial);

/* Assign a texture to a slot. Pass a NULL image view to clear the slot. */
KX_API KxResult KX_CALL kxMaterialSetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView imageView,
    KxSampler sampler);

/* Query the texture currently assigned to a slot. Either output pointer may
 * be NULL. */
KX_API KxResult KX_CALL kxMaterialGetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView* pImageView,
    KxSampler* pSampler);

/* Set a named material parameter, for example "baseColor", "metallic",
 * "roughness", "emissive", "normalScale", "occlusionStrength" or
 * "alphaCutoff". Unknown names are rejected. */
KX_API KxResult KX_CALL kxMaterialSetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    const void* pValue,
    size_t size);

/* Read a named material parameter back. */
KX_API KxResult KX_CALL kxMaterialGetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    void* pValue,
    size_t size);

/* Publish the material resources into a descriptor set so a pipeline can
 * consume it. The descriptor set layout must contain the bindings expected
 * by the material shader. Textures are written at
 * firstBinding + KxMaterialTextureSlot. */
KX_API KxResult KX_CALL kxMaterialUpdateDescriptorSet(
    KxDevice device,
    KxMaterial material,
    KxDescriptorSet descriptorSet,
    uint32_t firstBinding);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_MATERIAL_H */
