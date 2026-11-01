#ifndef KROMAX_SRC_RUNTIME_IQUEUEOPERATIONS_H
#define KROMAX_SRC_RUNTIME_IQUEUEOPERATIONS_H

/*
 * IQueueOperations.h
 * Queue operations: submit, present, wait.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Queue;
class Fence;
class Semaphore;

class IQueueOperations {
public:
    virtual ~IQueueOperations() = default;

    virtual KxResult getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, Queue** outQueue) = 0;
    virtual KxResult queueSubmit(Queue* queue, uint32_t submitCount, const KxSubmitInfo* pSubmits, Fence fence) = 0;
    virtual KxResult queuePresent(Queue* queue, const KxPresentInfoKHR* pPresentInfo) = 0;
    virtual KxResult queueWaitIdle(Queue* queue) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IQUEUEOPERATIONS_H