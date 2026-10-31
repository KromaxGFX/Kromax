/*
 * api_shader.cpp
 *
 * Shader module, shader environment, material and texture C API implementation.
 */

#include "api.h"
#include "api_internal.h"

#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareShaderModule.h"
#include "SoftwareDescriptorSet.h"
#include "SoftwareImage.h"
#include "SoftwareSampler.h"

#include <cstring>
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Shader Module from Source
 * ======================================================================= */

KxResult KX_CALL kxCreateShaderModuleFromSource(
    KxDevice device,
    const KxShaderSourceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule) {
    if (!pShaderModule) return KX_ERROR_INVALID_ARGUMENT;
    *pShaderModule = nullptr;
    if (!pCreateInfo) return KX_ERROR_INVALID_ARGUMENT;
    if (pCreateInfo->sType != KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_shader_module)
        return KX_ERROR_FEATURE_NOT_PRESENT;

    // Convert to legacy format for the software driver
    KxShaderModuleCreateInfo legacy_info{};
    legacy_info.sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    legacy_info.format = KX_SHADER_FORMAT_SOURCE_EXT;
    legacy_info.codeSize = pCreateInfo->sourceSize ? pCreateInfo->sourceSize
                                                    : static_cast<uint32_t>(std::strlen(pCreateInfo->pSource));
    legacy_info.pCode = pCreateInfo->pSource;

    return dev->dispatch()->create_shader_module(device, &legacy_info, pAllocator, pShaderModule);
}

KxResult KX_CALL kxCompileShader(
    const KxShaderSourceCreateInfo* pCreateInfo,
    KxShaderCompilationInfo* pCompilationInfo) {
    if (!pCreateInfo || !pCompilationInfo) return KX_ERROR_INVALID_ARGUMENT;

    // Standalone compilation without device - use the compiler directly
    // For now, return success with no diagnostics
    pCompilationInfo->sType = KX_STRUCTURE_TYPE_SHADER_COMPILATION_INFO;
    pCompilationInfo->diagnosticCount = 0;
    pCompilationInfo->pDiagnostics = nullptr;
    pCompilationInfo->irSize = 0;
    pCompilationInfo->pIr = nullptr;
    return KX_SUCCESS;
}

KxResult KX_CALL kxGetShaderModuleInfo(
    KxDevice device,
    KxShaderModule shaderModule,
    KxShaderCompilationInfo* pCompilationInfo) {
    if (!pCompilationInfo) return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* module = sw_dev->find_shader_module(shaderModule);
    if (!module) return KX_ERROR_INVALID_ARGUMENT;

    pCompilationInfo->sType = KX_STRUCTURE_TYPE_SHADER_COMPILATION_INFO;
    pCompilationInfo->diagnosticCount = 0;
    pCompilationInfo->pDiagnostics = nullptr;
    pCompilationInfo->irSize = 0;
    pCompilationInfo->pIr = nullptr;

    if (module->has_ir() && module->program) {
        // Could dump IR here if needed
    }
    return KX_SUCCESS;
}

const char* KX_CALL kxGetShaderLanguageName(KxShaderLanguage language) {
    switch (language) {
        case KX_SHADER_LANGUAGE_GLSL: return "GLSL";
        case KX_SHADER_LANGUAGE_HLSL: return "HLSL";
        case KX_SHADER_LANGUAGE_WGSL: return "WGSL";
        case KX_SHADER_LANGUAGE_KXSL: return "KXSL";
        default: return "Unknown";
    }
}

KxShaderLanguage KX_CALL kxDetectShaderLanguage(
    const char* pSource,
    size_t sourceSize) {
    if (!pSource || sourceSize == 0) return KX_SHADER_LANGUAGE_UNDEFINED;

    std::string source(pSource, sourceSize);

    if (source.find("#version") != std::string::npos ||
        source.find("layout(") != std::string::npos ||
        source.find("in vec3") != std::string::npos ||
        source.find("out vec3") != std::string::npos) {
        return KX_SHADER_LANGUAGE_GLSL;
    }
    if (source.find("cbuffer") != std::string::npos ||
        source.find("StructuredBuffer") != std::string::npos ||
        source.find("RWStructuredBuffer") != std::string::npos ||
        source.find("Texture2D") != std::string::npos ||
        source.find("SamplerState") != std::string::npos) {
        return KX_SHADER_LANGUAGE_HLSL;
    }
    if (source.find("var<") != std::string::npos ||
        source.find("@group") != std::string::npos ||
        source.find("@binding") != std::string::npos ||
        source.find("fn ") != std::string::npos) {
        return KX_SHADER_LANGUAGE_WGSL;
    }
    return KX_SHADER_LANGUAGE_GLSL;
}

/* =======================================================================
 * Shader Environment
 * ======================================================================= */

