#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H

/*
 * SoftwareDevice.h
 *
 * Software implementation of IDriverDevice. Holds memory pools, buffers,
 * images, command pools and command buffers, and executes recorded
 * command buffers on the CPU.
 */

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "IDriverDevice.h"
#include "PhysicalDevice.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareMemory.h"
#include "SoftwareBuffer.h"
#include "SoftwareImage.h"
#include "SoftwareImageView.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"
#include "SoftwareCommandPool.h"
#include "SoftwarePipeline.h"
#include "SoftwarePipelineLayout.h"
#include "SoftwareShaderModule.h"
#include "SoftwareDescriptorSetLayout.h"
#include "SoftwareDescriptorPool.h"
#include "SoftwareDescriptorSet.h"
#include "SoftwareSampler.h"

namespace kx {

class SoftwareDevice : public IDriverDevice {
public:
    explicit SoftwareDevice(PhysicalDevice* physical_device);
    ~SoftwareDevice() override = default;

    PhysicalDevice* physical_device() const noexcept { return physical_device_; }

    /* Object lookups (public handle -> internal object) */
    SoftwareMemory* find_memory(KxDeviceMemory mem) noexcept;
    SoftwareBuffer* find_buffer(KxBuffer buf) noexcept;
    SoftwareImage* find_image(KxImage img) noexcept;
    SoftwareImageView* find_image_view(KxImageView view) noexcept;
    SoftwareRenderPass* find_render_pass(KxRenderPass rp) noexcept;
    SoftwareFramebuffer* find_framebuffer(KxFramebuffer fb) noexcept;
    SoftwareCommandPool* find_command_pool(KxCommandPool pool) noexcept;
    SoftwareCommandBuffer* find_command_buffer(KxCommandBuffer cb) noexcept;
    SoftwarePipelineLayout* find_pipeline_layout(KxPipelineLayout layout) noexcept;
    SoftwarePipeline* find_pipeline(KxPipeline pipeline) noexcept;
    SoftwareShaderModule* find_shader_module(KxShaderModule module) noexcept;
    SoftwareDescriptorSetLayout* find_descriptor_set_layout(KxDescriptorSetLayout layout) noexcept;
    SoftwareDescriptorPool* find_descriptor_pool(KxDescriptorPool pool) noexcept;
    SoftwareDescriptorSet* find_descriptor_set(KxDescriptorSet set) noexcept;
    SoftwareSampler* find_sampler(KxSampler sampler) noexcept;

    /* Descriptor set accessors (for rasterizer) */
    KxPipelineLayout current_pipeline_layout() const noexcept { return current_pipeline_layout_; }
    const KxDescriptorSet* current_descriptor_sets() const noexcept { return current_descriptor_sets_; }
    uint32_t current_first_descriptor_set() const noexcept { return current_first_descriptor_set_; }
    uint32_t current_descriptor_set_count() const noexcept { return current_descriptor_set_count_; }

    /* Object registration */
    KxDeviceMemory register_memory(std::unique_ptr<SoftwareMemory> mem);
    KxBuffer register_buffer(std::unique_ptr<SoftwareBuffer> buf);
    KxImage register_image(std::unique_ptr<SoftwareImage> img);
    KxImageView register_image_view(std::unique_ptr<SoftwareImageView> view);
    KxRenderPass register_render_pass(std::unique_ptr<SoftwareRenderPass> rp);
    KxFramebuffer register_framebuffer(std::unique_ptr<SoftwareFramebuffer> fb);
    KxCommandPool register_command_pool(std::unique_ptr<SoftwareCommandPool> pool);
    KxCommandBuffer register_command_buffer(std::unique_ptr<SoftwareCommandBuffer> cmd);
    KxPipelineLayout register_pipeline_layout(std::unique_ptr<SoftwarePipelineLayout> layout);
    KxPipeline register_pipeline(std::unique_ptr<SoftwarePipeline> pipeline);
    KxShaderModule register_shader_module(std::unique_ptr<SoftwareShaderModule> module);
    KxDescriptorSetLayout register_descriptor_set_layout(std::unique_ptr<SoftwareDescriptorSetLayout> layout);
    KxDescriptorPool register_descriptor_pool(std::unique_ptr<SoftwareDescriptorPool> pool);
    KxDescriptorSet register_descriptor_set(std::unique_ptr<SoftwareDescriptorSet> set);
    KxSampler register_sampler(std::unique_ptr<SoftwareSampler> sampler);

