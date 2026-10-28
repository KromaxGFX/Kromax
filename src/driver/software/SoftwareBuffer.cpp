#include "SoftwareBuffer.h"
#include "SoftwareMemory.h"

namespace kx {

SoftwareBuffer::SoftwareBuffer(SoftwareMemory* memory, uint64_t offset,
                               uint64_t size, KxBufferUsageFlags usage) noexcept
    : memory_(memory), offset_(offset), size_(size), usage_(usage) {}

} /* namespace kx */