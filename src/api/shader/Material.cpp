/*
 * Material.cpp
 * Material creation, parameters, textures, and descriptor set updates implementation.
 */

#include "Material.h"

#include "api_internal.h"
#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareDescriptorSet.h"

#include <cstring>
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

// Internal material handle structure
struct MaterialHandle {
    KxShaderModule shader_module = nullptr;
    float base_color[4] = {1, 1, 1, 1};
    float metallic = 0.0f;
    float roughness = 1.0f;
    float emissive[3] = {0, 0, 0};
    float normal_scale = 1.0f;
    float occlusion_strength = 1.0f;
    float alpha_cutoff = 0.5f;
    KxImageView textures[6] = {};
    KxSampler samplers[6] = {};
};

KxResult KX_CALL kxCreateMaterial(
    KxDevice device,
    const KxMaterialCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial) {
    if (!pMaterial) return KX_ERROR_INVALID_ARGUMENT;
    *pMaterial = nullptr;
    if (!pCreateInfo) return KX_ERROR_INVALID_ARGUMENT;
    if (pCreateInfo->sType != KX_STRUCTURE_TYPE_MATERIAL_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto handle = reinterpret_cast<KxMaterial>(new MaterialHandle);
    if (!handle) return KX_ERROR_OUT_OF_HOST_MEMORY;

    MaterialHandle* mat = reinterpret_cast<MaterialHandle*>(handle);
    mat->shader_module = pCreateInfo->shaderModule;
    if (pCreateInfo->pTextures && pCreateInfo->textureCount > 0) {
        for (uint32_t i = 0; i < pCreateInfo->textureCount && i < 6; ++i) {
            const auto& tb = pCreateInfo->pTextures[i];
            if (tb.slot < 6) {
                mat->textures[tb.slot] = tb.imageView;
                mat->samplers[tb.slot] = tb.sampler;
            }
        }
    }
    if (pCreateInfo->baseColor) {
        mat->base_color[0] = pCreateInfo->baseColor[0];
        mat->base_color[1] = pCreateInfo->baseColor[1];
        mat->base_color[2] = pCreateInfo->baseColor[2];
        mat->base_color[3] = pCreateInfo->baseColor[3];
    }
    mat->metallic = pCreateInfo->metallic;
    mat->roughness = pCreateInfo->roughness;
    mat->emissive[0] = pCreateInfo->emissive[0];
    mat->emissive[1] = pCreateInfo->emissive[1];
    mat->emissive[2] = pCreateInfo->emissive[2];
    mat->normal_scale = pCreateInfo->normalScale;
    mat->occlusion_strength = pCreateInfo->occlusionStrength;
    mat->alpha_cutoff = pCreateInfo->alphaCutoff;

    *pMaterial = handle;
    return KX_SUCCESS;
}

void KX_CALL kxDestroyMaterial(
    KxDevice device,
    KxMaterial material,
    const KxAllocationCallbacks* pAllocator) {
    (void)device;
    (void)pAllocator;
    if (material) {
        delete reinterpret_cast<MaterialHandle*>(material);
    }
}

KxResult KX_CALL kxCreateDefaultMaterial(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxMaterial* pMaterial) {
    KxMaterialCreateInfo create_info{};
    create_info.sType = KX_STRUCTURE_TYPE_MATERIAL_CREATE_INFO;
    create_info.baseColor[0] = 1.0f;
    create_info.baseColor[1] = 1.0f;
    create_info.baseColor[2] = 1.0f;
    create_info.baseColor[3] = 1.0f;
    create_info.metallic = 0.0f;
    create_info.roughness = 1.0f;
    create_info.emissive[0] = 0.0f;
    create_info.emissive[1] = 0.0f;
    create_info.emissive[2] = 0.0f;
    create_info.normalScale = 1.0f;
    create_info.occlusionStrength = 1.0f;
    create_info.alphaCutoff = 0.5f;
    create_info.shaderModule = nullptr;
    create_info.textureCount = 0;
    create_info.pTextures = nullptr;

    return kxCreateMaterial(device, &create_info, pAllocator, pMaterial);
}

KxResult KX_CALL kxMaterialSetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView imageView,
    KxSampler sampler) {
    (void)device;
    if (!material || slot >= 6) return KX_ERROR_INVALID_ARGUMENT;

    auto* mat = reinterpret_cast<MaterialHandle*>(material);
    mat->textures[slot] = imageView;
    mat->samplers[slot] = sampler;
    return KX_SUCCESS;
}

