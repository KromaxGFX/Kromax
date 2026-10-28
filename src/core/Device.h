#ifndef KROMAX_SRC_CORE_DEVICE_H
#define KROMAX_SRC_CORE_DEVICE_H

/*
 * Device.h
 *
 * Internal Device object. Created from a PhysicalDevice; owns the
 * device-level dispatch table and child objects (memory, buffers, etc.).
 */

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Object.h"

namespace kx {

class Queue;

class Device : public Object {
public:
    Device(KxPhysicalDevice physical_device,
           const KxDeviceCreateInfo* create_info,
           const KxAllocationCallbacks* allocator) noexcept;

    ~Device() override = default;

    KxPhysicalDevice physical_device() const noexcept { return physical_device_; }
    const KxAllocationCallbacks* allocator() const noexcept { return allocator_; }

    const DeviceDispatch* dispatch() const noexcept { return &dispatch_; }

private:
    KxPhysicalDevice physical_device_;
    const KxAllocationCallbacks* allocator_;
    DeviceDispatch dispatch_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_DEVICE_H */