#ifndef KROMAX_SRC_RUNTIME_ICOMMANDFACTORY_H
#define KROMAX_SRC_RUNTIME_ICOMMANDFACTORY_H

/*
 * ICommandFactory.h
 * Command pool and command buffer management.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class CommandPool;
class CommandBuffer;

struct CommandPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    KxCommandPoolCreateFlags flags = 0;
    uint32_t queueFamilyIndex = 0;
};

struct CommandBufferAllocateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    const void* pNext = nullptr;
    CommandPool commandPool = nullptr;
    KxCommandBufferLevel level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    uint32_t commandBufferCount = 1;
};

class ICommandFactory {
public:
    virtual ~ICommandFactory() = default;

    virtual KxResult createCommandPool(const CommandPoolCreateInfo& info, CommandPool** outPool) = 0;
    virtual void destroyCommandPool(CommandPool pool) = 0;
    virtual KxResult allocateCommandBuffers(const CommandBufferAllocateInfo& info, CommandBuffer* pCommandBuffers) = 0;
    virtual void freeCommandBuffers(CommandPool pool, uint32_t commandBufferCount, const CommandBuffer* pCommandBuffers) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ICOMMANDFACTORY_H