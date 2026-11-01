#ifndef KX_ASYNC_COMPUTE_H
#define KX_ASYNC_COMPUTE_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Async Compute & Advanced Sync Extension (KX_KHR_async_compute)
 * ======================================================================= */

#define KX_KHR_ASYNC_COMPUTE_EXTENSION_NAME "KX_KHR_async_compute"
#define KX_KHR_ASYNC_COMPUTE_SPEC_VERSION 1

/* Forward declare */
typedef struct KxTimelineSemaphore_T* KxTimelineSemaphore;

/* Timeline semaphore create info */
typedef struct KxTimelineSemaphoreCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint64_t initialValue;
} KxTimelineSemaphoreCreateInfo;

/* Semaphore signal info */
typedef struct KxSemaphoreSignalInfo {
    KxStructureType sType;
    const void* pNext;
    KxSemaphore semaphore;
    uint64_t value;
} KxSemaphoreSignalInfo;

/* Semaphore wait info */
typedef struct KxSemaphoreWaitInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    uint32_t semaphoreCount;
    const KxSemaphore* pSemaphores;
    const uint64_t* pValues;
} KxSemaphoreWaitInfo;

/* =======================================================================
 * Timeline semaphore functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateTimelineSemaphore(
    KxDevice device,
    const KxTimelineSemaphoreCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxTimelineSemaphore* pTimelineSemaphore);

KX_API void KX_CALL kxDestroyTimelineSemaphore(
    KxDevice device,
    KxTimelineSemaphore timelineSemaphore,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxSignalSemaphore(
    KxDevice device,
    const KxSemaphoreSignalInfo* pSignalInfo);

KX_API KxResult KX_CALL kxWaitSemaphores(
    KxDevice device,
    const KxSemaphoreWaitInfo* pWaitInfo);

KX_API KxResult KX_CALL kxGetSemaphoreCounterValue(
    KxDevice device,
    KxSemaphore semaphore,
    uint64_t* pValue);

/* =======================================================================
 * Sparse Resources (KX_EXT_sparse_resources)
 * ======================================================================= */

typedef struct KxSparseMemoryBind {
    KxDeviceMemory memory;
    uint64_t memoryOffset;
    uint64_t size;
    uint32_t flags;
} KxSparseMemoryBind;

KX_API KxResult KX_CALL kxBindSparseMemory(
    KxDevice device,
    uint32_t bindCount,
    const void* pBinds);

/* =======================================================================
 * Multi-Queue (KX_KHR_device_group)
 * ======================================================================= */

typedef struct KxDeviceQueueInfo2 {
    KxStructureType sType;
    const void* pNext;
    KxQueueFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
} KxDeviceQueueInfo2;

KX_API KxResult KX_CALL kxGetDeviceQueue2(
    KxDevice device,
    const KxDeviceQueueInfo2* pQueueInfo,
    KxQueue* pQueue);

KX_API KxResult KX_CALL kxCmdUpdateBuffer(
    KxCommandBuffer commandBuffer,
    KxBuffer dstBuffer,
    uint64_t dstOffset,
    uint64_t size,
    const void* pData);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_ASYNC_COMPUTE_H */