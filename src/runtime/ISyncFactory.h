#ifndef KROMAX_SRC_RUNTIME_ISYNCFACTORY_H
#define KROMAX_SRC_RUNTIME_ISYNCFACTORY_H

/*
 * ISyncFactory.h
 * Synchronization primitives: semaphores and fences.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Semaphore;
class Fence;

struct SemaphoreCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    const void* pNext = nullptr;
    KxSemaphoreCreateFlags flags = 0;
};

class ISyncFactory {
public:
    virtual ~ISyncFactory() = default;

    virtual KxResult createSemaphore(const SemaphoreCreateInfo& info, Semaphore** outSemaphore) = 0;
    virtual void destroySemaphore(Semaphore semaphore) = 0;
    virtual KxResult createFence(const KxFenceCreateInfo& info, Fence** outFence) = 0;
    virtual void destroyFence(Fence fence) = 0;
    virtual KxResult waitForFences(uint32_t fenceCount, const Fence* pFences, KxBool32 waitAll, uint64_t timeout) = 0;
    virtual KxResult resetFences(uint32_t fenceCount, const Fence* pFences) = 0;
    virtual KxResult signalSemaphore(Semaphore semaphore, uint64_t value) = 0;
    virtual KxResult waitSemaphores(uint32_t semaphoreCount, const Semaphore* pSemaphores, const uint64_t* pValues, KxBool32 waitAll, uint64_t timeout) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISYNCFACTORY_H