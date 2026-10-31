/*
 * DeviceDispatchShader.cpp
 *
 * Shader, shader environment, material and texture dispatch thunks.
 */

#include "Device.h"
#include "SoftwareDevice.h"
#include <algorithm>

namespace kx {

/* Shader from source */
KxResult Device::dispatch_create_shader_module_from_source(KxDevice device,
                                                           const KxShaderSourceCreateInfo* info,
                                                           const KxAllocationCallbacks* alloc,
                                                           KxShaderModule* module) {
    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;
    if (!dev->dispatch() || !dev->dispatch()->create_shader_module) return KX_ERROR_FEATURE_NOT_PRESENT;

    // Convert to legacy format
    KxShaderModuleCreateInfo legacy_info{};
    legacy_info.sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    legacy_info.format = KX_SHADER_FORMAT_SOURCE_EXT;
    legacy_info.codeSize = info->sourceSize ? info->sourceSize
                                            : static_cast<uint32_t>(std::strlen(info->pSource));
    legacy_info.pCode = info->pSource;

    return dev->dispatch()->create_shader_module(device, &legacy_info, alloc, module);
}

/* Shader Environment */
KxResult Device::dispatch_create_shader_environment(KxDevice device,
                                                     const KxAllocationCallbacks* alloc,
                                                     KxShaderEnvironment* env) {
    (void)alloc;
    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;

    // Simple handle allocation
    struct EnvHandle { uint64_t id = 0; };
    static uint64_t next_env_id = 1;
    auto handle = reinterpret_cast<KxShaderEnvironment>(new EnvHandle{next_env_id++});
    *env = handle;
    return KX_SUCCESS;
}

void Device::dispatch_destroy_shader_environment(KxDevice device,
                                                  KxShaderEnvironment env,
                                                  const KxAllocationCallbacks* alloc) {
    (void)device;
    (void)alloc;
    if (env) {
        delete reinterpret_cast<uint64_t*>(env);
    }
}

KxResult Device::dispatch_shader_environment_set_uniform(KxDevice device,
                                                          KxShaderEnvironment env,
                                                          uint32_t set,
                                                          uint32_t binding,
                                                          const void* pData,
                                                          size_t size) {
    (void)device;
    (void)env;
    (void)set;
    (void)binding;
    (void)pData;
    (void)size;
    return KX_SUCCESS;
}

KxResult Device::dispatch_shader_environment_set_texture(KxDevice device,
                                                          KxShaderEnvironment env,
                                                          uint32_t set,
                                                          uint32_t binding,
                                                          KxImageView imageView,
                                                          KxSampler sampler) {
    (void)device;
    (void)env;
    (void)set;
    (void)binding;
    (void)imageView;
    (void)sampler;
    return KX_SUCCESS;
}

void Device::dispatch_cmd_bind_shader_environment(KxCommandBuffer cb,
                                                   KxShaderEnvironment env) {
    (void)cb;
    (void)env;
}

/* Materials */
KxResult Device::dispatch_create_material(KxDevice device,
                                           const KxMaterialCreateInfo* info,
                                           const KxAllocationCallbacks* alloc,
                                           KxMaterial* material) {
    (void)alloc;
    if (!material) return KX_ERROR_INVALID_ARGUMENT;
    *material = nullptr;

    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;

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

    auto handle = reinterpret_cast<KxMaterial>(new MaterialHandle);
    if (!handle) return KX_ERROR_OUT_OF_HOST_MEMORY;

    MaterialHandle* mat = reinterpret_cast<MaterialHandle*>(handle);
    mat->shader_module = info->shaderModule;
    if (info->pTextures && info->textureCount > 0) {
        for (uint32_t i = 0; i < info->textureCount && i < 6; ++i) {
            const auto& tb = info->pTextures[i];
            if (tb.slot < 6) {
                mat->textures[tb.slot] = tb.imageView;
                mat->samplers[tb.slot] = tb.sampler;
            }
        }
    }
    if (info->baseColor) {
        mat->base_color[0] = info->baseColor[0];
        mat->base_color[1] = info->baseColor[1];
        mat->base_color[2] = info->baseColor[2];
        mat->base_color[3] = info->baseColor[3];
    }
    mat->metallic = info->metallic;
    mat->roughness = info->roughness;
    mat->emissive[0] = info->emissive[0];
    mat->emissive[1] = info->emissive[1];
    mat->emissive[2] = info->emissive[2];
    mat->normal_scale = info->normalScale;
    mat->occlusion_strength = info->occlusionStrength;
    mat->alpha_cutoff = info->alphaCutoff;

    *material = handle;
    return KX_SUCCESS;
}

void Device::dispatch_destroy_material(KxDevice device,
                                        KxMaterial material,
                                        const KxAllocationCallbacks* alloc) {
    (void)device;
    (void)alloc;
    if (material) {
        delete reinterpret_cast<void*>(material);
    }
}

KxResult Device::dispatch_material_set_texture(KxDevice device,
                                                KxMaterial material,
                                                KxMaterialTextureSlot slot,
                                                KxImageView imageView,
                                                KxSampler sampler) {
    (void)device;
    if (!material || slot >= 6) return KX_ERROR_INVALID_ARGUMENT;

    struct MaterialHandle {
        KxImageView textures[6];
        KxSampler samplers[6];
    };

    auto* mat = reinterpret_cast<MaterialHandle*>(material);
    mat->textures[slot] = imageView;
    mat->samplers[slot] = sampler;
    return KX_SUCCESS;
}

KxResult Device::dispatch_material_get_texture(KxDevice device,
                                                KxMaterial material,
                                                KxMaterialTextureSlot slot,
                                                KxImageView* pImageView,
                                                KxSampler* pSampler) {
    (void)device;
    if (!material || slot >= 6) return KX_ERROR_INVALID_ARGUMENT;

    struct MaterialHandle {
        KxImageView textures[6];
        KxSampler samplers[6];
    };

    auto* mat = reinterpret_cast<MaterialHandle*>(material);
    if (pImageView) *pImageView = mat->textures[slot];
    if (pSampler) *pSampler = mat->samplers[slot];
    return KX_SUCCESS;
}

KxResult Device::dispatch_material_set_parameter(KxDevice device,
                                                  KxMaterial material,
                                                  const char* pName,
                                                  const void* pValue,
                                                  size_t size) {
    (void)device;
    if (!material || !pName || !pValue) return KX_ERROR_INVALID_ARGUMENT;

    struct MaterialHandle {
        float base_color[4];
        float metallic;
        float roughness;
        float emissive[3];
        float normal_scale;
        float occlusion_strength;
        float alpha_cutoff;
    };

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

KxResult Device::dispatch_material_get_parameter(KxDevice device,
                                                  KxMaterial material,
                                                  const char* pName,
                                                  void* pValue,
                                                  size_t size) {
    (void)device;
    if (!material || !pName || !pValue) return KX_ERROR_INVALID_ARGUMENT;

    struct MaterialHandle {
        float base_color[4];
        float metallic;
        float roughness;
        float emissive[3];
        float normal_scale;
        float occlusion_strength;
        float alpha_cutoff;
    };

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

KxResult Device::dispatch_material_update_descriptor_set(KxDevice device,
                                                          KxMaterial material,
                                                          KxDescriptorSet descriptorSet,
                                                          uint32_t firstBinding) {
    if (!material) return KX_ERROR_INVALID_ARGUMENT;

    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;

    struct MaterialHandle {
        KxShaderModule shader_module;
        float base_color[4];
        float metallic;
        float roughness;
        float emissive[3];
        float normal_scale;
        float occlusion_strength;
        float alpha_cutoff;
        KxImageView textures[6];
        KxSampler samplers[6];
    };

    auto* mat = reinterpret_cast<MaterialHandle*>(material);

    KxWriteDescriptorSet writes[7];
    uint32_t write_count = 0;

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
        dev->driver_device()->update_descriptor_sets(write_count, writes, 0, nullptr);
    }

    return KX_SUCCESS;
}

/* Texture Helpers */
KxResult Device::dispatch_get_image_info(KxDevice device,
                                          KxImage image,
                                          KxTextureInfo* pInfo) {
    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;

    auto* sw_dev = dynamic_cast<SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    pInfo->sType = KX_STRUCTURE_TYPE_MAX_ENUM;
    pInfo->width = img->width();
    pInfo->height = img->height();
    pInfo->depth = img->depth();
    pInfo->mipLevels = img->mip_levels();
    pInfo->arrayLayers = img->array_layers();
    pInfo->format = img->format();
    pInfo->layout = img->layout();
    pInfo->rowPitch = static_cast<uint32_t>(img->row_pitch());
    pInfo->slicePitch = static_cast<uint32_t>(img->slice_pitch());
    pInfo->dataSize = img->data_size();

    return KX_SUCCESS;
}

KxResult Device::dispatch_create_texture_2d(KxDevice device,
                                             uint32_t width,
                                             uint32_t height,
                                             KxFormat format,
                                             const void* pPixels,
                                             uint32_t mipLevels,
                                             const KxAllocationCallbacks* alloc,
                                             KxImage* pImage) {
    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;
    if (!dev->dispatch() || !dev->dispatch()->create_image) return KX_ERROR_FEATURE_NOT_PRESENT;

    KxImageCreateInfo create_info{};
    create_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = KX_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.width = width;
    create_info.height = height;
    create_info.depth = 1;
    create_info.mipLevels = mipLevels > 0 ? mipLevels : 1;
    create_info.arrayLayers = 1;
    create_info.tiling = KX_IMAGE_TILING_LINEAR;
    create_info.initialLayout = KX_IMAGE_LAYOUT_PREINITIALIZED;
    create_info.usage = KX_IMAGE_USAGE_SAMPLED_BIT | KX_IMAGE_USAGE_TRANSFER_DST_BIT;

    return dev->dispatch()->create_image(device, &create_info, alloc, pImage);
}

KxResult Device::dispatch_generate_mipmaps(KxDevice device, KxImage image) {
    auto* dev = find_device_locked(device);
    if (!dev) return KX_ERROR_DEVICE_LOST;

    auto* sw_dev = dynamic_cast<SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    if (img->mip_levels() <= 1) return KX_SUCCESS;

    uint32_t w = img->width();
    uint32_t h = img->height();
    uint32_t pixel_size = 4; // Assume RGBA8

    for (uint32_t level = 1; level < img->mip_levels(); ++level) {
        uint32_t src_w = std::max<uint32_t>(1, w >> (level - 1));
        uint32_t src_h = std::max<uint32_t>(1, h >> (level - 1));
        uint32_t dst_w = std::max<uint32_t>(1, w >> level);
        uint32_t dst_h = std::max<uint32_t>(1, h >> level);

        uint8_t* dst = img->data() + (img->slice_pitch() * level);
        const uint8_t* src = img->data() + (img->slice_pitch() * (level - 1));

        for (uint32_t y = 0; y < dst_h; ++y) {
            for (uint32_t x = 0; x < dst_w; ++x) {
                uint32_t sx0 = x * 2;
                uint32_t sy0 = y * 2;
                uint32_t sx1 = std::min(sx0 + 1, src_w - 1);
                uint32_t sy1 = std::min(sy0 + 1, src_h - 1);

                float r = 0, g = 0, b = 0, a = 0;
                int count = 0;
                for (uint32_t sy = sy0; sy <= sy1; ++sy) {
                    for (uint32_t sx = sx0; sx <= sx1; ++sx) {
                        const uint8_t* p = src + sy * src_w * pixel_size + sx * pixel_size;
                        r += p[0] / 255.0f;
                        g += p[1] / 255.0f;
                        b += p[2] / 255.0f;
                        a += p[3] / 255.0f;
                        ++count;
                    }
                }
                uint8_t* dp = dst + y * dst_w * pixel_size + x * pixel_size;
                dp[0] = static_cast<uint8_t>(std::clamp(r / count, 0.0f, 1.0f) * 255.0f);
                dp[1] = static_cast<uint8_t>(std::clamp(g / count, 0.0f, 1.0f) * 255.0f);
                dp[2] = static_cast<uint8_t>(std::clamp(b / count, 0.0f, 1.0f) * 255.0f);
                dp[3] = static_cast<uint8_t>(std::clamp(a / count, 0.0f, 1.0f) * 255.0f);
            }
        }
    }

    return KX_SUCCESS;
}

KxSamplerState Device::dispatch_default_sampler_state(void) {
    KxSamplerState state{};
    state.magFilter = KX_FILTER_NEAREST;
    state.minFilter = KX_FILTER_NEAREST;
    state.addressModeU = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.addressModeV = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.addressModeW = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    state.mipLodBias = 0.0f;
    state.anisotropyEnable = KX_FALSE;
    state.maxAnisotropy = 1.0f;
    state.compareEnable = KX_FALSE;
    state.compareOp = KX_COMPARE_OP_ALWAYS;
    state.minLod = 0.0f;
    state.maxLod = 1000.0f;
    state.borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    state.unnormalizedCoordinates = KX_FALSE;
    state.maxLodLevel = 0;
    state.srgbDecode = KX_FALSE;
    return state;
}

KxResult Device::dispatch_sample_texture(const KxBoundTexture* pTexture,
                                          float u, float v,
                                          float outColor[4]) {
    return kxSampleTexture(pTexture, u, v, outColor);
}

KxResult Device::dispatch_sample_texture_grad(const KxBoundTexture* pTexture,
                                               float u, float v,
                                               float dudx, float dvdx,
                                               float dudy, float dvdy,
                                               float outColor[4]) {
    return kxSampleTextureGrad(pTexture, u, v, dudx, dvdx, dudy, dvdy, outColor);
}

KxResult Device::dispatch_sample_texture_lod(const KxBoundTexture* pTexture,
                                              float u, float v,
                                              float lod,
                                              float outColor[4]) {
    return kxSampleTextureLod(pTexture, u, v, lod, outColor);
}

} /* namespace kx */