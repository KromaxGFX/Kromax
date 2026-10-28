#include "Device.h"

namespace kx {

Device::Device(KxPhysicalDevice physical_device,
               const KxDeviceCreateInfo* create_info,
               const KxAllocationCallbacks* allocator) noexcept
    : Object(ObjectType::Device),
      physical_device_(physical_device),
      allocator_(allocator) {
    /* Zero-init the device dispatch table; filled in by the driver. */
    dispatch_.allocate_memory = nullptr;
    dispatch_.free_memory = nullptr;
    dispatch_.create_buffer = nullptr;
    dispatch_.destroy_buffer = nullptr;
    dispatch_.create_image = nullptr;
    dispatch_.destroy_image = nullptr;
    dispatch_.create_command_pool = nullptr;
    dispatch_.destroy_command_pool = nullptr;
    dispatch_.allocate_command_buffers = nullptr;
    dispatch_.free_command_buffers = nullptr;
    dispatch_.begin_command_buffer = nullptr;
    dispatch_.end_command_buffer = nullptr;
    dispatch_.queue_submit = nullptr;
    dispatch_.queue_wait_idle = nullptr;
    dispatch_.device_wait_idle = nullptr;
}

} /* namespace kx */