KxResult KX_CALL kxCreateShaderEnvironment(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxShaderEnvironment* pEnvironment) {
    (void)pAllocator;
    if (!pEnvironment) return KX_ERROR_INVALID_ARGUMENT;
    *pEnvironment = nullptr;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    // For the software driver, we use a simple opaque handle
    // The actual environment data is stored in the device
    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    // Create a simple handle
    struct EnvHandle { uint64_t id = 0; };
    static uint64_t next_env_id = 1;
    auto handle = reinterpret_cast<KxShaderEnvironment>(new EnvHandle{next_env_id++});
    *pEnvironment = handle;
    return KX_SUCCESS;
}

void KX_CALL kxDestroyShaderEnvironment(
    KxDevice device,
    KxShaderEnvironment environment,
    const KxAllocationCallbacks* pAllocator) {
    (void)device;
    (void)pAllocator;
    if (environment) {
        delete reinterpret_cast<uint64_t*>(environment);
    }
}

KxResult KX_CALL kxShaderEnvironmentSetUniform(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    const void* pData,
    size_t size) {
    (void)device;
    (void)environment;
    (void)set;
    (void)binding;
    (void)pData;
    (void)size;
    return KX_SUCCESS;
}

KxResult KX_CALL kxShaderEnvironmentSetTexture(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    KxImageView imageView,
    KxSampler sampler) {
    (void)device;
    (void)environment;
    (void)set;
    (void)binding;
    (void)imageView;
    (void)sampler;
    return KX_SUCCESS;
}

void KX_CALL kxCmdBindShaderEnvironment(
    KxCommandBuffer commandBuffer,
    KxShaderEnvironment environment) {
    // Record the environment binding in the command buffer
    // The software driver will merge it with descriptor sets at draw time
    (void)commandBuffer;
    (void)environment;
}

/* =======================================================================
 * Materials
 * ======================================================================= */

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

    // For now, create a simple material handle
    struct MaterialHandle {
        KxShaderModule shader_module = nullptr;
        float base_color[4] = {1, 1, 1, 1};
        float metallic = 0.0f;
        float roughness = 1.0f;
        float emissive[3] = {0, 0, 0};
        float normal_scale = 1.0f;
        float occlusion_strength = 1.0f;
        float alpha_cutoff = 0.5f;
        KxImageView textures[6] = {}; // KX_MATERIAL_TEXTURE_MAX_ENUM
        KxSampler samplers[6] = {};
    };

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
        delete reinterpret_cast<void*>(material);
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

    struct MaterialHandle {
        KxImageView textures[6];
        KxSampler samplers[6];
    };

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

    struct MaterialHandle {
        KxImageView textures[6];
        KxSampler samplers[6];
    };

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

KxResult KX_CALL kxMaterialGetParameter(
    KxDevice device,
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

/* =======================================================================
 * Texture Helpers
 * ======================================================================= */

KxResult KX_CALL kxGetImageInfo(
    KxDevice device,
    KxImage image,
    KxTextureInfo* pInfo) {
    if (!pInfo) return KX_ERROR_INVALID_ARGUMENT;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    pInfo->sType = KX_STRUCTURE_TYPE_MAX_ENUM; // Not defined yet
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

KxResult KX_CALL kxCreateTexture2D(
    KxDevice device,
    uint32_t width,
    uint32_t height,
    KxFormat format,
    const void* pPixels,
    uint32_t mipLevels,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage) {
    if (!pImage) return KX_ERROR_INVALID_ARGUMENT;
    *pImage = nullptr;

    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_image)
        return KX_ERROR_FEATURE_NOT_PRESENT;

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

    KxResult result = dev->dispatch()->create_image(device, &create_info, pAllocator, pImage);
    if (result != KX_SUCCESS) return result;

    if (pPixels && dev->dispatch()->bind_image_memory && dev->dispatch()->allocate_memory) {
        // Allocate and bind memory, then copy pixels
        // This is simplified - a full implementation would use staging buffers
    }

    return KX_SUCCESS;
}

KxResult KX_CALL kxGenerateMipmaps(
    KxDevice device,
    KxImage image) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;

    auto* sw_dev = dynamic_cast<kx::SoftwareDevice*>(dev->driver_device());
    if (!sw_dev) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* img = sw_dev->find_image(image);
    if (!img) return KX_ERROR_INVALID_ARGUMENT;

    if (img->mip_levels() <= 1) return KX_SUCCESS;

    // Generate mipmaps using box filter
    uint32_t w = img->width();
    uint32_t h = img->height();
    uint32_t pixel_size = 4; // Assume RGBA8 for now

    for (uint32_t level = 1; level < img->mip_levels(); ++level) {
        uint32_t src_w = std::max<uint32_t>(1, w >> (level - 1));
        uint32_t src_h = std::max<uint32_t>(1, h >> (level - 1));
        uint32_t dst_w = std::max<uint32_t>(1, w >> level);
        uint32_t dst_h = std::max<uint32_t>(1, h >> level);

        // Simple box filter downsample
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

KxSamplerState KX_CALL kxDefaultSamplerState(void) {
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

#ifdef __cplusplus
} /* extern "C" */
#endif