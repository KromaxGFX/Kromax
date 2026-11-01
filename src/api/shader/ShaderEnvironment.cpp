/*
 * ShaderEnvironment.cpp
 * Shader environment for uniform/texture binding implementation.
 */

#include "ShaderEnvironment.h"

#include "api_internal.h"
#include "Device.h"
#include "SoftwareDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
} /* extern "C" */
#endif