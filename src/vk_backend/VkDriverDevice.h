#ifndef KROMAX_SRC_VK_BACKEND_VKDRIVERDEVICE_H
#define KROMAX_SRC_VK_BACKEND_VKDRIVERDEVICE_H

/*
 * VkDriverDevice.h
 *
 * Vulkan implementation of IDriverDevice. Wraps Vulkan objects and
 * provides the backend-specific implementations for all device operations.
 */

#include "IDriverDevice.h"
#include "VkLoader.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Forward declarations
namespace kx {
class VkDeviceImpl;
}

namespace kx {

// Forward declarations for Vulkan object implementations
class VkShaderModuleImpl;
class VkPipelineImpl;
class VkPipelineLayoutImpl;
class VkDescriptorSetLayoutImpl;
class VkDescriptorPoolImpl;
class VkDescriptorSetImpl;
class VkCommandPoolImpl;
class VkCommandBufferImpl;
class VkRenderPassImpl;
class VkFramebufferImpl;
class VkBufferImpl;
class VkImageImpl;
class VkImageViewImpl;
class VkSamplerImpl;
class VkMemoryImpl;
class VkSwapchainImpl;
class VkSemaphoreImpl;
class VkFenceImpl;
class VkQueueImpl;

class VkDriverDevice final : public IDriverDevice {
public:
    explicit VkDriverDevice(VkDeviceImpl* device);
    ~VkDriverDevice() override;

    // Disable copy
    VkDriverDevice(const VkDriverDevice&) = delete;
    VkDriverDevice& operator=(const VkDriverDevice&) = delete;

    // IDriverDevice implementation
    KxResult allocate_memory(const KxMemoryAllocateInfo* allocate_info,
                             const KxAllocationCallbacks* allocator,
                             KxDeviceMemory* memory) override;
    void free_memory(KxDeviceMemory memory,
                     const KxAllocationCallbacks* allocator) override;

    KxResult create_buffer(const KxBufferCreateInfo* create_info,
                           const KxAllocationCallbacks* allocator,
                           KxBuffer* buffer) override;
    void destroy_buffer(KxBuffer buffer,
                        const KxAllocationCallbacks* allocator) override;

    KxResult create_image(const KxImageCreateInfo* create_info,
                          const KxAllocationCallbacks* allocator,
                          KxImage* image) override;
    void destroy_image(KxImage image,
                       const KxAllocationCallbacks* allocator) override;

    KxResult create_image_view(const KxImageViewCreateInfo* create_info,
                               const KxAllocationCallbacks* allocator,
                               KxImageView* image_view) override;
    void destroy_image_view(KxImageView image_view,
                            const KxAllocationCallbacks* allocator) override;

    KxResult create_render_pass(const KxRenderPassCreateInfo* create_info,
                                const KxAllocationCallbacks* allocator,
                                KxRenderPass* render_pass) override;
    void destroy_render_pass(KxRenderPass render_pass,
                             const KxAllocationCallbacks* allocator) override;

    KxResult create_framebuffer(const KxFramebufferCreateInfo* create_info,
                                const KxAllocationCallbacks* allocator,
                                KxFramebuffer* framebuffer) override;
    void destroy_framebuffer(KxFramebuffer framebuffer,
                             const KxAllocationCallbacks* allocator) override;

    KxResult create_command_pool(const KxCommandPoolCreateInfo* create_info,
                                 const KxAllocationCallbacks* allocator,
                                 KxCommandPool* command_pool) override;
    void destroy_command_pool(KxCommandPool command_pool,
                              const KxAllocationCallbacks* allocator) override;

    KxResult allocate_command_buffers(const KxCommandBufferAllocateInfo* allocate_info,
                                      KxCommandBuffer* command_buffers) override;
    void free_command_buffers(KxCommandPool command_pool,
                              uint32_t count,
                              const KxCommandBuffer* command_buffers) override;

    KxResult queue_submit(uint32_t submit_count,
                          const KxSubmitInfo* submits,
                          KxFence fence) override;

    // Command buffer recording
    KxResult begin_command_buffer(KxCommandBuffer command_buffer,
                                  const KxCommandBufferBeginInfo* begin_info) override;
    KxResult end_command_buffer(KxCommandBuffer command_buffer) override;
    KxResult reset_command_buffer(KxCommandBuffer command_buffer,
                                  KxCommandBufferResetFlags flags) override;