    void unregister_memory(KxDeviceMemory mem);
    void unregister_buffer(KxBuffer buf);
    void unregister_image(KxImage img);
    void unregister_image_view(KxImageView view);
    void unregister_render_pass(KxRenderPass rp);
    void unregister_framebuffer(KxFramebuffer fb);
    void unregister_command_pool(KxCommandPool pool);
    void unregister_command_buffer(KxCommandBuffer cb);
    void unregister_pipeline_layout(KxPipelineLayout layout);
    void unregister_pipeline(KxPipeline pipeline);
    void unregister_shader_module(KxShaderModule module);
    void unregister_descriptor_set_layout(KxDescriptorSetLayout layout);
    void unregister_descriptor_pool(KxDescriptorPool pool);
    void unregister_descriptor_set(KxDescriptorSet set);
    void unregister_sampler(KxSampler sampler);

    /* IDriverDevice */
    KxResult allocate_memory(const KxMemoryAllocateInfo*, const KxAllocationCallbacks*, KxDeviceMemory*) override;
    void free_memory(KxDeviceMemory, const KxAllocationCallbacks*) override;
    KxResult create_buffer(const KxBufferCreateInfo*, const KxAllocationCallbacks*, KxBuffer*) override;
    void destroy_buffer(KxBuffer, const KxAllocationCallbacks*) override;
    KxResult create_image(const KxImageCreateInfo*, const KxAllocationCallbacks*, KxImage*) override;
    void destroy_image(KxImage, const KxAllocationCallbacks*) override;
    KxResult create_image_view(const KxImageViewCreateInfo*, const KxAllocationCallbacks*, KxImageView*) override;
    void destroy_image_view(KxImageView, const KxAllocationCallbacks*) override;
    KxResult create_render_pass(const KxRenderPassCreateInfo*, const KxAllocationCallbacks*, KxRenderPass*) override;
    void destroy_render_pass(KxRenderPass, const KxAllocationCallbacks*) override;
    KxResult create_framebuffer(const KxFramebufferCreateInfo*, const KxAllocationCallbacks*, KxFramebuffer*) override;
    void destroy_framebuffer(KxFramebuffer, const KxAllocationCallbacks*) override;
    KxResult create_command_pool(const KxCommandPoolCreateInfo*, const KxAllocationCallbacks*, KxCommandPool*) override;
    void destroy_command_pool(KxCommandPool, const KxAllocationCallbacks*) override;
    KxResult allocate_command_buffers(const KxCommandBufferAllocateInfo*, KxCommandBuffer*) override;
    void free_command_buffers(KxCommandPool, uint32_t, const KxCommandBuffer*) override;
    KxResult begin_command_buffer(KxCommandBuffer, const KxCommandBufferBeginInfo*) override;
    KxResult end_command_buffer(KxCommandBuffer) override;
    KxResult reset_command_buffer(KxCommandBuffer, KxCommandBufferResetFlags) override;

    void cmd_begin_render_pass(KxCommandBuffer, const KxRenderPassBeginInfo*, KxSubpassContents) override;
    void cmd_end_render_pass(KxCommandBuffer) override;
    void cmd_next_subpass(KxCommandBuffer, KxSubpassContents) override;

    KxResult queue_submit(uint32_t, const KxSubmitInfo*, KxFence) override;
    KxResult queue_wait_idle() override;
    KxResult device_wait_idle() override;
    KxResult bind_buffer_memory(KxBuffer, KxDeviceMemory, uint64_t) override;
    KxResult bind_image_memory(KxImage, KxDeviceMemory, uint64_t) override;
    KxResult map_memory(KxDeviceMemory, uint64_t, uint64_t, KxMemoryMapFlags, void**) override;
    void unmap_memory(KxDeviceMemory) override;

