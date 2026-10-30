#include "Device.h"
#include "SoftwareDevice.h"

namespace kx {

/* Shader Module */

KxResult Device::dispatch_create_shader_module(KxDevice device,
                                               const KxShaderModuleCreateInfo* info,
                                               const KxAllocationCallbacks* alloc,
                                               KxShaderModule* module) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_shader_module(info, alloc, module)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_shader_module(KxDevice device, KxShaderModule module,
                                            const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_shader_module(module, alloc);
}

/* Pipeline Layout */

KxResult Device::dispatch_create_pipeline_layout(KxDevice device,
                                                 const KxPipelineLayoutCreateInfo* info,
                                                 const KxAllocationCallbacks* alloc,
                                                 KxPipelineLayout* layout) {
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_pipeline_layout(info, alloc, layout)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_pipeline_layout(KxDevice device, KxPipelineLayout layout,
                                              const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_pipeline_layout(layout, alloc);
}

/* Pipeline */

KxResult Device::dispatch_create_pipeline(KxDevice device,
                                          KxPipelineCache pipelineCache,
                                          const KxPipelineCreateInfo* info,
                                          const KxAllocationCallbacks* alloc,
                                          KxPipeline* pipeline) {
    (void)pipelineCache;
    auto* dev = find_device_locked(device);
    return dev ? dev->driver_device()->create_pipeline(info, alloc, pipeline)
               : KX_ERROR_DEVICE_LOST;
}

void Device::dispatch_destroy_pipeline(KxDevice device, KxPipeline pipeline,
                                       const KxAllocationCallbacks* alloc) {
    auto* dev = find_device_locked(device);
    if (dev) dev->driver_device()->destroy_pipeline(pipeline, alloc);
}

} /* namespace kx */