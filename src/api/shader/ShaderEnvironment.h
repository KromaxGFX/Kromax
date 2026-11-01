#ifndef KROMAX_SRC_API_SHADER_SHADERENVIRONMENT_H
#define KROMAX_SRC_API_SHADER_SHADERENVIRONMENT_H

/*
 * ShaderEnvironment.h
 * Shader environment for uniform/texture binding.
 */

#include "kromax.h"
#include "kx_core.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Shader environment lifecycle
KxResult KX_CALL kxCreateShaderEnvironment(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxShaderEnvironment* pEnvironment);

void KX_CALL kxDestroyShaderEnvironment(
    KxDevice device,
    KxShaderEnvironment environment,
    const KxAllocationCallbacks* pAllocator);

// Set uniform data in environment
KxResult KX_CALL kxShaderEnvironmentSetUniform(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    const void* pData,
    size_t size);

// Set texture/sampler in environment
KxResult KX_CALL kxShaderEnvironmentSetTexture(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    KxImageView imageView,
    KxSampler sampler);

// Bind environment in command buffer
void KX_CALL kxCmdBindShaderEnvironment(
    KxCommandBuffer commandBuffer,
    KxShaderEnvironment environment);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // KROMAX_SRC_API_SHADER_SHADERENVIRONMENT_H