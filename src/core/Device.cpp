#include "Device.h"
#include "SoftwareDevice.h"
#include "SoftwareImageView.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"
#include "Queue.h"

namespace kx {

Device::Device(KxPhysicalDevice physical_device,
               const KxDeviceCreateInfo* create_info,
               const KxAllocationCallbacks* allocator,
               std::unique_ptr<IDriverDevice> driver_device) noexcept
    : Object(ObjectType::Device),
      physical_device_(physical_device),
      allocator_(allocator),
      driver_device_(std::move(driver_device)) {
    /* Fill the device dispatch table with static thunks. */
    dispatch_.allocate_memory = &Device::dispatch_allocate_memory;
    dispatch_.free_memory = &Device::dispatch_free_memory;
    dispatch_.create_buffer = &Device::dispatch_create_buffer;
    dispatch_.destroy_buffer = &Device::dispatch_destroy_buffer;
    dispatch_.create_image = &Device::dispatch_create_image;
    dispatch_.destroy_image = &Device::dispatch_destroy_image;
    dispatch_.create_image_view = &Device::dispatch_create_image_view;
    dispatch_.destroy_image_view = &Device::dispatch_destroy_image_view;
    dispatch_.create_render_pass = &Device::dispatch_create_render_pass;
    dispatch_.destroy_render_pass = &Device::dispatch_destroy_render_pass;
    dispatch_.create_framebuffer = &Device::dispatch_create_framebuffer;
    dispatch_.destroy_framebuffer = &Device::dispatch_destroy_framebuffer;
    dispatch_.create_command_pool = &Device::dispatch_create_command_pool;
    dispatch_.destroy_command_pool = &Device::dispatch_destroy_command_pool;
    dispatch_.allocate_command_buffers = &Device::dispatch_allocate_command_buffers;
    dispatch_.free_command_buffers = &Device::dispatch_free_command_buffers;
    dispatch_.begin_command_buffer = &Device::dispatch_begin_command_buffer;
    dispatch_.end_command_buffer = &Device::dispatch_end_command_buffer;
    dispatch_.queue_submit = &Device::dispatch_queue_submit;
    dispatch_.queue_wait_idle = &Device::dispatch_queue_wait_idle;
    dispatch_.device_wait_idle = &Device::dispatch_device_wait_idle;
    dispatch_.bind_buffer_memory = &Device::dispatch_bind_buffer_memory;
    dispatch_.bind_image_memory = &Device::dispatch_bind_image_memory;
    dispatch_.map_memory = &Device::dispatch_map_memory;
    dispatch_.unmap_memory = &Device::dispatch_unmap_memory;
    dispatch_.create_shader_module = &Device::dispatch_create_shader_module;
    dispatch_.destroy_shader_module = &Device::dispatch_destroy_shader_module;
    dispatch_.create_pipeline_layout = &Device::dispatch_create_pipeline_layout;
    dispatch_.destroy_pipeline_layout = &Device::dispatch_destroy_pipeline_layout;
    dispatch_.create_pipeline = &Device::dispatch_create_pipeline;
    dispatch_.destroy_pipeline = &Device::dispatch_destroy_pipeline;
}

SoftwareCommandBuffer* Device::find_command_buffer(KxCommandBuffer cb) const {
    return static_cast<SoftwareDevice*>(driver_device_.get())->find_command_buffer(cb);
}

SoftwareImageView* Device::find_image_view(KxImageView view) const {
    return static_cast<SoftwareDevice*>(driver_device_.get())->find_image_view(view);
}

SoftwareRenderPass* Device::find_render_pass(KxRenderPass rp) const {
    return static_cast<SoftwareDevice*>(driver_device_.get())->find_render_pass(rp);
}

SoftwareFramebuffer* Device::find_framebuffer(KxFramebuffer fb) const {
    return static_cast<SoftwareDevice*>(driver_device_.get())->find_framebuffer(fb);
}

Queue* Device::get_queue(uint32_t family_index, uint32_t index) noexcept {
    // Create queue if it doesn't exist (simple approach: create on demand)
    for (auto& q : queues_) {
        if (q->family_index() == family_index && q->index() == index) {
            return q.get();
        }
    }
    auto queue = std::make_unique<Queue>(family_index, index);
    Queue* ptr = queue.get();
    queues_.push_back(std::move(queue));
    return ptr;
}

bool Device::owns_queue(KxQueue queue) const noexcept {
    for (auto& q : queues_) {
        if (reinterpret_cast<KxQueue>(q.get()) == queue) return true;
    }
    return false;
}

} /* namespace kx */