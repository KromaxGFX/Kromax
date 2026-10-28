#include "Instance.h"

#include <cstring>

#include "DriverRegistry.h"

namespace kx {

Instance::Instance(const KxInstanceCreateInfo* create_info,
                   const KxAllocationCallbacks* allocator) noexcept
    : Object(ObjectType::Instance),
      allocator_(allocator),
      alloc_callbacks_(allocator) {
    if (create_info) {
        if (create_info->pApplicationInfo) {
            log_info("Kromax instance created");
        }
        for (uint32_t i = 0; i < create_info->enabledExtensionCount; ++i) {
            enabled_extensions_.push_back(create_info->ppEnabledExtensionNames[i]);
        }
        for (uint32_t i = 0; i < create_info->enabledLayerCount; ++i) {
            enabled_layers_.push_back(create_info->ppEnabledLayerNames[i]);
        }
    }

    /* Build the default (software-only) dispatch table. */
    dispatch_.create_device = nullptr;
    dispatch_.destroy_device = nullptr;
    dispatch_.enumerate_physical_devices = nullptr;
    dispatch_.get_proc_addr = nullptr;
}

bool Instance::extension_enabled(const char* name) const noexcept {
    if (!name) return false;
    for (const auto& ext : enabled_extensions_) {
        if (ext == name) return true;
    }
    return false;
}

} /* namespace kx */