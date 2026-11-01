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

    virtual KxResult queue_submit(
        uint32_t submit_count,
        const KxSubmitInfo* submits,
        KxFence fence) = 0;

    // Command buffer recording
    virtual KxResult begin_command_buffer(
        KxCommandBuffer command_buffer,
        const KxCommandBufferBeginInfo* begin_info) = 0;

    virtual KxResult end_command_buffer(KxCommandBuffer command_buffer) = 0;

    virtual KxResult reset_command_buffer(
        KxCommandBuffer command_buffer,
        KxCommandBufferResetFlags flags) = 0;

    virtual void cmd_begin_render_pass(
        KxCommandBuffer command_buffer,
        const KxRenderPassBeginInfo* render_pass_begin,
        KxSubpassContents contents) = 0;

    virtual void cmd_end_render_pass(KxCommandBuffer command_buffer) = 0;

    virtual void cmd_next_subpass(
        KxCommandBuffer command_buffer,
        KxSubpassContents contents) = 0;

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    virtual void cmd_begin_rendering(
        KxCommandBuffer command_buffer,
        const KxRenderingInfoKHR* rendering_info) = 0;

    virtual void cmd_end_rendering(KxCommandBuffer command_buffer) = 0;

    // Synchronization 2 (VK_KHR_synchronization2)
    virtual void cmd_pipeline_barrier2(
        KxCommandBuffer command_buffer,
        const KxDependencyInfoKHR* dependency_info) = 0;

    virtual void cmd_write_timestamp2(
        KxCommandBuffer command_buffer,
        KxPipelineStageFlags2KHR stage,
        KxQueryPool query_pool,
        uint32_t query) = 0;

    virtual void cmd_set_event2(
        KxCommandBuffer command_buffer,
        KxEvent event,
        const KxDependencyInfoKHR* dependency_info) = 0;

    virtual void cmd_reset_event2(
        KxCommandBuffer command_buffer,
        KxEvent event,
        KxPipelineStageFlags2KHR stage_mask) = 0;

    virtual void cmd_wait_events2(
        KxCommandBuffer command_buffer,
        uint32_t event_count,
        const KxEvent* events,
        const KxDependencyInfoKHR* dependency_infos) = 0;

    // Copy Commands v2 (VK_KHR_copy_commands2)
    virtual void cmd_copy_buffer2(
        KxCommandBuffer command_buffer,
        const void* copy_buffer_info) = 0;

    virtual void cmd_copy_image2(
        KxCommandBuffer command_buffer,
        const void* copy_image_info) = 0;

    virtual void cmd_blit_image2(
        KxCommandBuffer command_buffer,
        const void* blit_image_info) = 0;

    virtual void cmd_copy_buffer_to_image2(
        KxCommandBuffer command_buffer,
        const void* copy_buffer_to_image_info) = 0;

    virtual void cmd_copy_image_to_buffer2(
        KxCommandBuffer command_buffer,
        const void* copy_image_to_buffer_info) = 0;

    virtual void cmd_resolve_image2(
        KxCommandBuffer command_buffer,
        const void* resolve_image_info) = 0;

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    virtual void cmd_set_depth_clamp_enable(
        KxCommandBuffer command_buffer,
        KxBool32 depth_clamp_enable) = 0;

    virtual void cmd_set_polygon_mode(
        KxCommandBuffer command_buffer,
        KxPolygonMode polygon_mode) = 0;

    virtual void cmd_set_rasterization_samples(
        KxCommandBuffer command_buffer,
        KxSampleCountFlagBits rasterization_samples) = 0;

    virtual void cmd_set_sample_mask(
        KxCommandBuffer command_buffer,
        KxSampleMask sample_mask) = 0;

    virtual void cmd_set_alpha_to_coverage_enable(
        KxCommandBuffer command_buffer,
        KxBool32 alpha_to_coverage_enable) = 0;

    virtual void cmd_set_alpha_to_one_enable(
        KxCommandBuffer command_buffer,
        KxBool32 alpha_to_one_enable) = 0;

    virtual void cmd_set_logic_op_enable(
        KxCommandBuffer command_buffer,
        KxBool32 logic_op_enable) = 0;

    virtual void cmd_set_color_blend_enable(
        KxCommandBuffer command_buffer,
        uint32_t first_attachment,
        uint32_t attachment_count,
        const KxBool32* color_blend_enables) = 0;

    virtual void cmd_set_color_blend_equation(
        KxCommandBuffer command_buffer,
        uint32_t first_attachment,
        uint32_t attachment_count,
        const void* color_blend_equations) = 0;

    virtual void cmd_set_color_write_mask(
        KxCommandBuffer command_buffer,
        uint32_t first_attachment,
        uint32_t attachment_count,
        const KxColorComponentFlags* color_write_masks) = 0;

    virtual void cmd_set_tessellation_domain_origin(
        KxCommandBuffer command_buffer,
        KxTessellationDomainOriginKHR domain_origin) = 0;

    virtual void cmd_set_rasterization_stream(
        KxCommandBuffer command_buffer,
        uint32_t rasterization_stream) = 0;

    virtual void cmd_set_conservative_rasterization_mode(
        KxCommandBuffer command_buffer,
        KxConservativeRasterizationModeEXT conservative_rasterization_mode) = 0;

    virtual void cmd_set_extra_primitive_overestimation_size(
        KxCommandBuffer command_buffer,
        float extra_primitive_overestimation_size) = 0;

    virtual void cmd_set_depth_clip_enable(
        KxCommandBuffer command_buffer,
        KxBool32 depth_clip_enable) = 0;

    virtual void cmd_set_depth_clip_negative_one_to_one(
        KxCommandBuffer command_buffer,
        KxBool32 negative_one_to_one) = 0;

    virtual void cmd_set_viewport_w_scaling_enable_nv(
        KxCommandBuffer command_buffer,
        KxBool32 viewport_w_scaling_enable) = 0;

    virtual void cmd_set_viewport_swizzle_nv(
        KxCommandBuffer command_buffer,
        uint32_t first_viewport,
        uint32_t viewport_count,
        const void* viewport_swizzles) = 0;

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

    /* Fence */
    virtual KxResult create_fence(
        const KxFenceCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxFence* fence) = 0;
    virtual void destroy_fence(
        KxFence fence,
        const KxAllocationCallbacks* allocator) = 0;
    virtual KxResult wait_for_fences(
        uint32_t fence_count,
        const KxFence* fences,
        KxBool32 wait_all,
        uint64_t timeout) = 0;
    virtual KxResult reset_fences(
        uint32_t fence_count,
        const KxFence* fences) = 0;
    virtual KxResult get_fence_status(KxFence fence) = 0;

    /* Semaphore */
    virtual KxResult create_semaphore(
        const KxSemaphoreCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxSemaphore* semaphore) = 0;
    virtual void destroy_semaphore(
        KxSemaphore semaphore,
        const KxAllocationCallbacks* allocator) = 0;

    /* Swapchain */
    virtual KxResult create_swapchain(
        const KxSwapchainCreateInfoKHR* create_info,
        const KxAllocationCallbacks* allocator,
        KxSwapchainKHR* swapchain) = 0;
    virtual void destroy_swapchain(
        KxSwapchainKHR swapchain,
        const KxAllocationCallbacks* allocator) = 0;
    virtual KxResult acquire_next_image(
        KxSwapchainKHR swapchain,
        uint64_t timeout,
        KxSemaphore semaphore,
        KxFence fence,
        uint32_t* image_index) = 0;
    virtual KxResult queue_present(
        KxQueue queue,
        const KxPresentInfoKHR* present_info) = 0;

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