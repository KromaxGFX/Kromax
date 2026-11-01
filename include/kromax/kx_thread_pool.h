#ifndef KX_THREAD_POOL_H
#define KX_THREAD_POOL_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Thread Pool Extension (KX_EXT_thread_pool)
 * ======================================================================= */

#define KX_EXT_THREAD_POOL_EXTENSION_NAME "KX_EXT_thread_pool"
#define KX_EXT_THREAD_POOL_SPEC_VERSION 1

typedef struct KxThreadPool_T* KxThreadPool;

/* Thread pool create flags */
typedef uint32_t KxThreadPoolCreateFlags;
#define KX_THREAD_POOL_CREATE_NONE 0u
#define KX_THREAD_POOL_CREATE_MAX_ENUM 0x7FFFFFFFu

/* Thread pool create info */
typedef struct KxThreadPoolCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxThreadPoolCreateFlags flags;
    uint32_t threadCount;  /* 0 = hardware_concurrency() */
} KxThreadPoolCreateInfo;

/* Create a thread pool for parallel command recording and rasterization */
KX_API KxResult KX_CALL kxCreateThreadPool(
    KxDevice device,
    const KxThreadPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxThreadPool* pThreadPool);

/* Destroy a thread pool */
KX_API void KX_CALL kxDestroyThreadPool(
    KxDevice device,
    KxThreadPool threadPool,
    const KxAllocationCallbacks* pAllocator);

/* Set thread pool on command buffer for parallel recording */
KX_API void KX_CALL kxCmdSetThreadPool(
    KxCommandBuffer commandBuffer,
    KxThreadPool threadPool);

/* Get thread pool statistics */
typedef struct KxThreadPoolStats {
    uint32_t threadCount;
    uint32_t activeThreads;
    uint64_t tasksCompleted;
    uint64_t tasksQueued;
} KxThreadPoolStats;

KX_API KxResult KX_CALL kxGetThreadPoolStats(
    KxThreadPool threadPool,
    KxThreadPoolStats* pStats);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_THREAD_POOL_H */