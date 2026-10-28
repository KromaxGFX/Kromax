#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREBUFFER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREBUFFER_H

/*
 * SoftwareBuffer.h
 *
 * Software buffer: a slice of a SoftwareMemory allocation.
 */

#include "kx_core.h"

namespace kx {

class SoftwareMemory;

class SoftwareBuffer {
public:
    SoftwareBuffer(SoftwareMemory* memory, uint64_t offset, uint64_t size,
                   KxBufferUsageFlags usage) noexcept;

    SoftwareMemory* memory() const noexcept { return memory_; }
    uint64_t offset() const noexcept { return offset_; }
    uint64_t size() const noexcept { return size_; }
    KxBufferUsageFlags usage() const noexcept { return usage_; }
    uint8_t* mapped_ptr() const noexcept {
        return memory_ ? memory_->data() + offset_ : nullptr;
    }

private:
    SoftwareMemory* memory_;
    uint64_t offset_;
    uint64_t size_;
    KxBufferUsageFlags usage_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREBUFFER_H */