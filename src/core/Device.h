#ifndef KROMAX_SRC_CORE_DEVICE_H
#define KROMAX_SRC_CORE_DEVICE_H

/*
 * Device.h
 *
 * Internal Device object. Created from a PhysicalDevice; owns the
 * device-level dispatch table and child objects (memory, buffers, etc.).
 */

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Object.h"
#include "Dispatch.h"
#include "IDriverDevice.h"

namespace kx {

class Queue;
class SoftwareCommandBuffer;

class Device : public Object {
public:
    Device(KxPhysicalDevice physical_device,
           const KxDeviceCreateInfo* create_info,
           const KxAllocationCallbacks* allocator,
           std::unique_ptr<IDriverDevice> driver_device) noexcept;

    ~Device() override = default;

    KxPhysicalDevice physical_device() const noexcept { return physical_device_; }
    const KxAllocationCallbacks* allocator() const noexcept { return allocator_; }

    const DeviceDispatch* dispatch() const noexcept { return &dispatch_; }
    IDriverDevice* driver_device() const noexcept { return driver_device_.get(); }

    /* Static dispatch thunks that forward to the driver device. */
    static KxResult dispatch_allocate_memory(KxDevice device,
                                             const KxMemoryAllocateInfo* info,
                                             const KxAllocationCallbacks* alloc,
                                             KxDeviceMemory* mem);
    static void dispatch_free_memory(KxDevice device, KxDeviceMemory mem,
                                     const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_buffer(KxDevice device,
                                           const KxBufferCreateInfo* info,
                                           const KxAllocationCallbacks* alloc,
                                           KxBuffer* buf);
    static void dispatch_destroy_buffer(KxDevice device, KxBuffer buf,
                                        const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_image(KxDevice device,
                                          const KxImageCreateInfo* info,
                                          const KxAllocationCallbacks* alloc,
                                          KxImage* img);
    static void dispatch_destroy_image(KxDevice device, KxImage img,
                                       const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_image_view(KxDevice device,
                                               const KxImageViewCreateInfo* info,
                                               const KxAllocationCallbacks* alloc,
                                               KxImageView* view);
    static void dispatch_destroy_image_view(KxDevice device, KxImageView view,
                                            const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_render_pass(KxDevice device,
                                                const KxRenderPassCreateInfo* info,
                                                const KxAllocationCallbacks* alloc,
                                                KxRenderPass* rp);
    static void dispatch_destroy_render_pass(KxDevice device, KxRenderPass rp,
                                             const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_framebuffer(KxDevice device,
                                                const KxFramebufferCreateInfo* info,
                                                const KxAllocationCallbacks* alloc,
                                                KxFramebuffer* fb);
    static void dispatch_destroy_framebuffer(KxDevice device, KxFramebuffer fb,
                                             const KxAllocationCallbacks* alloc);
    static KxResult dispatch_create_command_pool(KxDevice device,
                                                 const KxCommandPoolCreateInfo* info,
                                                 const KxAllocationCallbacks* alloc,
                                                 KxCommandPool* pool);
    static void dispatch_destroy_command_pool(KxDevice device, KxCommandPool pool,
                                              const KxAllocationCallbacks* alloc);
    static KxResult dispatch_allocate_command_buffers(KxDevice device,
                                                      const KxCommandBufferAllocateInfo* info,
                                                      KxCommandBuffer* buffers);
    static void dispatch_free_command_buffers(KxDevice device, KxCommandPool pool,
                                              uint32_t count,
                                              const KxCommandBuffer* buffers);
    static KxResult dispatch_begin_command_buffer(KxCommandBuffer cb,
                                                  const KxCommandBufferBeginInfo* info);
    static KxResult dispatch_end_command_buffer(KxCommandBuffer cb);
    static KxResult dispatch_queue_submit(KxQueue queue, uint32_t count,
                                          const KxSubmitInfo* submits, KxFence fence);
    static KxResult dispatch_queue_wait_idle(KxQueue queue);
    static KxResult dispatch_device_wait_idle(KxDevice device);
    static KxResult dispatch_bind_buffer_memory(KxDevice device,
                                                KxBuffer buffer,
                                                KxDeviceMemory memory,
                                                uint64_t memory_offset);
    static KxResult dispatch_bind_image_memory(KxDevice device,
                                               KxImage image,
                                               KxDeviceMemory memory,
                                               uint64_t memory_offset);
    static KxResult dispatch_map_memory(KxDevice device,
                                        KxDeviceMemory memory,
                                        uint64_t offset,
                                        uint64_t size,
                                        KxMemoryMapFlags flags,
                                        void** pp_data);
    static void dispatch_unmap_memory(KxDevice device, KxDeviceMemory memory);

    // Descriptor set layout
    static KxResult dispatch_create_descriptor_set_layout(KxDevice device,
                                                          const KxDescriptorSetLayoutCreateInfo* info,
                                                          const KxAllocationCallbacks* alloc,
                                                          KxDescriptorSetLayout* layout);
    static void dispatch_destroy_descriptor_set_layout(KxDevice device, KxDescriptorSetLayout layout,
                                                        const KxAllocationCallbacks* alloc);

    // Descriptor pool
    static KxResult dispatch_create_descriptor_pool(KxDevice device,
                                                    const KxDescriptorPoolCreateInfo* info,
                                                    const KxAllocationCallbacks* alloc,
                                                    KxDescriptorPool* pool);
    static void dispatch_destroy_descriptor_pool(KxDevice device, KxDescriptorPool pool,
                                                 const KxAllocationCallbacks* alloc);
    static KxResult dispatch_reset_descriptor_pool(KxDevice device, KxDescriptorPool pool,
                                                   KxDescriptorPoolResetFlags flags);

    // Descriptor set
    static KxResult dispatch_allocate_descriptor_sets(KxDevice device,
                                                      const KxDescriptorSetAllocateInfo* info,
                                                      KxDescriptorSet* sets);
    static KxResult dispatch_free_descriptor_sets(KxDevice device, KxDescriptorPool pool,
                                                  uint32_t count, const KxDescriptorSet* sets);
    static void dispatch_update_descriptor_sets(KxDevice device, uint32_t descriptor_write_count,
                                                const KxWriteDescriptorSet* descriptor_writes,
                                                uint32_t descriptor_copy_count,
                                                const KxCopyDescriptorSet* descriptor_copies);

    // Sampler
    static KxResult dispatch_create_sampler(KxDevice device,
                                            const KxSamplerCreateInfo* info,
                                            const KxAllocationCallbacks* alloc,
                                            KxSampler* sampler);
    static void dispatch_destroy_sampler(KxDevice device, KxSampler sampler,
                                         const KxAllocationCallbacks* alloc);

    // Shader module
    static KxResult dispatch_create_shader_module(KxDevice device,
                                                  const KxShaderModuleCreateInfo* info,
                                                  const KxAllocationCallbacks* alloc,
                                                  KxShaderModule* module);
    static void dispatch_destroy_shader_module(KxDevice device, KxShaderModule module,
                                               const KxAllocationCallbacks* alloc);

    // Pipeline layout
    static KxResult dispatch_create_pipeline_layout(KxDevice device,
                                                    const KxPipelineLayoutCreateInfo* info,
                                                    const KxAllocationCallbacks* alloc,
                                                    KxPipelineLayout* layout);
    static void dispatch_destroy_pipeline_layout(KxDevice device, KxPipelineLayout layout,
                                                 const KxAllocationCallbacks* alloc);

    // Pipeline
    static KxResult dispatch_create_pipeline(KxDevice device,
                                             KxPipelineCache pipelineCache,
                                             const KxPipelineCreateInfo* info,
                                             const KxAllocationCallbacks* alloc,
                                             KxPipeline* pipeline);
    static void dispatch_destroy_pipeline(KxDevice device, KxPipeline pipeline,
                                          const KxAllocationCallbacks* alloc);

    SoftwareCommandBuffer* find_command_buffer(KxCommandBuffer cb) const;
    class SoftwareImageView* find_image_view(KxImageView view) const;
    class SoftwareRenderPass* find_render_pass(KxRenderPass rp) const;
    class SoftwareFramebuffer* find_framebuffer(KxFramebuffer fb) const;

    Queue* get_queue(uint32_t family_index, uint32_t index) noexcept;

    bool owns_queue(KxQueue queue) const noexcept;

private:
    KxPhysicalDevice physical_device_;
    const KxAllocationCallbacks* allocator_;
    std::unique_ptr<IDriverDevice> driver_device_;
    DeviceDispatch dispatch_;
    std::vector<std::unique_ptr<Queue>> queues_;
};

/* Lookup a Device from its handle. Defined in api.cpp. */
Device* find_device_locked(KxDevice device);
Device* find_device_locked_by_command_buffer(KxCommandBuffer cb);
Device* find_device_locked_by_queue(KxQueue queue);

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_DEVICE_H */