    void cmd_begin_render_pass(KxCommandBuffer command_buffer,
                               const KxRenderPassBeginInfo* render_pass_begin,
                               KxSubpassContents contents) override;
    void cmd_end_render_pass(KxCommandBuffer command_buffer) override;
    void cmd_next_subpass(KxCommandBuffer command_buffer,
                          KxSubpassContents contents) override;

    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    void cmd_begin_rendering(KxCommandBuffer command_buffer,
                             const KxRenderingInfoKHR* rendering_info) override;
    void cmd_end_rendering(KxCommandBuffer command_buffer) override;

    // Synchronization 2 (VK_KHR_synchronization2)
    void cmd_pipeline_barrier2(KxCommandBuffer command_buffer,
                               const KxDependencyInfoKHR* dependency_info) override;
    void cmd_write_timestamp2(KxCommandBuffer command_buffer,
                              KxPipelineStageFlags2KHR stage,
                              KxQueryPool query_pool,
                              uint32_t query) override;
    void cmd_set_event2(KxCommandBuffer command_buffer,
                        KxEvent event,
                        const KxDependencyInfoKHR* dependency_info) override;
    void cmd_reset_event2(KxCommandBuffer command_buffer,
                          KxEvent event,
                          KxPipelineStageFlags2KHR stage_mask) override;
    void cmd_wait_events2(KxCommandBuffer command_buffer,
                          uint32_t event_count,
                          const KxEvent* events,
                          const KxDependencyInfoKHR* dependency_infos) override;

    // Copy Commands v2 (VK_KHR_copy_commands2)
    void cmd_copy_buffer2(KxCommandBuffer command_buffer,
                          const void* copy_buffer_info) override;
    void cmd_copy_image2(KxCommandBuffer command_buffer,
                         const void* copy_image_info) override;
    void cmd_blit_image2(KxCommandBuffer command_buffer,
                         const void* blit_image_info) override;
    void cmd_copy_buffer_to_image2(KxCommandBuffer command_buffer,
                                   const void* copy_buffer_to_image_info) override;
    void cmd_copy_image_to_buffer2(KxCommandBuffer command_buffer,
                                   const void* copy_image_to_buffer_info) override;
    void cmd_resolve_image2(KxCommandBuffer command_buffer,
                            const void* resolve_image_info) override;

    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    void cmd_set_depth_clamp_enable(KxCommandBuffer command_buffer,
                                    KxBool32 depth_clamp_enable) override;
    void cmd_set_polygon_mode(KxCommandBuffer command_buffer,
                              KxPolygonMode polygon_mode) override;
    void cmd_set_rasterization_samples(KxCommandBuffer command_buffer,
                                       KxSampleCountFlagBits rasterization_samples) override;
    void cmd_set_sample_mask(KxCommandBuffer command_buffer,
                             KxSampleMask sample_mask) override;
    void cmd_set_alpha_to_coverage_enable(KxCommandBuffer command_buffer,
                                          KxBool32 alpha_to_coverage_enable) override;
    void cmd_set_alpha_to_one_enable(KxCommandBuffer command_buffer,
                                     KxBool32 alpha_to_one_enable) override;
    void cmd_set_logic_op_enable(KxCommandBuffer command_buffer,
                                 KxBool32 logic_op_enable) override;
    void cmd_set_color_blend_enable(KxCommandBuffer command_buffer,
                                    uint32_t first_attachment,
                                    uint32_t attachment_count,
                                    const KxBool32* color_blend_enables) override;
    void cmd_set_color_blend_equation(KxCommandBuffer command_buffer,
                                      uint32_t first_attachment,
                                      uint32_t attachment_count,
                                      const void* color_blend_equations) override;
    void cmd_set_color_write_mask(KxCommandBuffer command_buffer,
                                  uint32_t first_attachment,
                                  uint32_t attachment_count,
                                  const KxColorComponentFlags* color_write_masks) override;
    void cmd_set_tessellation_domain_origin(KxCommandBuffer command_buffer,
                                            KxTessellationDomainOriginKHR domain_origin) override;
    void cmd_set_rasterization_stream(KxCommandBuffer command_buffer,
                                      uint32_t rasterization_stream) override;
    void cmd_set_conservative_rasterization_mode(KxCommandBuffer command_buffer,
                                                 KxConservativeRasterizationModeEXT conservative_rasterization_mode) override;
    void cmd_set_extra_primitive_overestimation_size(KxCommandBuffer command_buffer,
                                                     float extra_primitive_overestimation_size) override;
    void cmd_set_depth_clip_enable(KxCommandBuffer command_buffer,
                                   KxBool32 depth_clip_enable) override;
    void cmd_set_depth_clip_negative_one_to_one(KxCommandBuffer command_buffer,
                                                KxBool32 negative_one_to_one) override;
    void cmd_set_viewport_w_scaling_enable_nv(KxCommandBuffer command_buffer,
                                              KxBool32 viewport_w_scaling_enable) override;
    void cmd_set_viewport_swizzle_nv(KxCommandBuffer command_buffer,
                                     uint32_t first_viewport,
                                     uint32_t viewport_count,
                                     const void* viewport_swizzles) override;

