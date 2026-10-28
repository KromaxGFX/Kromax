#include "SoftwareMemory.h"

namespace kx {

SoftwareMemory::SoftwareMemory(uint64_t size, uint32_t memory_type_index,
                               const KxAllocationCallbacks* /*allocator*/) noexcept
    : data_(static_cast<std::size_t>(size)),
      size_(size),
      memory_type_index_(memory_type_index),
      host_visible_(true) {}

} /* namespace kx */