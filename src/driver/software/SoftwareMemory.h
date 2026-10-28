#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMEMORY_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMEMORY_H

/*
 * SoftwareMemory.h
 *
 * Software implementation of a device memory allocation: a system heap
 * block with optional host mapping.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareMemory {
public:
    SoftwareMemory(uint64_t size, uint32_t memory_type_index,
                   const KxAllocationCallbacks* allocator) noexcept;
    ~SoftwareMemory() = default;

    SoftwareMemory(const SoftwareMemory&) = delete;
    SoftwareMemory& operator=(const SoftwareMemory&) = delete;

    uint8_t* data() noexcept { return data_.data(); }
    const uint8_t* data() const noexcept { return data_.data(); }
    uint64_t size() const noexcept { return size_; }
    uint32_t memory_type_index() const noexcept { return memory_type_index_; }
    bool host_visible() const noexcept { return host_visible_; }

private:
    std::vector<uint8_t> data_;
    uint64_t size_;
    uint32_t memory_type_index_;
    bool host_visible_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREMEMORY_H */