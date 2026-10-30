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

    /* Object registration */
    KxDeviceMemory register_memory(std::unique_ptr<SoftwareMemory> mem);
    KxBuffer register_buffer(std::unique_ptr<SoftwareBuffer> buf);
    KxImage register_image(std::unique_ptr<SoftwareImage> img);
    KxImageView register_image_view(std::unique_ptr<SoftwareImageView> view);
    KxRenderPass register_render_pass(std::unique_ptr<SoftwareRenderPass> rp);
    KxFramebuffer register_framebuffer(std::unique_ptr<SoftwareFramebuffer> fb);
    KxCommandPool register_command_pool(std::unique_ptr<SoftwareCommandPool> pool);
    KxCommandBuffer register_command_buffer(
        std::unique_ptr<SoftwareCommandBuffer> cmd);
    KxPipelineLayout register_pipeline_layout(std::unique_ptr<SoftwarePipelineLayout> layout);
    KxPipeline register_pipeline(std::unique_ptr<SoftwarePipeline> pipeline);
    KxShaderModule register_shader_module(std::unique_ptr<SoftwareShaderModule> module);

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

    /* IDriverDevice */
    KxResult allocate_memory(
        const KxMemoryAllocateInfo* allocate_info,
        const KxAllocationCallbacks* allocator,
        KxDeviceMemory* memory) override;
    void free_memory(KxDeviceMemory memory,
                     const KxAllocationCallbacks* allocator) override;
    KxResult create_buffer(
        const KxBufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxBuffer* buffer) override;
    void destroy_buffer(KxBuffer buffer,
                        const KxAllocationCallbacks* allocator) override;
    KxResult create_image(
        const KxImageCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImage* image) override;
    void destroy_image(KxImage image,
                       const KxAllocationCallbacks* allocator) override;
    KxResult create_image_view(
        const KxImageViewCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImageView* image_view) override;
    void destroy_image_view(KxImageView image_view,
                            const KxAllocationCallbacks* allocator) override;
    KxResult create_render_pass(
        const KxRenderPassCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxRenderPass* render_pass) override;
    void destroy_render_pass(KxRenderPass render_pass,
                             const KxAllocationCallbacks* allocator) override;
    KxResult create_framebuffer(
        const KxFramebufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxFramebuffer* framebuffer) override;
    void destroy_framebuffer(KxFramebuffer framebuffer,
                             const KxAllocationCallbacks* allocator) override;
    KxResult create_command_pool(
        const KxCommandPoolCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxCommandPool* command_pool) override;
    void destroy_command_pool(KxCommandPool command_pool,
                              const KxAllocationCallbacks* allocator) override;
    KxResult allocate_command_buffers(
        const KxCommandBufferAllocateInfo* allocate_info,
        KxCommandBuffer* command_buffers) override;
    void free_command_buffers(KxCommandPool command_pool,
                              uint32_t count,
                              const KxCommandBuffer* command_buffers) override;
    KxResult begin_command_buffer(
        KxCommandBuffer command_buffer,
        const KxCommandBufferBeginInfo* begin_info) override;
    KxResult end_command_buffer(KxCommandBuffer command_buffer) override;
    KxResult queue_submit(uint32_t submit_count,
                          const KxSubmitInfo* submits,
                          KxFence fence) override;
    KxResult queue_wait_idle() override;
    KxResult device_wait_idle() override;
    KxResult bind_buffer_memory(
        KxBuffer buffer,
        KxDeviceMemory memory,
        uint64_t memory_offset) override;
    KxResult bind_image_memory(
        KxImage image,
        KxDeviceMemory memory,
        uint64_t memory_offset) override;
    KxResult map_memory(
        KxDeviceMemory memory,
        uint64_t offset,
        uint64_t size,
        KxMemoryMapFlags flags,
        void** pp_data) override;
    void unmap_memory(
        KxDeviceMemory memory) override;

    /* Pipeline */
    KxResult create_pipeline_layout(
        const KxPipelineLayoutCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxPipelineLayout* pipeline_layout) override;
    void destroy_pipeline_layout(KxPipelineLayout pipeline_layout,
                                 const KxAllocationCallbacks* allocator) override;
    KxResult create_pipeline(
        const KxPipelineCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxPipeline* pipeline) override;
    void destroy_pipeline(KxPipeline pipeline,
                          const KxAllocationCallbacks* allocator) override;

    /* Shader module */
    KxResult create_shader_module(
        const KxShaderModuleCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxShaderModule* shader_module) override;
    void destroy_shader_module(KxShaderModule shader_module,
                               const KxAllocationCallbacks* allocator) override;

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
    static uint32_t format_pixel_size(KxFormat format);

    // Current pipeline state for draw commands
    SoftwarePipeline* current_pipeline_ = nullptr;
    CmdBindVertexBuffers current_vertex_buffers_{};
    CmdBindIndexBuffer current_index_buffer_{};

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
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H */