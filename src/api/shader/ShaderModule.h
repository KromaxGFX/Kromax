#ifndef KROMAX_SRC_API_SHADER_SHADERMODULE_H
#define KROMAX_SRC_API_SHADER_SHADERMODULE_H

/*
 * ShaderModule.h
 * Shader module creation, compilation, and reflection API.
 */

#include "kromax.h"
#include "kx_core.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Shader module from source code
KxResult KX_CALL kxCreateShaderModuleFromSource(
    KxDevice device,
    const KxShaderSourceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule);

// Standalone shader compilation (no device required)
KxResult KX_CALL kxCompileShader(
    const KxShaderSourceCreateInfo* pCreateInfo,
    KxShaderCompilationInfo* pCompilationInfo);

// Query shader module compilation info/IR
KxResult KX_CALL kxGetShaderModuleInfo(
    KxDevice device,
    KxShaderModule shaderModule,
    KxShaderCompilationInfo* pCompilationInfo);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // KROMAX_SRC_API_SHADER_SHADERMODULE_H