KxResult KX_CALL kxMaterialGetTexture(
    KxDevice device,
    KxMaterial material,
    KxMaterialTextureSlot slot,
    KxImageView* pImageView,
    KxSampler* pSampler) {
    (void)device;
    if (!material || slot >= 6) return KX_ERROR_INVALID_ARGUMENT;

    auto* mat = reinterpret_cast<MaterialHandle*>(material);
    if (pImageView) *pImageView = mat->textures[slot];
    if (pSampler) *pSampler = mat->samplers[slot];
    return KX_SUCCESS;
}

KxResult KX_CALL kxMaterialSetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    const void* pValue,
    size_t size) {
    (void)device;
    if (!material || !pName || !pValue) return KX_ERROR_INVALID_ARGUMENT;

    auto* mat = reinterpret_cast<MaterialHandle*>(material);

    if (std::strcmp(pName, "baseColor") == 0 && size == sizeof(float) * 4) {
        std::memcpy(mat->base_color, pValue, size);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "metallic") == 0 && size == sizeof(float)) {
        mat->metallic = *static_cast<const float*>(pValue);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "roughness") == 0 && size == sizeof(float)) {
        mat->roughness = *static_cast<const float*>(pValue);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "emissive") == 0 && size == sizeof(float) * 3) {
        std::memcpy(mat->emissive, pValue, size);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "normalScale") == 0 && size == sizeof(float)) {
        mat->normal_scale = *static_cast<const float*>(pValue);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "occlusionStrength") == 0 && size == sizeof(float)) {
        mat->occlusion_strength = *static_cast<const float*>(pValue);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "alphaCutoff") == 0 && size == sizeof(float)) {
        mat->alpha_cutoff = *static_cast<const float*>(pValue);
        return KX_SUCCESS;
    }

    return KX_ERROR_INVALID_ARGUMENT;
}

KxResult KX_CALL kxMaterialGetParameter(
    KxDevice device,
    KxMaterial material,
    const char* pName,
    void* pValue,
    size_t size) {
    (void)device;
    if (!material || !pName || !pValue) return KX_ERROR_INVALID_ARGUMENT;

    auto* mat = reinterpret_cast<MaterialHandle*>(material);

    if (std::strcmp(pName, "baseColor") == 0 && size == sizeof(float) * 4) {
        std::memcpy(pValue, mat->base_color, size);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "metallic") == 0 && size == sizeof(float)) {
        *static_cast<float*>(pValue) = mat->metallic;
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "roughness") == 0 && size == sizeof(float)) {
        *static_cast<float*>(pValue) = mat->roughness;
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "emissive") == 0 && size == sizeof(float) * 3) {
        std::memcpy(pValue, mat->emissive, size);
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "normalScale") == 0 && size == sizeof(float)) {
        *static_cast<float*>(pValue) = mat->normal_scale;
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "occlusionStrength") == 0 && size == sizeof(float)) {
        *static_cast<float*>(pValue) = mat->occlusion_strength;
        return KX_SUCCESS;
    }
    if (std::strcmp(pName, "alphaCutoff") == 0 && size == sizeof(float)) {
        *static_cast<float*>(pValue) = mat->alpha_cutoff;
        return KX_SUCCESS;
    }

    return KX_ERROR_INVALID_ARGUMENT;
}

KxResult KX_CALL kxMaterialUpdateDescriptorSet(
    KxDevice device,
    KxMaterial material,
    KxDescriptorSet descriptorSet,
    uint32_t firstBinding) {
    if (!material) return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* mat = reinterpret_cast<MaterialHandle*>(material);

    // Write material parameters as a uniform buffer at firstBinding
    // And textures at firstBinding + slot

    // This would need a uniform buffer allocation - simplified for now
    KxWriteDescriptorSet writes[7];
    uint32_t write_count = 0;

    // Uniform block at firstBinding
    // (Would need to allocate and upload a buffer with the material params)

    // Textures at firstBinding + slot
    for (uint32_t slot = 0; slot < 6; ++slot) {
        if (mat->textures[slot]) {
            KxWriteDescriptorSet write{};
            write.sType = KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSet;
            write.dstBinding = firstBinding + 1 + slot;
            write.dstArrayElement = 0;
            write.descriptorCount = 1;
            write.descriptorType = KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            KxDescriptorImageInfo img_info{};
            img_info.sampler = mat->samplers[slot];
            img_info.imageView = mat->textures[slot];
            img_info.imageLayout = KX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            write.pImageInfo = &img_info;
            writes[write_count++] = write;
        }
    }

    if (write_count > 0) {
        sw_dev->update_descriptor_sets(write_count, writes, 0, nullptr);
    }

    return KX_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif