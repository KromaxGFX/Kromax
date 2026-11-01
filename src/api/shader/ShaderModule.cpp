/*
 * ShaderModule.cpp
 * Shader module creation, compilation, and reflection implementation.
 */

#include "ShaderModule.h"

#include "api_internal.h"
#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareShaderModule.h"

#include <cstring>
#include <algorithm>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} /* extern "C" */
#endif