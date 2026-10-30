#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDPOOL_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDPOOL_H

/*
 * SoftwareCommandPool.h
 *
 * Software command pool: a simple handle holder for command buffers.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class SoftwareCommandPool {
public:
    SoftwareCommandPool(uint32_t queue_family_index) noexcept
        : queue_family_index_(queue_family_index) {}
    explicit SoftwareCommandPool(const KxCommandPoolCreateInfo* create_info) noexcept
        : queue_family_index_(create_info ? create_info->queueFamilyIndex : 0) {}
    ~SoftwareCommandPool() = default;

    uint32_t queue_family_index() const noexcept { return queue_family_index_; }

private:
    uint32_t queue_family_index_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARECOMMANDPOOL_H */