    KxResult queue_wait_idle() override;
    KxResult device_wait_idle() override;

    KxResult bind_buffer_memory(KxBuffer buffer,
                                KxDeviceMemory memory,
                                uint64_t memory_offset) override;
    KxResult bind_image_memory(KxImage image,
                               KxDeviceMemory memory,
                               uint64_t memory_offset) override;

    KxResult map_memory(KxDeviceMemory memory,
                        uint64_t offset,
                        uint64_t size,
                        KxMemoryMapFlags flags,
                        void** pp_data) override;
    void unmap_memory(KxDeviceMemory memory) override;

    // Pipeline
    KxResult create_pipeline_layout(const KxPipelineLayoutCreateInfo* create_info,
                                    const KxAllocationCallbacks* allocator,
                                    KxPipelineLayout* pipeline_layout) override;
    void destroy_pipeline_layout(KxPipelineLayout pipeline_layout,
                                 const KxAllocationCallbacks* allocator) override;
    KxResult create_pipeline(const KxPipelineCreateInfo* create_info,
                             const KxAllocationCallbacks* allocator,
                             KxPipeline* pipeline) override;
    void destroy_pipeline(KxPipeline pipeline,
                          const KxAllocationCallbacks* allocator) override;

    // Shader module
    KxResult create_shader_module(const KxShaderModuleCreateInfo* create_info,
                                  const KxAllocationCallbacks* allocator,
                                  KxShaderModule* shader_module) override;
    void destroy_shader_module(KxShaderModule shader_module,
                               const KxAllocationCallbacks* allocator) override;

    // Descriptor set layout
    KxResult create_descriptor_set_layout(const KxDescriptorSetLayoutCreateInfo* create_info,
                                          const KxAllocationCallbacks* allocator,
                                          KxDescriptorSetLayout* layout) override;
    void destroy_descriptor_set_layout(KxDescriptorSetLayout layout,
                                       const KxAllocationCallbacks* allocator) override;

    // Descriptor pool
    KxResult create_descriptor_pool(const KxDescriptorPoolCreateInfo* create_info,
                                    const KxAllocationCallbacks* allocator,
                                    KxDescriptorPool* pool) override;
    void destroy_descriptor_pool(KxDescriptorPool pool,
                                 const KxAllocationCallbacks* allocator) override;
    KxResult reset_descriptor_pool(KxDescriptorPool pool,
                                   KxDescriptorPoolResetFlags flags) override;

    // Descriptor set
    KxResult allocate_descriptor_sets(const KxDescriptorSetAllocateInfo* allocate_info,
                                      KxDescriptorSet* sets) override;
    KxResult free_descriptor_sets(KxDescriptorPool pool,
                                  uint32_t count,
                                  const KxDescriptorSet* sets) override;
    void update_descriptor_sets(uint32_t descriptor_write_count,
                                const KxWriteDescriptorSet* descriptor_writes,
                                uint32_t descriptor_copy_count,
                                const KxCopyDescriptorSet* descriptor_copies) override;

    // Sampler
    KxResult create_sampler(const KxSamplerCreateInfo* create_info,
                            const KxAllocationCallbacks* allocator,
                            KxSampler* sampler) override;
    void destroy_sampler(KxSampler sampler,
                         const KxAllocationCallbacks* allocator) override;

    // Fence
    KxResult create_fence(const KxFenceCreateInfo* create_info,
                          const KxAllocationCallbacks* allocator,
                          KxFence* fence) override;
    void destroy_fence(KxFence fence,
                       const KxAllocationCallbacks* allocator) override;
    KxResult wait_for_fences(uint32_t fence_count,
                             const KxFence* fences,
                             KxBool32 wait_all,
                             uint64_t timeout) override;
    KxResult reset_fences(uint32_t fence_count,
                          const KxFence* fences) override;
    KxResult get_fence_status(KxFence fence) override;

    // Semaphore
    KxResult create_semaphore(const KxSemaphoreCreateInfo* create_info,
                              const KxAllocationCallbacks* allocator,
                              KxSemaphore* semaphore) override;
    void destroy_semaphore(KxSemaphore semaphore,
                           const KxAllocationCallbacks* allocator) override;

