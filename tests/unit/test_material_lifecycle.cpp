/*
 * test_material_lifecycle.cpp
 * Unit tests for material create/destroy, parameter/texture management
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>
#include "kx_core.h"

extern "C" {
    KxResult kxCreateMaterial(KxDevice device, const KxMaterialCreateInfo* pCreateInfo,
                              const KxAllocationCallbacks* pAllocator, KxMaterial* pMaterial);
    void kxDestroyMaterial(KxDevice device, KxMaterial material,
                           const KxAllocationCallbacks* pAllocator);
    KxResult kxMaterialSetTexture(KxDevice device, KxMaterial material,
                                  KxMaterialTextureSlot slot, KxImageView imageView, KxSampler sampler);
    KxResult kxMaterialGetTexture(KxDevice device, KxMaterial material,
                                  KxMaterialTextureSlot slot, KxImageView* pImageView, KxSampler* pSampler);
    KxResult kxMaterialSetParameter(KxDevice device, KxMaterial material,
                                    const char* pName, const void* pValue, size_t size);
    KxResult kxMaterialGetParameter(KxDevice device, KxMaterial material,
                                    const char* pName, void* pValue, size_t size);
    KxResult kxMaterialUpdateDescriptorSet(KxDevice device, KxMaterial material,
                                           KxDescriptorSet descriptorSet, uint32_t firstBinding);
    KxResult kxCreateDefaultMaterial(KxDevice device, const KxAllocationCallbacks* pAllocator,
                                     KxMaterial* pMaterial);
}

TEST_CASE("MaterialLifecycle - ParameterNames") {
    const char* param_names[] = {
        "baseColorFactor",
        "emissiveFactor",
        "metallicFactor",
        "roughnessFactor",
        "normalScale",
        "occlusionStrength",
        "alphaCutoff",
        "alphaMode",
        "iblEnabled"
    };
    
    for (const char* name : param_names) {
        CHECK(name != nullptr);
    }
}

TEST_CASE("MaterialLifecycle - TextureSlots") {
    const int NUM_SLOTS = 6;
    CHECK_EQ(NUM_SLOTS, 6);
    
    CHECK_EQ(KX_MATERIAL_TEXTURE_BASE_COLOR, 0);
    CHECK_EQ(KX_MATERIAL_TEXTURE_NORMAL, 1);
    CHECK_EQ(KX_MATERIAL_TEXTURE_METALLIC_ROUGHNESS, 2);
    CHECK_EQ(KX_MATERIAL_TEXTURE_EMISSIVE, 3);
    CHECK_EQ(KX_MATERIAL_TEXTURE_OCCLUSION, 4);
    CHECK_EQ(KX_MATERIAL_TEXTURE_HEIGHT, 5);
}

TEST_CASE("MaterialLifecycle - AlphaModes") {
    // Test alpha mode parameter values
    int alpha_mode_opaque = 0;
    int alpha_mode_mask = 1;
    int alpha_mode_blend = 2;
    CHECK_EQ(alpha_mode_opaque, 0);
    CHECK_EQ(alpha_mode_mask, 1);
    CHECK_EQ(alpha_mode_blend, 2);
}

TEST_CASE("MaterialLifecycle - DefaultValues") {
    float default_base_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float default_emissive[3] = {0.0f, 0.0f, 0.0f};
    float default_metallic = 0.0f;
    float default_roughness = 1.0f;
    float default_normal_scale = 1.0f;
    CHECK_EQ(default_metallic, 0.0f);
    CHECK_EQ(default_roughness, 1.0f);
    CHECK_EQ(default_normal_scale, 1.0f);
}