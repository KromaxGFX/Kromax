#ifndef KROMAX_SRC_DRIVER_IDRIVERDEVICE_H
#define KROMAX_SRC_DRIVER_IDRIVERDEVICE_H

/*
 * IDriverDevice.h
 *
 * Abstract device interface implemented by each driver's device object.
 * The Kromax Device wraps this and forwards dispatch calls to it.
 */

#include "kx_core.h"

namespace kx {

class SoftwareBuffer;
class SoftwareImage;
class SoftwareImageView;
class SoftwareRenderPass;
class SoftwareFramebuffer;
class SoftwareCommandBuffer;
class SoftwareCommandPool;
class SoftwarePipeline;
class SoftwarePipelineLayout;
class SoftwareShaderModule;
class SoftwareDescriptorSetLayout;
class SoftwareDescriptorPool;
class SoftwareDescriptorSet;
class SoftwareSampler;

class IDriverDevice {
public:
    virtual ~IDriverDevice() = default;

    virtual KxResult allocate_memory(
        const KxMemoryAllocateInfo* allocate_info,
        const KxAllocationCallbacks* allocator,
        KxDeviceMemory* memory) = 0;

    virtual void free_memory(
        KxDeviceMemory memory,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_buffer(
        const KxBufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxBuffer* buffer) = 0;

    virtual void destroy_buffer(
        KxBuffer buffer,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_image(
        const KxImageCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImage* image) = 0;

    virtual void destroy_image(
        KxImage image,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_image_view(
        const KxImageViewCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImageView* image_view) = 0;

    virtual void destroy_image_view(
        KxImageView image_view,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_render_pass(
        const KxRenderPassCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxRenderPass* render_pass) = 0;

    virtual void destroy_render_pass(
        KxRenderPass render_pass,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_framebuffer(
        const KxFramebufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxFramebuffer* framebuffer) = 0;

    virtual void destroy_framebuffer(
        KxFramebuffer framebuffer,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult create_command_pool(
        const KxCommandPoolCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxCommandPool* command_pool) = 0;

    virtual void destroy_command_pool(
        KxCommandPool command_pool,
        const KxAllocationCallbacks* allocator) = 0;

    virtual KxResult allocate_command_buffers(
        const KxCommandBufferAllocateInfo* allocate_info,
        KxCommandBuffer* command_buffers) = 0;

    virtual void free_command_buffers(
        KxCommandPool command_pool,
        uint32_t count,
        const KxCommandBuffer* command_buffers) = 0;

    virtual KxResult begin_command_buffer(
        KxCommandBuffer command_buffer,
        const KxCommandBufferBeginInfo* begin_info) = 0;

    virtual KxResult end_command_buffer(KxCommandBuffer command_buffer) = 0;

    virtual KxResult queue_submit(
        uint32_t submit_count,
        const KxSubmitInfo* submits,
        KxFence fence) = 0;

    virtual KxResult queue_wait_idle() = 0;
    virtual KxResult device_wait_idle() = 0;

    virtual KxResult bind_buffer_memory(
        KxBuffer buffer,
        KxDeviceMemory memory,
        uint64_t memory_offset) = 0;

    virtual KxResult bind_image_memory(
        KxImage image,
        KxDeviceMemory memory,
        uint64_t memory_offset) = 0;

    virtual KxResult map_memory(
        KxDeviceMemory memory,
        uint64_t offset,
        uint64_t size,
        KxMemoryMapFlags flags,
        void** pp_data) = 0;

    virtual void unmap_memory(
        KxDeviceMemory memory) = 0;

    /* Pipeline */
    virtual KxResult create_pipeline_layout(
        const KxPipelineLayoutCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxPipelineLayout* pipeline_layout) = 0;
    virtual void destroy_pipeline_layout(
        KxPipelineLayout pipeline_layout,
        const KxAllocationCallbacks* allocator) = 0;
    virtual KxResult create_pipeline(
        const KxPipelineCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxPipeline* pipeline) = 0;
    virtual void destroy_pipeline(
        KxPipeline pipeline,
        const KxAllocationCallbacks* allocator) = 0;

    /* Shader module */
    virtual KxResult create_shader_module(
        const KxShaderModuleCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxShaderModule* shader_module) = 0;
    virtual void destroy_shader_module(
        KxShaderModule shader_module,
        const KxAllocationCallbacks* allocator) = 0;

    /* Descriptor set layout */
    virtual KxResult create_descriptor_set_layout(
        const KxDescriptorSetLayoutCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxDescriptorSetLayout* layout) = 0;
    virtual void destroy_descriptor_set_layout(
        KxDescriptorSetLayout layout,
        const KxAllocationCallbacks* allocator) = 0;

    /* Descriptor pool */
    virtual KxResult create_descriptor_pool(
        const KxDescriptorPoolCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxDescriptorPool* pool) = 0;
    virtual void destroy_descriptor_pool(
        KxDescriptorPool pool,
        const KxAllocationCallbacks* allocator) = 0;
    virtual KxResult reset_descriptor_pool(
        KxDescriptorPool pool,
        KxDescriptorPoolResetFlags flags) = 0;

    /* Descriptor set */
    virtual KxResult allocate_descriptor_sets(
        const KxDescriptorSetAllocateInfo* allocate_info,
        KxDescriptorSet* sets) = 0;
    virtual KxResult free_descriptor_sets(
        KxDescriptorPool pool,
        uint32_t count,
        const KxDescriptorSet* sets) = 0;
    virtual void update_descriptor_sets(
        uint32_t descriptor_write_count,
        const KxWriteDescriptorSet* descriptor_writes,
        uint32_t descriptor_copy_count,
        const KxCopyDescriptorSet* descriptor_copies) = 0;

    /* Sampler */
    virtual KxResult create_sampler(
        const KxSamplerCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxSampler* sampler) = 0;
    virtual void destroy_sampler(
        KxSampler sampler,
        const KxAllocationCallbacks* allocator) = 0;

    virtual SoftwareBuffer* find_buffer(KxBuffer buffer) = 0;
    virtual SoftwareImage* find_image(KxImage image) = 0;
    virtual SoftwareImageView* find_image_view(KxImageView view) = 0;
    virtual SoftwareRenderPass* find_render_pass(KxRenderPass rp) = 0;
    virtual SoftwareFramebuffer* find_framebuffer(KxFramebuffer fb) = 0;
    virtual SoftwareCommandBuffer* find_command_buffer(KxCommandBuffer cb) = 0;
    virtual SoftwareCommandPool* find_command_pool(KxCommandPool pool) = 0;
    virtual SoftwarePipelineLayout* find_pipeline_layout(KxPipelineLayout layout) = 0;
    virtual SoftwarePipeline* find_pipeline(KxPipeline pipeline) = 0;
    virtual SoftwareShaderModule* find_shader_module(KxShaderModule module) = 0;
    virtual SoftwareDescriptorSetLayout* find_descriptor_set_layout(KxDescriptorSetLayout layout) = 0;
    virtual SoftwareDescriptorPool* find_descriptor_pool(KxDescriptorPool pool) = 0;
    virtual SoftwareDescriptorSet* find_descriptor_set(KxDescriptorSet set) = 0;
    virtual SoftwareSampler* find_sampler(KxSampler sampler) = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_IDRIVERDEVICE_H */