    // Swapchain
    KxResult create_swapchain(const KxSwapchainCreateInfoKHR* create_info,
                              const KxAllocationCallbacks* allocator,
                              KxSwapchainKHR* swapchain) override;
    void destroy_swapchain(KxSwapchainKHR swapchain,
                           const KxAllocationCallbacks* allocator) override;
    KxResult acquire_next_image(KxSwapchainKHR swapchain,
                                uint64_t timeout,
                                KxSemaphore semaphore,
                                KxFence fence,
                                uint32_t* image_index) override;
    KxResult queue_present(KxQueue queue,
                           const KxPresentInfoKHR* present_info) override;

    // Object lookup (returns Software* types for compatibility, will return nullptr for Vulkan)
    SoftwareBuffer* find_buffer(KxBuffer buffer) override { return nullptr; }
    SoftwareImage* find_image(KxImage image) override { return nullptr; }
    SoftwareImageView* find_image_view(KxImageView view) override { return nullptr; }
    SoftwareRenderPass* find_render_pass(KxRenderPass rp) override { return nullptr; }
    SoftwareFramebuffer* find_framebuffer(KxFramebuffer fb) override { return nullptr; }
    SoftwareCommandBuffer* find_command_buffer(KxCommandBuffer cb) override { return nullptr; }
    SoftwareCommandPool* find_command_pool(KxCommandPool pool) override { return nullptr; }
    SoftwarePipelineLayout* find_pipeline_layout(KxPipelineLayout layout) override { return nullptr; }
    SoftwarePipeline* find_pipeline(KxPipeline pipeline) override { return nullptr; }
    SoftwareShaderModule* find_shader_module(KxShaderModule module) override { return nullptr; }
    SoftwareDescriptorSetLayout* find_descriptor_set_layout(KxDescriptorSetLayout layout) override { return nullptr; }
    SoftwareDescriptorPool* find_descriptor_pool(KxDescriptorPool pool) override { return nullptr; }
    SoftwareDescriptorSet* find_descriptor_set(KxDescriptorSet set) override { return nullptr; }
    SoftwareSampler* find_sampler(KxSampler sampler) override { return nullptr; }

    // Accessors
    VkDeviceImpl* vk_device() const noexcept;
    VkFunctionTable& functions() noexcept;
    VkFunctionTable& functions() const noexcept;

    // Object registration for internal tracking
    KxShaderModule register_shader_module(std::unique_ptr<VkShaderModuleImpl> module);
    KxPipelineLayout register_pipeline_layout(std::unique_ptr<VkPipelineLayoutImpl> layout);
    KxPipeline register_pipeline(std::unique_ptr<VkPipelineImpl> pipeline);
    KxDescriptorSetLayout register_descriptor_set_layout(std::unique_ptr<VkDescriptorSetLayoutImpl> layout);
    KxDescriptorPool register_descriptor_pool(std::unique_ptr<VkDescriptorPoolImpl> pool);
    KxDescriptorSet register_descriptor_set(std::unique_ptr<VkDescriptorSetImpl> set);
    KxCommandPool register_command_pool(std::unique_ptr<VkCommandPoolImpl> pool);
    KxCommandBuffer register_command_buffer(std::unique_ptr<VkCommandBufferImpl> buffer);
    KxRenderPass register_render_pass(std::unique_ptr<VkRenderPassImpl> render_pass);
    KxFramebuffer register_framebuffer(std::unique_ptr<VkFramebufferImpl> framebuffer);
    KxBuffer register_buffer(std::unique_ptr<VkBufferImpl> buffer);
    KxImage register_image(std::unique_ptr<VkImageImpl> image);
    KxImageView register_image_view(std::unique_ptr<VkImageViewImpl> view);
    KxSampler register_sampler(std::unique_ptr<VkSamplerImpl> sampler);
    KxDeviceMemory register_memory(std::unique_ptr<VkMemoryImpl> memory);
    KxFence register_fence(std::unique_ptr<VkFenceImpl> fence);
    KxSemaphore register_semaphore(std::unique_ptr<VkSemaphoreImpl> semaphore);
    KxSwapchainKHR register_swapchain(std::unique_ptr<VkSwapchainImpl> swapchain);
    KxQueue register_queue(std::unique_ptr<VkQueueImpl> queue);

