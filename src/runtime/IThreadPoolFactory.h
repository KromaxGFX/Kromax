#ifndef KROMAX_SRC_RUNTIME_ITHREADPOOLFACTORY_H
#define KROMAX_SRC_RUNTIME_ITHREADPOOLFACTORY_H

/*
 * IThreadPoolFactory.h
 * Thread pool management.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class ThreadPool;

struct ThreadPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_THREAD_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    uint32_t threadCount = 0;
    uint32_t flags = 0;
};

class IThreadPoolFactory {
public:
    virtual ~IThreadPoolFactory() = default;

    virtual KxResult createThreadPool(const ThreadPoolCreateInfo& info, ThreadPool** outThreadPool) = 0;
    virtual void destroyThreadPool(ThreadPool* threadPool) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ITHREADPOOLFACTORY_H