    /* Pipeline */
    KxResult create_pipeline_layout(const KxPipelineLayoutCreateInfo*, const KxAllocationCallbacks*, KxPipelineLayout*) override;
    void destroy_pipeline_layout(KxPipelineLayout, const KxAllocationCallbacks*) override;
    KxResult create_pipeline(const KxPipelineCreateInfo*, const KxAllocationCallbacks*, KxPipeline*) override;
    void destroy_pipeline(KxPipeline, const KxAllocationCallbacks*) override;

    /* Shader module */
    KxResult create_shader_module(const KxShaderModuleCreateInfo*, const KxAllocationCallbacks*, KxShaderModule*) override;
    void destroy_shader_module(KxShaderModule, const KxAllocationCallbacks*) override;

    /* Descriptor set layout */
    KxResult create_descriptor_set_layout(const KxDescriptorSetLayoutCreateInfo*, const KxAllocationCallbacks*, KxDescriptorSetLayout*) override;
    void destroy_descriptor_set_layout(KxDescriptorSetLayout, const KxAllocationCallbacks*) override;

    /* Descriptor pool */
    KxResult create_descriptor_pool(const KxDescriptorPoolCreateInfo*, const KxAllocationCallbacks*, KxDescriptorPool*) override;
    void destroy_descriptor_pool(KxDescriptorPool, const KxAllocationCallbacks*) override;
    KxResult reset_descriptor_pool(KxDescriptorPool, KxDescriptorPoolResetFlags) override;

    /* Descriptor set */
    KxResult allocate_descriptor_sets(const KxDescriptorSetAllocateInfo*, KxDescriptorSet*) override;
    KxResult free_descriptor_sets(KxDescriptorPool, uint32_t, const KxDescriptorSet*) override;
    void update_descriptor_sets(uint32_t, const KxWriteDescriptorSet*, uint32_t, const KxCopyDescriptorSet*) override;

    /* Sampler */
    KxResult create_sampler(const KxSamplerCreateInfo*, const KxAllocationCallbacks*, KxSampler*) override;
    void destroy_sampler(KxSampler, const KxAllocationCallbacks*) override;

    /* Fence */
    KxResult create_fence(const KxFenceCreateInfo*, const KxAllocationCallbacks*, KxFence*) override;
    void destroy_fence(KxFence, const KxAllocationCallbacks*) override;
    KxResult wait_for_fences(uint32_t, const KxFence*, KxBool32, uint64_t) override;
    KxResult reset_fences(uint32_t, const KxFence*) override;
    KxResult get_fence_status(KxFence) override;

    /* Semaphore */
    KxResult create_semaphore(const KxSemaphoreCreateInfo*, const KxAllocationCallbacks*, KxSemaphore*) override;
    void destroy_semaphore(KxSemaphore, const KxAllocationCallbacks*) override;

    /* Swapchain */
    KxResult create_swapchain(const KxSwapchainCreateInfoKHR*, const KxAllocationCallbacks*, KxSwapchainKHR*) override;
    void destroy_swapchain(KxSwapchainKHR, const KxAllocationCallbacks*) override;
    KxResult acquire_next_image(KxSwapchainKHR, uint64_t, KxSemaphore, KxFence, uint32_t*) override;
    KxResult queue_present(KxQueue, const KxPresentInfoKHR*) override;

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    void cmd_begin_rendering(KxCommandBuffer, const KxRenderingInfoKHR*) override;
    void cmd_end_rendering(KxCommandBuffer) override;

    // Synchronization 2 (VK_KHR_synchronization2)
    void cmd_pipeline_barrier2(KxCommandBuffer, const KxDependencyInfoKHR*) override;
    void cmd_write_timestamp2(KxCommandBuffer, KxPipelineStageFlags2KHR, KxQueryPool, uint32_t) override;
    void cmd_set_event2(KxCommandBuffer, KxEvent, const KxDependencyInfoKHR*) override;
    void cmd_reset_event2(KxCommandBuffer, KxEvent, KxPipelineStageFlags2KHR) override;
    void cmd_wait_events2(KxCommandBuffer, uint32_t, const KxEvent*, const KxDependencyInfoKHR*) override;