    // Object finders for Vulkan objects
    VkShaderModuleImpl* find_vk_shader_module(KxShaderModule module);
    VkPipelineLayoutImpl* find_vk_pipeline_layout(KxPipelineLayout layout);
    VkPipelineImpl* find_vk_pipeline(KxPipeline pipeline);
    VkDescriptorSetLayoutImpl* find_vk_descriptor_set_layout(KxDescriptorSetLayout layout);
    VkDescriptorPoolImpl* find_vk_descriptor_pool(KxDescriptorPool pool);
    VkDescriptorSetImpl* find_vk_descriptor_set(KxDescriptorSet set);
    VkCommandPoolImpl* find_vk_command_pool(KxCommandPool pool);
    VkCommandBufferImpl* find_vk_command_buffer(KxCommandBuffer buffer);
    VkRenderPassImpl* find_vk_render_pass(KxRenderPass render_pass);
    VkFramebufferImpl* find_vk_framebuffer(KxFramebuffer framebuffer);
    VkBufferImpl* find_vk_buffer(KxBuffer buffer);
    VkImageImpl* find_vk_image(KxImage image);
    VkImageViewImpl* find_vk_image_view(KxImageView view);
    VkSamplerImpl* find_vk_sampler(KxSampler sampler);
    VkMemoryImpl* find_vk_memory(KxDeviceMemory memory);
    VkFenceImpl* find_vk_fence(KxFence fence);
    VkSemaphoreImpl* find_vk_semaphore(KxSemaphore semaphore);
    VkSwapchainImpl* find_vk_swapchain(KxSwapchainKHR swapchain);
    VkQueueImpl* find_vk_queue(KxQueue queue);

    void unregister_shader_module(KxShaderModule module);
    void unregister_pipeline_layout(KxPipelineLayout layout);
    void unregister_pipeline(KxPipeline pipeline);
    void unregister_descriptor_set_layout(KxDescriptorSetLayout layout);
    void unregister_descriptor_pool(KxDescriptorPool pool);
    void unregister_descriptor_set(KxDescriptorSet set);
    void unregister_command_pool(KxCommandPool pool);
    void unregister_command_buffer(KxCommandBuffer buffer);
    void unregister_render_pass(KxRenderPass render_pass);
    void unregister_framebuffer(KxFramebuffer framebuffer);
    void unregister_buffer(KxBuffer buffer);
    void unregister_image(KxImage image);
    void unregister_image_view(KxImageView view);
    void unregister_sampler(KxSampler sampler);
    void unregister_memory(KxDeviceMemory memory);
    void unregister_fence(KxFence fence);
    void unregister_semaphore(KxSemaphore semaphore);
    void unregister_swapchain(KxSwapchainKHR swapchain);
    void unregister_queue(KxQueue queue);

private:
    VkDeviceImpl* vk_device_;

    // Object registries using unordered_map for O(1) lookup
    std::unordered_map<uint64_t, std::unique_ptr<VkShaderModuleImpl>> shader_modules_;
    std::unordered_map<uint64_t, std::unique_ptr<VkPipelineLayoutImpl>> pipeline_layouts_;
    std::unordered_map<uint64_t, std::unique_ptr<VkPipelineImpl>> pipelines_;
    std::unordered_map<uint64_t, std::unique_ptr<VkDescriptorSetLayoutImpl>> descriptor_set_layouts_;
    std::unordered_map<uint64_t, std::unique_ptr<VkDescriptorPoolImpl>> descriptor_pools_;
    std::unordered_map<uint64_t, std::unique_ptr<VkDescriptorSetImpl>> descriptor_sets_;
    std::unordered_map<uint64_t, std::unique_ptr<VkCommandPoolImpl>> command_pools_;
    std::unordered_map<uint64_t, std::unique_ptr<VkCommandBufferImpl>> command_buffers_;
    std::unordered_map<uint64_t, std::unique_ptr<VkRenderPassImpl>> render_passes_;
    std::unordered_map<uint64_t, std::unique_ptr<VkFramebufferImpl>> framebuffers_;
    std::unordered_map<uint64_t, std::unique_ptr<VkBufferImpl>> buffers_;
    std::unordered_map<uint64_t, std::unique_ptr<VkImageImpl>> images_;
    std::unordered_map<uint64_t, std::unique_ptr<VkImageViewImpl>> image_views_;
    std::unordered_map<uint64_t, std::unique_ptr<VkSamplerImpl>> samplers_;
    std::unordered_map<uint64_t, std::unique_ptr<VkMemoryImpl>> memories_;
    std::unordered_map<uint64_t, std::unique_ptr<VkFenceImpl>> fences_;
    std::unordered_map<uint64_t, std::unique_ptr<VkSemaphoreImpl>> semaphores_;
    std::unordered_map<uint64_t, std::unique_ptr<VkSwapchainImpl>> swapchains_;
    std::unordered_map<uint64_t, std::unique_ptr<VkQueueImpl>> queues_;

    uint64_t next_handle_ = 1;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKDRIVERDEVICE_H