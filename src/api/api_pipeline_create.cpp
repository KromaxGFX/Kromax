/*
 * api_pipeline_create.cpp
 *
 * Pipeline, Shader module, PipelineLayout creation C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Shader modules
 * ======================================================================= */

KxResult KX_CALL kxCreateShaderModule(
    KxDevice device,
    const KxShaderModuleCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule) {
    if (!pShaderModule) return KX_ERROR_INVALID_ARGUMENT;
    *pShaderModule = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_shader_module)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_shader_module(device, pCreateInfo, pAllocator,
                                                  pShaderModule);
}

void KX_CALL kxDestroyShaderModule(
    KxDevice device,
    KxShaderModule shaderModule,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_shader_module) return;
    dev->dispatch()->destroy_shader_module(device, shaderModule, pAllocator);
}

/* =======================================================================
 * Pipelines
 * ======================================================================= */

KxResult KX_CALL kxCreatePipelineLayout(
    KxDevice device,
    const KxPipelineLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipelineLayout* pPipelineLayout) {
    if (!pPipelineLayout) return KX_ERROR_INVALID_ARGUMENT;
    *pPipelineLayout = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_pipeline_layout)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_pipeline_layout(device, pCreateInfo, pAllocator,
                                                    pPipelineLayout);
}

void KX_CALL kxDestroyPipelineLayout(
    KxDevice device,
    KxPipelineLayout pipelineLayout,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_pipeline_layout) return;
    dev->dispatch()->destroy_pipeline_layout(device, pipelineLayout, pAllocator);
}

KxResult KX_CALL kxCreatePipeline(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const KxPipelineCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipeline* pPipeline) {
    if (!pPipeline) return KX_ERROR_INVALID_ARGUMENT;
    *pPipeline = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_pipeline)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_pipeline(device, pipelineCache, pCreateInfo, pAllocator,
                                            pPipeline);
}

void KX_CALL kxDestroyPipeline(
    KxDevice device,
    KxPipeline pipeline,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_pipeline) return;
    dev->dispatch()->destroy_pipeline(device, pipeline, pAllocator);
}

#ifdef __cplusplus
} /* extern "C" */
#endif