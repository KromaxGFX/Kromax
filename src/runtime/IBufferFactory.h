#ifndef KROMAX_SRC_RUNTIME_IBUFFERFACTORY_H
#define KROMAX_SRC_RUNTIME_IBUFFERFACTORY_H

/*
 * IBufferFactory.h
 * Buffer creation, destruction, and memory requirements.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Buffer;

struct BufferCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    KxBufferCreateFlags flags = 0;
    KxDeviceSize size = 0;
    KxBufferUsageFlags usage = 0;
    KxSharingMode sharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
};

class IBufferFactory {
public:
    virtual ~IBufferFactory() = default;

    virtual KxResult createBuffer(const BufferCreateInfo& info, Buffer** outBuffer) = 0;
    virtual void destroyBuffer(Buffer* buffer) = 0;
    virtual KxResult getBufferMemoryRequirements(Buffer* buffer, KxMemoryRequirements* pRequirements) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IBUFFERFACTORY_H