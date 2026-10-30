/*
 * api_command.cpp
 *
 * Command pool, Command buffer, Queue submit C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Command pools
 * ======================================================================= */

KxResult KX_CALL kxCreateCommandPool(
    KxDevice device,
    const KxCommandPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxCommandPool* pCommandPool) {
    if (!pCommandPool) return KX_ERROR_INVALID_ARGUMENT;
    *pCommandPool = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_command_pool)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_command_pool(device, pCreateInfo, pAllocator,
                                                pCommandPool);
}

void KX_CALL kxDestroyCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_command_pool)
        return;
    dev->dispatch()->destroy_command_pool(device, commandPool, pAllocator);
}

KxResult KX_CALL kxResetCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    KxCommandPoolResetFlags flags) {
    (void)device; (void)commandPool; (void)flags;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* =======================================================================
 * Command buffers
 * ======================================================================= */

KxResult KX_CALL kxAllocateCommandBuffers(
    KxDevice device,
    const KxCommandBufferAllocateInfo* pAllocateInfo,
    KxCommandBuffer* pCommandBuffers) {
    if (!pCommandBuffers) return KX_ERROR_INVALID_ARGUMENT;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->allocate_command_buffers)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->allocate_command_buffers(device, pAllocateInfo,
                                                     pCommandBuffers);
}

void KX_CALL kxFreeCommandBuffers(
    KxDevice device,
    KxCommandPool commandPool,
    uint32_t commandBufferCount,
    const KxCommandBuffer* pCommandBuffers) {
    (void)device; (void)commandPool; (void)commandBufferCount;
    (void)pCommandBuffers;
}

KxResult KX_CALL kxBeginCommandBuffer(
    KxCommandBuffer commandBuffer,
    const KxCommandBufferBeginInfo* pBeginInfo) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->begin_command_buffer)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->begin_command_buffer(commandBuffer, pBeginInfo);
}

KxResult KX_CALL kxEndCommandBuffer(KxCommandBuffer commandBuffer) {
    kx::Device* dev = kx::find_device_locked_by_command_buffer(commandBuffer);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->end_command_buffer)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->end_command_buffer(commandBuffer);
}

KxResult KX_CALL kxResetCommandBuffer(
    KxCommandBuffer commandBuffer,
    KxCommandBufferResetFlags flags) {
    (void)commandBuffer; (void)flags;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* =======================================================================
 * Fences / semaphores
 * ======================================================================= */

KxResult KX_CALL kxCreateFence(
    KxDevice device,
    const KxFenceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxFence* pFence) {
    (void)device; (void)pCreateInfo; (void)pAllocator; (void)pFence;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroyFence(
    KxDevice device,
    KxFence fence,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)fence; (void)pAllocator;
}

KxResult KX_CALL kxWaitForFences(
    KxDevice device,
    uint32_t fenceCount,
    const KxFence* pFences,
    KxBool32 waitAll,
    uint64_t timeout) {
    (void)device; (void)fenceCount; (void)pFences; (void)waitAll;
    (void)timeout;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxResetFences(
    KxDevice device,
    uint32_t fenceCount,
    const KxFence* pFences) {
    (void)device; (void)fenceCount; (void)pFences;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxGetFenceStatus(KxDevice device, KxFence fence) {
    (void)device; (void)fence;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxCreateSemaphore(
    KxDevice device,
    const KxSemaphoreCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSemaphore* pSemaphore) {
    (void)device; (void)pCreateInfo; (void)pAllocator; (void)pSemaphore;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroySemaphore(
    KxDevice device,
    KxSemaphore semaphore,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)semaphore; (void)pAllocator;
}

/* =======================================================================
 * Queue submit
 * ======================================================================= */

KxResult KX_CALL kxQueueSubmit(
    KxQueue queue,
    uint32_t submitCount,
    const KxSubmitInfo* pSubmits,
    KxFence fence) {
    kx::Device* dev = kx::find_device_locked_by_queue(queue);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->queue_submit)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->queue_submit(queue, submitCount, pSubmits, fence);
}

KxResult KX_CALL kxQueueWaitIdle(KxQueue queue) {
    (void)queue;
    // For software driver, queue operations are synchronous, so wait_idle is a no-op
    return KX_SUCCESS;
}

KxResult KX_CALL kxDeviceWaitIdle(KxDevice device) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->device_wait_idle)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->device_wait_idle(device);
}

#ifdef __cplusplus
} /* extern "C" */
#endif