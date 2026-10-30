#ifndef KROMAX_SRC_CORE_DISPATCH_H
#define KROMAX_SRC_CORE_DISPATCH_H

/*
 * Dispatch.h
 *
 * Dispatch-table infrastructure. Each internal object type carries a
 * pointer to a table of virtual function pointers; the public C API layer
 * looks up the table from the handle and invokes the entry, which keeps
 * the ABI stable and allows layers (validation, etc.) to decorate calls.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

/* Forward declarations of internal object types */
class Instance;
class PhysicalDevice;
class Device;
class Queue;
class DeviceMemory;
class Buffer;
class Image;
class ImageView;
class ShaderModule;
class PipelineLayout;
class Pipeline;
class CommandPool;
class CommandBuffer;
class Fence;
class Semaphore;
class RenderPass;
class Framebuffer;

/* -----------------------------------------------------------------------
 * Instance dispatch table
 * --------------------------------------------------------------------- */
struct InstanceDispatch {
    KxResult (KX_CALL* create_device)(
        KxPhysicalDevice, const KxDeviceCreateInfo*,
        const KxAllocationCallbacks*, KxDevice*);
    void (KX_CALL* destroy_device)(
        KxDevice, const KxAllocationCallbacks*);
    KxResult (KX_CALL* enumerate_physical_devices)(
        KxInstance, uint32_t*, KxPhysicalDevice*);
    KxResult (KX_CALL* get_proc_addr)(
        KxInstance, const char*, KxPfnVoidFunction*);
};

/* -----------------------------------------------------------------------
 * Device dispatch table
 * --------------------------------------------------------------------- */
struct DeviceDispatch {
    KxResult (KX_CALL* allocate_memory)(
        KxDevice, const KxMemoryAllocateInfo*,
        const KxAllocationCallbacks*, KxDeviceMemory*);
    void (KX_CALL* free_memory)(
        KxDevice, KxDeviceMemory, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_buffer)(
        KxDevice, const KxBufferCreateInfo*,
        const KxAllocationCallbacks*, KxBuffer*);
    void (KX_CALL* destroy_buffer)(
        KxDevice, KxBuffer, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_image)(
        KxDevice, const KxImageCreateInfo*,
        const KxAllocationCallbacks*, KxImage*);
    void (KX_CALL* destroy_image)(
        KxDevice, KxImage, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_image_view)(
        KxDevice, const KxImageViewCreateInfo*,
        const KxAllocationCallbacks*, KxImageView*);
    void (KX_CALL* destroy_image_view)(
        KxDevice, KxImageView, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_render_pass)(
        KxDevice, const KxRenderPassCreateInfo*,
        const KxAllocationCallbacks*, KxRenderPass*);
    void (KX_CALL* destroy_render_pass)(
        KxDevice, KxRenderPass, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_framebuffer)(
        KxDevice, const KxFramebufferCreateInfo*,
        const KxAllocationCallbacks*, KxFramebuffer*);
    void (KX_CALL* destroy_framebuffer)(
        KxDevice, KxFramebuffer, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_command_pool)(
        KxDevice, const KxCommandPoolCreateInfo*,
        const KxAllocationCallbacks*, KxCommandPool*);
    void (KX_CALL* destroy_command_pool)(
        KxDevice, KxCommandPool, const KxAllocationCallbacks*);
    KxResult (KX_CALL* allocate_command_buffers)(
        KxDevice, const KxCommandBufferAllocateInfo*,
        KxCommandBuffer*);
    void (KX_CALL* free_command_buffers)(
        KxDevice, KxCommandPool, uint32_t, const KxCommandBuffer*);
    KxResult (KX_CALL* begin_command_buffer)(
        KxCommandBuffer, const KxCommandBufferBeginInfo*);
    KxResult (KX_CALL* end_command_buffer)(KxCommandBuffer);
    KxResult (KX_CALL* queue_submit)(
        KxQueue, uint32_t, const KxSubmitInfo*, KxFence);
    KxResult (KX_CALL* queue_wait_idle)(KxQueue);
    KxResult (KX_CALL* device_wait_idle)(KxDevice);
    KxResult (KX_CALL* bind_buffer_memory)(
        KxDevice, KxBuffer, KxDeviceMemory, uint64_t);
    KxResult (KX_CALL* bind_image_memory)(
        KxDevice, KxImage, KxDeviceMemory, uint64_t);
    KxResult (KX_CALL* map_memory)(
        KxDevice, KxDeviceMemory, uint64_t, uint64_t, KxMemoryMapFlags, void**);
    void (KX_CALL* unmap_memory)(
        KxDevice, KxDeviceMemory);
    KxResult (KX_CALL* create_shader_module)(
        KxDevice, const KxShaderModuleCreateInfo*,
        const KxAllocationCallbacks*, KxShaderModule*);
    void (KX_CALL* destroy_shader_module)(
        KxDevice, KxShaderModule, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_pipeline_layout)(
        KxDevice, const KxPipelineLayoutCreateInfo*,
        const KxAllocationCallbacks*, KxPipelineLayout*);
    void (KX_CALL* destroy_pipeline_layout)(
        KxDevice, KxPipelineLayout, const KxAllocationCallbacks*);
    KxResult (KX_CALL* create_pipeline)(
        KxDevice, KxPipelineCache, const KxPipelineCreateInfo*,
        const KxAllocationCallbacks*, KxPipeline*);
    void (KX_CALL* destroy_pipeline)(
        KxDevice, KxPipeline, const KxAllocationCallbacks*);

    // Descriptor set layout
    KxResult (KX_CALL* create_descriptor_set_layout)(
        KxDevice, const KxDescriptorSetLayoutCreateInfo*,
        const KxAllocationCallbacks*, KxDescriptorSetLayout*);
    void (KX_CALL* destroy_descriptor_set_layout)(
        KxDevice, KxDescriptorSetLayout, const KxAllocationCallbacks*);

    // Descriptor pool
    KxResult (KX_CALL* create_descriptor_pool)(
        KxDevice, const KxDescriptorPoolCreateInfo*,
        const KxAllocationCallbacks*, KxDescriptorPool*);
    void (KX_CALL* destroy_descriptor_pool)(
        KxDevice, KxDescriptorPool, const KxAllocationCallbacks*);
    KxResult (KX_CALL* reset_descriptor_pool)(
        KxDevice, KxDescriptorPool, KxDescriptorPoolResetFlags);

    // Descriptor set
    KxResult (KX_CALL* allocate_descriptor_sets)(
        KxDevice, const KxDescriptorSetAllocateInfo*, KxDescriptorSet*);
    KxResult (KX_CALL* free_descriptor_sets)(
        KxDevice, KxDescriptorPool, uint32_t, const KxDescriptorSet*);
    void (KX_CALL* update_descriptor_sets)(
        KxDevice, uint32_t, const KxWriteDescriptorSet*,
        uint32_t, const KxCopyDescriptorSet*);

    // Sampler
    KxResult (KX_CALL* create_sampler)(
        KxDevice, const KxSamplerCreateInfo*,
        const KxAllocationCallbacks*, KxSampler*);
    void (KX_CALL* destroy_sampler)(
        KxDevice, KxSampler, const KxAllocationCallbacks*);
};

/* -----------------------------------------------------------------------
 * Global dispatch table pointer (set up at instance creation)
 * --------------------------------------------------------------------- */
extern InstanceDispatch g_instance_dispatch;
extern DeviceDispatch g_device_dispatch;

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_DISPATCH_H */