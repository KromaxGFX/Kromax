#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMATERIAL_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMATERIAL_H

/*
 * SoftwareMaterial.h
 *
 * Material system implementation with built-in PBR shader compilation and caching.
 */

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "SoftwareDevice.h"
#include "SoftwareShaderModule.h"
#include "kx_core.h"

namespace kx {

// Forward declarations
class SoftwareDevice;
class SoftwareShaderModule;

// Texture slot for material
struct MaterialTextureSlot {
    KxImageView image_view = nullptr;
    KxSampler sampler = nullptr;
};

// PBR Material parameters
struct PBRMaterialParams {
    float base_color_factor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float emissive_factor[3] = {0.0f, 0.0f, 0.0f};
    float metallic_factor = 0.0f;
    float roughness_factor = 1.0f;
    float normal_scale = 1.0f;
    float occlusion_strength = 1.0f;
    float alpha_cutoff = 0.5f;
    float alpha_mode = 0.0f;  // 0=OPAQUE, 1=MASK, 2=BLEND
    int ibl_enabled = 0;
};

// Shader cache for PBR shaders
class SoftwareMaterialCache {
public:
    explicit SoftwareMaterialCache(SoftwareDevice* device);
    ~SoftwareMaterialCache();

    SoftwareShaderModule* get_pbr_shader(KxShaderStage stage);
    void clear();

private:
    SoftwareDevice* device_ = nullptr;
    std::unordered_map<KxShaderStage, std::unique_ptr<SoftwareShaderModule>> pbr_shaders_;
};

// Software material instance
class SoftwareMaterial {
public:
    explicit SoftwareMaterial(SoftwareDevice* device, const KxMaterialCreateInfo* create_info = nullptr);
    ~SoftwareMaterial();

    void set_texture(KxMaterialTextureSlot slot, KxImageView image_view, KxSampler sampler);
    void get_texture(KxMaterialTextureSlot slot, KxImageView* image_view, KxSampler* sampler) const;

    void set_parameter(const char* name, const void* value, size_t size);
    void get_parameter(const char* name, void* value, size_t size) const;

    KxResult update_descriptor_set(KxDescriptorSet descriptor_set, uint32_t first_binding);

    // Public access to shaders
    SoftwareShaderModule* vertex_shader = nullptr;
    SoftwareShaderModule* fragment_shader = nullptr;

    // Material parameters
    PBRMaterialParams params;

    // Texture slots (6 standard slots)
    std::array<MaterialTextureSlot, 6> texture_slots{};

private:
    SoftwareDevice* device_ = nullptr;
    SoftwareMaterialCache cache_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMATERIAL_H */