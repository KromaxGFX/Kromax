#ifndef KROMAX_SRC_RUNTIME_IMEMORYMANAGER_H
#define KROMAX_SRC_RUNTIME_IMEMORYMANAGER_H

/*
 * IMemoryManager.h
 * Memory allocation, freeing, mapping, and binding.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Buffer;
class Image;

class IMemoryManager {
public:
    virtual ~IMemoryManager() = default;

    virtual KxResult allocateMemory(const KxMemoryAllocateInfo& info, KxDeviceMemory* pMemory) = 0;
    virtual void freeMemory(KxDeviceMemory memory) = 0;
    virtual KxResult mapMemory(KxDeviceMemory memory, KxDeviceSize offset, KxDeviceSize size, KxMemoryMapFlags flags, void** ppData) = 0;
    virtual void unmapMemory(KxDeviceMemory memory) = 0;
    virtual KxResult bindBufferMemory(Buffer* buffer, KxDeviceMemory memory, KxDeviceSize offset) = 0;
    virtual KxResult bindImageMemory(Image* image, KxDeviceMemory memory, KxDeviceSize offset) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IMEMORYMANAGER_H