    // Copy Commands v2 (VK_KHR_copy_commands2)
    void cmd_copy_buffer2(KxCommandBuffer, const void*) override;
    void cmd_copy_image2(KxCommandBuffer, const void*) override;
    void cmd_blit_image2(KxCommandBuffer, const void*) override;
    void cmd_copy_buffer_to_image2(KxCommandBuffer, const void*) override;
    void cmd_copy_image_to_buffer2(KxCommandBuffer, const void*) override;
    void cmd_resolve_image2(KxCommandBuffer, const void*) override;

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    void cmd_set_depth_clamp_enable(KxCommandBuffer, KxBool32) override;
    void cmd_set_polygon_mode(KxCommandBuffer, KxPolygonMode) override;
    void cmd_set_rasterization_samples(KxCommandBuffer, KxSampleCountFlagBits) override;
    void cmd_set_sample_mask(KxCommandBuffer, KxSampleMask) override;
    void cmd_set_alpha_to_coverage_enable(KxCommandBuffer, KxBool32) override;
    void cmd_set_alpha_to_one_enable(KxCommandBuffer, KxBool32) override;
    void cmd_set_logic_op_enable(KxCommandBuffer, KxBool32) override;
    void cmd_set_color_blend_enable(KxCommandBuffer, uint32_t, uint32_t, const KxBool32*) override;
    void cmd_set_color_blend_equation(KxCommandBuffer, uint32_t, uint32_t, const void*) override;
    void cmd_set_color_write_mask(KxCommandBuffer, uint32_t, uint32_t, const KxColorComponentFlags*) override;
    void cmd_set_tessellation_domain_origin(KxCommandBuffer, KxTessellationDomainOriginKHR) override;
    void cmd_set_rasterization_stream(KxCommandBuffer, uint32_t) override;
    void cmd_set_conservative_rasterization_mode(KxCommandBuffer, KxConservativeRasterizationModeEXT) override;
    void cmd_set_extra_primitive_overestimation_size(KxCommandBuffer, float) override;
    void cmd_set_depth_clip_enable(KxCommandBuffer, KxBool32) override;
    void cmd_set_depth_clip_negative_one_to_one(KxCommandBuffer, KxBool32) override;
    void cmd_set_viewport_w_scaling_enable_nv(KxCommandBuffer, KxBool32) override;
    void cmd_set_viewport_swizzle_nv(KxCommandBuffer, uint32_t, uint32_t, const void*) override;

private:
    PhysicalDevice* physical_device_;

    void execute_command(const CommandVariant& cmd);
    void execute_clear_image(const CmdClearImage& cmd);
    void execute_copy_buffer(const CmdCopyBuffer& cmd);
    void execute_copy_buffer_to_image(const CmdCopyBufferToImage& cmd);
    void execute_begin_render_pass(const CmdBeginRenderPass& cmd);
    void execute_end_render_pass(const CmdEndRenderPass& cmd);
    void execute_next_subpass(const CmdNextSubpass& cmd);
    void execute_draw(const CmdDraw& cmd);
    void execute_draw_indexed(const CmdDrawIndexed& cmd);
    void execute_bind_descriptor_sets(const CmdBindDescriptorSets& cmd);
    void execute_dispatch(const CmdDispatch& cmd);

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    void execute_begin_rendering(const CmdBeginRendering& cmd);
    void execute_end_rendering(const CmdEndRendering& cmd);

    // Synchronization 2 (VK_KHR_synchronization2)
    void execute_pipeline_barrier2(const CmdPipelineBarrier2& cmd);
    void execute_write_timestamp2(const CmdWriteTimestamp2& cmd);
    void execute_set_event2(const CmdSetEvent2& cmd);
    void execute_reset_event2(const CmdResetEvent2& cmd);
    void execute_wait_events2(const CmdWaitEvents2& cmd);

