#ifndef KROMAX_SRC_CORE_PHYSICALDEVICE_H
#define KROMAX_SRC_CORE_PHYSICALDEVICE_H

/*
 * PhysicalDevice.h
 *
 * Internal PhysicalDevice object. Wraps a driver-visible device and
 * exposes its properties, features, memory heaps, and queue families.
 */

#include <vector>

#include "Object.h"

namespace kx {

class PhysicalDevice : public Object {
public:
    PhysicalDevice() noexcept
        : Object(ObjectType::PhysicalDevice) {}

    const KxPhysicalDeviceProperties& properties() const noexcept {
        return properties_;
    }
    void set_properties(const KxPhysicalDeviceProperties& props) noexcept {
        properties_ = props;
    }

    const KxPhysicalDeviceFeatures& features() const noexcept {
        return features_;
    }
    void set_features(const KxPhysicalDeviceFeatures& feats) noexcept {
        features_ = feats;
    }

    const std::vector<KxQueueFamilyProperties>& queue_families() const noexcept {
        return queue_families_;
    }
    void set_queue_families(
        const std::vector<KxQueueFamilyProperties>& families) noexcept {
        queue_families_ = families;
    }

    const KxPhysicalDeviceMemoryProperties& memory_properties() const noexcept {
        return memory_properties_;
    }
    void set_memory_properties(
        const KxPhysicalDeviceMemoryProperties& props) noexcept {
        memory_properties_ = props;
    }

private:
    KxPhysicalDeviceProperties properties_;
    KxPhysicalDeviceFeatures features_;
    std::vector<KxQueueFamilyProperties> queue_families_;
    KxPhysicalDeviceMemoryProperties memory_properties_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_PHYSICALDEVICE_H */