    // Copy Commands v2 (VK_KHR_copy_commands2)
    void execute_copy_buffer2(const CmdCopyBuffer2& cmd);
    void execute_copy_image2(const CmdCopyImage2& cmd);
    void execute_blit_image2(const CmdBlitImage2& cmd);
    void execute_copy_buffer_to_image2(const CmdCopyBufferToImage2& cmd);
    void execute_copy_image_to_buffer2(const CmdCopyImageToBuffer2& cmd);
    void execute_resolve_image2(const CmdResolveImage2& cmd);

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    void execute_set_depth_clamp_enable(const CmdSetDepthClampEnable& cmd);
    void execute_set_polygon_mode(const CmdSetPolygonMode& cmd);
    void execute_set_rasterization_samples(const CmdSetRasterizationSamples& cmd);
    void execute_set_sample_mask(const CmdSetSampleMask& cmd);
    void execute_set_alpha_to_coverage_enable(const CmdSetAlphaToCoverageEnable& cmd);
    void execute_set_alpha_to_one_enable(const CmdSetAlphaToOneEnable& cmd);
    void execute_set_logic_op_enable(const CmdSetLogicOpEnable& cmd);
    void execute_set_color_blend_enable(const CmdSetColorBlendEnable& cmd);
    void execute_set_color_blend_equation(const CmdSetColorBlendEquation& cmd);
    void execute_set_color_write_mask(const CmdSetColorWriteMask& cmd);
    void execute_set_tessellation_domain_origin(const CmdSetTessellationDomainOrigin& cmd);
    void execute_set_rasterization_stream(const CmdSetRasterizationStream& cmd);
    void execute_set_conservative_rasterization_mode(const CmdSetConservativeRasterizationMode& cmd);
    void execute_set_extra_primitive_overestimation_size(const CmdSetExtraPrimitiveOverestimationSize& cmd);
    void execute_set_depth_clip_enable(const CmdSetDepthClipEnable& cmd);
    void execute_set_depth_clip_negative_one_to_one(const CmdSetDepthClipNegativeOneToOne& cmd);
    void execute_set_viewport_w_scaling_enable_nv(const CmdSetViewportWScalingEnableNV& cmd);
    void execute_set_viewport_swizzle_nv(const CmdSetViewportSwizzleNV& cmd);

    static uint32_t format_pixel_size(KxFormat format);

    // Current pipeline state for draw commands
    SoftwarePipeline* current_pipeline_ = nullptr;
    CmdBindVertexBuffers current_vertex_buffers_{};
    CmdBindIndexBuffer current_index_buffer_{};

    // Current descriptor sets state
    KxPipelineLayout current_pipeline_layout_ = nullptr;
    KxDescriptorSet current_descriptor_sets_[16];
    uint32_t current_first_descriptor_set_ = 0;
    uint32_t current_descriptor_set_count_ = 0;

    // Current render pass state
    SoftwareRenderPass* current_render_pass_ = nullptr;
    SoftwareFramebuffer* current_framebuffer_ = nullptr;
    uint32_t current_subpass_ = 0;

    std::vector<std::unique_ptr<SoftwareMemory>> memories_;
    std::vector<std::unique_ptr<SoftwareBuffer>> buffers_;
    std::vector<std::unique_ptr<SoftwareImage>> images_;
    std::vector<std::unique_ptr<SoftwareImageView>> image_views_;
    std::vector<std::unique_ptr<SoftwareRenderPass>> render_passes_;
    std::vector<std::unique_ptr<SoftwareFramebuffer>> framebuffers_;
    std::vector<std::unique_ptr<SoftwareCommandPool>> command_pools_;
    std::vector<std::unique_ptr<SoftwareCommandBuffer>> command_buffers_;
    std::vector<std::unique_ptr<SoftwarePipelineLayout>> pipeline_layouts_;
    std::vector<std::unique_ptr<SoftwarePipeline>> pipelines_;
    std::vector<std::unique_ptr<SoftwareShaderModule>> shader_modules_;
    std::vector<std::unique_ptr<SoftwareDescriptorSetLayout>> descriptor_set_layouts_;
    std::vector<std::unique_ptr<SoftwareDescriptorPool>> descriptor_pools_;
    std::vector<std::unique_ptr<SoftwareDescriptorSet>> descriptor_sets_;
    std::vector<std::unique_ptr<SoftwareSampler>> samplers_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H */