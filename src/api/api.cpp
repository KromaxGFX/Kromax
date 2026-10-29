/*
 * api.cpp
 *
 * Public C API implementation. Thin layer: validates arguments, translates
 * opaque handles to internal kx:: objects, forwards to dispatch tables,
 * and returns KxResult.
 */

#include "api.h"

#include <cstring>
#include <utility>

#include "IDriver.h"
#include "DriverRegistry.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "Queue.h"

#include "SoftwareDriver.h"

namespace kx {

/* Global registry definitions */
std::vector<std::unique_ptr<Instance>>& g_instances() {
    static std::vector<std::unique_ptr<Instance>> instances;
    return instances;
}

std::vector<std::unique_ptr<PhysicalDevice>>& g_physical_devices() {
    static std::vector<std::unique_ptr<PhysicalDevice>> physical_devices;
    return physical_devices;
}

std::vector<std::unique_ptr<Device>>& g_devices() {
    static std::vector<std::unique_ptr<Device>> devices;
    return devices;
}

Instance* find_instance_locked(KxInstance instance) {
    for (auto& inst : g_instances()) {
        if (reinterpret_cast<KxInstance>(inst.get()) == instance)
            return inst.get();
    }
    return nullptr;
}

Device* find_device_locked(KxDevice device) {
    for (auto& dev : g_devices()) {
        if (reinterpret_cast<KxDevice>(dev.get()) == device) return dev.get();
    }
    return nullptr;
}

PhysicalDevice* find_physical_device_locked(KxPhysicalDevice pd) {
    for (auto& p : g_physical_devices()) {
        if (reinterpret_cast<KxPhysicalDevice>(p.get()) == pd) return p.get();
    }
    return nullptr;
}

} /* namespace kx */

/* =======================================================================
 * Public C API functions (extern "C" linkage)
 * ======================================================================= */

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Instance
 * ======================================================================= */

KxResult KX_CALL kxCreateInstance(
    const KxInstanceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxInstance* pInstance) {
    if (!pInstance) return KX_ERROR_INVALID_ARGUMENT;
    *pInstance = nullptr;
    if (!pCreateInfo) return KX_ERROR_INVALID_ARGUMENT;
    if (pCreateInfo->sType != KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    /* Register built-in drivers (static registration). */
    kx::DriverRegistry::instance().register_driver(
        std::make_unique<kx::SoftwareDriver>());

    auto inst = std::make_unique<kx::Instance>(pCreateInfo, pAllocator);
    KxInstance handle = reinterpret_cast<KxInstance>(inst.get());
    kx::g_instances().push_back(std::move(inst));
    *pInstance = handle;
    return KX_SUCCESS;
}

void KX_CALL kxDestroyInstance(
    KxInstance instance,
    const KxAllocationCallbacks* /*pAllocator*/) {
    if (!instance) return;
    for (auto it = kx::g_instances().begin(); it != kx::g_instances().end(); ++it) {
        if (reinterpret_cast<KxInstance>(it->get()) == instance) {
            kx::g_instances().erase(it);
            return;
        }
    }
}

/* =======================================================================
 * Physical devices
 * ======================================================================= */

KxResult KX_CALL kxEnumeratePhysicalDevices(
    KxInstance instance,
    uint32_t* pPhysicalDeviceCount,
    KxPhysicalDevice* pPhysicalDevices) {
    if (!pPhysicalDeviceCount) return KX_ERROR_INVALID_ARGUMENT;

    kx::Instance* inst = kx::find_instance_locked(instance);
    if (!inst) return KX_ERROR_INVALID_ARGUMENT;

    auto& registry = kx::DriverRegistry::instance();

    uint32_t total = 0;
    for (auto& driver : registry.drivers()) {
        auto result = driver->enumerate_physical_devices(inst);
        if (!result.has_value()) continue;
        total += static_cast<uint32_t>(result.value().size());
    }

    if (!pPhysicalDevices) {
        *pPhysicalDeviceCount = total;
        return KX_SUCCESS;
    }

    uint32_t count = *pPhysicalDeviceCount;
    uint32_t written = 0;
    for (auto& driver : registry.drivers()) {
        auto result = driver->enumerate_physical_devices(inst);
        if (!result.has_value()) continue;
        for (auto& phys : result.value()) {
            if (written >= count) break;
            pPhysicalDevices[written] =
                reinterpret_cast<KxPhysicalDevice>(phys.get());
            kx::g_physical_devices().push_back(std::move(phys));
            ++written;
        }
        if (written >= count) break;
    }

    *pPhysicalDeviceCount = written;
    return KX_SUCCESS;
}

KxResult KX_CALL kxGetPhysicalDeviceProperties(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceProperties* pProperties) {
    if (!pProperties) return KX_ERROR_INVALID_ARGUMENT;
    kx::PhysicalDevice* pd = kx::find_physical_device_locked(physicalDevice);
    if (!pd) return KX_ERROR_INVALID_ARGUMENT;
    *pProperties = pd->properties();
    return KX_SUCCESS;
}

KxResult KX_CALL kxGetPhysicalDeviceFeatures(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceFeatures* pFeatures) {
    if (!pFeatures) return KX_ERROR_INVALID_ARGUMENT;
    kx::PhysicalDevice* pd = kx::find_physical_device_locked(physicalDevice);
    if (!pd) return KX_ERROR_INVALID_ARGUMENT;
    *pFeatures = pd->features();
    return KX_SUCCESS;
}

KxResult KX_CALL kxGetPhysicalDeviceMemoryProperties(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceMemoryProperties* pMemoryProperties) {
    if (!pMemoryProperties) return KX_ERROR_INVALID_ARGUMENT;
    kx::PhysicalDevice* pd = kx::find_physical_device_locked(physicalDevice);
    if (!pd) return KX_ERROR_INVALID_ARGUMENT;
    *pMemoryProperties = pd->memory_properties();
    return KX_SUCCESS;
}

KxResult KX_CALL kxGetPhysicalDeviceQueueFamilyProperties(
    KxPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    KxQueueFamilyProperties* pQueueFamilyProperties) {
    if (!pQueueFamilyPropertyCount) return KX_ERROR_INVALID_ARGUMENT;
    kx::PhysicalDevice* pd = kx::find_physical_device_locked(physicalDevice);
    if (!pd) return KX_ERROR_INVALID_ARGUMENT;

    const auto& families = pd->queue_families();
    uint32_t count = static_cast<uint32_t>(families.size());

    if (!pQueueFamilyProperties) {
        *pQueueFamilyPropertyCount = count;
        return KX_SUCCESS;
    }

    uint32_t available = *pQueueFamilyPropertyCount;
    uint32_t to_copy = (available < count) ? available : count;
    for (uint32_t i = 0; i < to_copy; ++i) {
        pQueueFamilyProperties[i] = families[i];
    }
    *pQueueFamilyPropertyCount = to_copy;
    return KX_SUCCESS;
}

/* =======================================================================
 * Device
 * ======================================================================= */

KxResult KX_CALL kxCreateDevice(
    KxPhysicalDevice physicalDevice,
    const KxDeviceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDevice* pDevice) {
    if (!pDevice) return KX_ERROR_INVALID_ARGUMENT;
    *pDevice = nullptr;
    if (!pCreateInfo) return KX_ERROR_INVALID_ARGUMENT;
    if (pCreateInfo->sType != KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    kx::PhysicalDevice* pd = kx::find_physical_device_locked(physicalDevice);
    if (!pd) return KX_ERROR_INVALID_ARGUMENT;

    auto& registry = kx::DriverRegistry::instance();
    kx::IDriver* owner = nullptr;
    for (auto& driver : registry.drivers()) {
        owner = driver.get();
        break;
    }
    if (!owner) return KX_ERROR_INITIALIZATION_FAILED;

    auto result = owner->create_device(pd, pCreateInfo);
    if (!result.has_value()) return result.error();

    auto driver_device = std::move(result.value());

    auto dev = std::make_unique<kx::Device>(
        physicalDevice, pCreateInfo, pAllocator, std::move(driver_device));
    KxDevice handle = reinterpret_cast<KxDevice>(dev.get());
    kx::g_devices().push_back(std::move(dev));
    *pDevice = handle;
    return KX_SUCCESS;
}

void KX_CALL kxDestroyDevice(
    KxDevice device,
    const KxAllocationCallbacks* /*pAllocator*/) {
    if (!device) return;
    for (auto it = kx::g_devices().begin(); it != kx::g_devices().end(); ++it) {
        if (reinterpret_cast<KxDevice>(it->get()) == device) {
            kx::g_devices().erase(it);
            return;
        }
    }
}

KxResult KX_CALL kxGetDeviceQueue(
    KxDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    KxQueue* pQueue) {
    if (!pQueue) return KX_ERROR_INVALID_ARGUMENT;
    *pQueue = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    (void)dev; (void)queueFamilyIndex; (void)queueIndex;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* =======================================================================
 * Memory / Buffer / Image (delegated to device dispatch)
 * ======================================================================= */

KxResult KX_CALL kxAllocateMemory(
    KxDevice device,
    const KxMemoryAllocateInfo* pAllocateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDeviceMemory* pMemory) {
    if (!pMemory) return KX_ERROR_INVALID_ARGUMENT;
    *pMemory = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->allocate_memory)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->allocate_memory(device, pAllocateInfo, pAllocator,
                                            pMemory);
}

void KX_CALL kxFreeMemory(
    KxDevice device,
    KxDeviceMemory memory,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->free_memory) return;
    dev->dispatch()->free_memory(device, memory, pAllocator);
}

KxResult KX_CALL kxCreateBuffer(
    KxDevice device,
    const KxBufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxBuffer* pBuffer) {
    if (!pBuffer) return KX_ERROR_INVALID_ARGUMENT;
    *pBuffer = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_buffer)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_buffer(device, pCreateInfo, pAllocator,
                                          pBuffer);
}

void KX_CALL kxDestroyBuffer(
    KxDevice device,
    KxBuffer buffer,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_buffer) return;
    dev->dispatch()->destroy_buffer(device, buffer, pAllocator);
}

KxResult KX_CALL kxBindBufferMemory(
    KxDevice device,
    KxBuffer buffer,
    KxDeviceMemory memory,
    uint64_t memoryOffset) {
    (void)device; (void)buffer; (void)memory; (void)memoryOffset;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxCreateImage(
    KxDevice device,
    const KxImageCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage) {
    if (!pImage) return KX_ERROR_INVALID_ARGUMENT;
    *pImage = nullptr;
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev) return KX_ERROR_INVALID_ARGUMENT;
    if (!dev->dispatch() || !dev->dispatch()->create_image)
        return KX_ERROR_FEATURE_NOT_PRESENT;
    return dev->dispatch()->create_image(device, pCreateInfo, pAllocator,
                                         pImage);
}

void KX_CALL kxDestroyImage(
    KxDevice device,
    KxImage image,
    const KxAllocationCallbacks* pAllocator) {
    kx::Device* dev = kx::find_device_locked(device);
    if (!dev || !dev->dispatch() || !dev->dispatch()->destroy_image) return;
    dev->dispatch()->destroy_image(device, image, pAllocator);
}

KxResult KX_CALL kxBindImageMemory(
    KxDevice device,
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memoryOffset) {
    (void)device; (void)image; (void)memory; (void)memoryOffset;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* Image views */

KxResult KX_CALL kxCreateImageView(
    KxDevice device,
    const KxImageViewCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImageView* pImageView) {
    (void)device; (void)pCreateInfo; (void)pAllocator; (void)pImageView;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroyImageView(
    KxDevice device,
    KxImageView imageView,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)imageView; (void)pAllocator;
}

/* Shader modules */

KxResult KX_CALL kxCreateShaderModule(
    KxDevice device,
    const KxShaderModuleCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule) {
    (void)device; (void)pCreateInfo; (void)pAllocator; (void)pShaderModule;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroyShaderModule(
    KxDevice device,
    KxShaderModule shaderModule,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)shaderModule; (void)pAllocator;
}

/* Pipelines */

KxResult KX_CALL kxCreatePipelineLayout(
    KxDevice device,
    const KxPipelineLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipelineLayout* pPipelineLayout) {
    (void)device; (void)pCreateInfo; (void)pAllocator; (void)pPipelineLayout;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroyPipelineLayout(
    KxDevice device,
    KxPipelineLayout pipelineLayout,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)pipelineLayout; (void)pAllocator;
}

KxResult KX_CALL kxCreatePipeline(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const KxPipelineCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipeline* pPipeline) {
    (void)device; (void)pipelineCache; (void)pCreateInfo; (void)pAllocator;
    (void)pPipeline;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KX_CALL kxDestroyPipeline(
    KxDevice device,
    KxPipeline pipeline,
    const KxAllocationCallbacks* pAllocator) {
    (void)device; (void)pipeline; (void)pAllocator;
}

/* Command pools */

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

/* Command buffers */

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
    (void)commandBuffer; (void)pBeginInfo;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxEndCommandBuffer(KxCommandBuffer commandBuffer) {
    (void)commandBuffer;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxResetCommandBuffer(
    KxCommandBuffer commandBuffer,
    KxCommandBufferResetFlags flags) {
    (void)commandBuffer; (void)flags;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* Fences / semaphores */

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

/* Queue submit */

KxResult KX_CALL kxQueueSubmit(
    KxQueue queue,
    uint32_t submitCount,
    const KxSubmitInfo* pSubmits,
    KxFence fence) {
    (void)queue; (void)submitCount; (void)pSubmits; (void)fence;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxQueueWaitIdle(KxQueue queue) {
    (void)queue;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KX_CALL kxDeviceWaitIdle(KxDevice device) {
    (void)device;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

/* Proc addresses */

KxPfnVoidFunction KX_CALL kxGetDeviceProcAddr(
    KxDevice device,
    const char* pName) {
    (void)device; (void)pName;
    return nullptr;
}

KxPfnVoidFunction KX_CALL kxGetInstanceProcAddr(
    KxInstance instance,
    const char* pName) {
    (void)instance; (void)pName;
    return nullptr;
}

/* =======================================================================
 * Commands recordable in a command buffer (stubs)
 * ======================================================================= */

void KX_CALL kxCmdBindPipeline(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipeline pipeline) {
    (void)commandBuffer; (void)pipelineBindPoint; (void)pipeline;
}

void KX_CALL kxCmdBindVertexBuffers(
    KxCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const KxBuffer* pBuffers,
    const uint64_t* pOffsets) {
    (void)commandBuffer; (void)firstBinding; (void)bindingCount;
    (void)pBuffers; (void)pOffsets;
}

void KX_CALL kxCmdBindDescriptors(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const void* pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets) {
    (void)commandBuffer; (void)pipelineBindPoint; (void)layout;
    (void)firstSet; (void)setCount; (void)pDescriptorSets;
    (void)dynamicOffsetCount; (void)pDynamicOffsets;
}

void KX_CALL kxCmdDraw(
    KxCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance) {
    (void)commandBuffer; (void)vertexCount; (void)instanceCount;
    (void)firstVertex; (void)firstInstance;
}

void KX_CALL kxCmdDrawIndexed(
    KxCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance) {
    (void)commandBuffer; (void)indexCount; (void)instanceCount;
    (void)firstIndex; (void)vertexOffset; (void)firstInstance;
}

void KX_CALL kxCmdDispatch(
    KxCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ) {
    (void)commandBuffer; (void)groupCountX; (void)groupCountY;
    (void)groupCountZ;
}

void KX_CALL kxCmdCopyBuffer(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxBuffer dstBuffer,
    uint32_t regionCount,
    const void* pRegions) {
    (void)commandBuffer; (void)srcBuffer; (void)dstBuffer;
    (void)regionCount; (void)pRegions;
}

void KX_CALL kxCmdCopyBufferToImage(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxImage dstImage,
    KxImageLayout dstImageLayout,
    uint32_t regionCount,
    const void* pRegions) {
    (void)commandBuffer; (void)srcBuffer; (void)dstImage;
    (void)dstImageLayout; (void)regionCount; (void)pRegions;
}

void KX_CALL kxCmdClearImage(
    KxCommandBuffer commandBuffer,
    KxImage image,
    KxImageLayout imageLayout,
    const void* pColor,
    const void* pDepthStencil,
    uint32_t rangeCount,
    const void* pRanges) {
    (void)commandBuffer; (void)image; (void)imageLayout;
    (void)pColor; (void)pDepthStencil; (void)rangeCount; (void)pRanges;
}

void KX_CALL kxCmdPipelineBarrier(
    KxCommandBuffer commandBuffer,
    KxPipelineStageFlags srcStageMask,
    KxPipelineStageFlags dstStageMask,
    KxDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const void* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const void* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const void* pImageMemoryBarriers) {
    (void)commandBuffer; (void)srcStageMask; (void)dstStageMask;
    (void)dependencyFlags; (void)memoryBarrierCount; (void)pMemoryBarriers;
    (void)bufferMemoryBarrierCount; (void)pBufferMemoryBarriers;
    (void)imageMemoryBarrierCount; (void)pImageMemoryBarriers;
}

/* =======================================================================
 * Result string conversion
 * ======================================================================= */

const char* KX_CALL kxResultToString(KxResult result) {
    switch (result) {
    case KX_SUCCESS: return "KX_SUCCESS";
    case KX_NOT_READY: return "KX_NOT_READY";
    case KX_TIMEOUT: return "KX_TIMEOUT";
    case KX_EVENT_SET: return "KX_EVENT_SET";
    case KX_EVENT_RESET: return "KX_EVENT_RESET";
    case KX_INCOMPLETE: return "KX_INCOMPLETE";
    case KX_ERROR_OUT_OF_HOST_MEMORY: return "KX_ERROR_OUT_OF_HOST_MEMORY";
    case KX_ERROR_OUT_OF_DEVICE_MEMORY: return "KX_ERROR_OUT_OF_DEVICE_MEMORY";
    case KX_ERROR_INITIALIZATION_FAILED: return "KX_ERROR_INITIALIZATION_FAILED";
    case KX_ERROR_DEVICE_LOST: return "KX_ERROR_DEVICE_LOST";
    case KX_ERROR_MEMORY_MAP_FAILED: return "KX_ERROR_MEMORY_MAP_FAILED";
    case KX_ERROR_LAYER_NOT_PRESENT: return "KX_ERROR_LAYER_NOT_PRESENT";
    case KX_ERROR_EXTENSION_NOT_PRESENT: return "KX_ERROR_EXTENSION_NOT_PRESENT";
    case KX_ERROR_FEATURE_NOT_PRESENT: return "KX_ERROR_FEATURE_NOT_PRESENT";
    case KX_ERROR_INCOMPATIBLE_DRIVER: return "KX_ERROR_INCOMPATIBLE_DRIVER";
    case KX_ERROR_TOO_MANY_OBJECTS: return "KX_ERROR_TOO_MANY_OBJECTS";
    case KX_ERROR_FORMAT_NOT_SUPPORTED: return "KX_ERROR_FORMAT_NOT_SUPPORTED";
    case KX_ERROR_FRAGMENTED_POOL: return "KX_ERROR_FRAGMENTED_POOL";
    case KX_ERROR_OUT_OF_POOL_MEMORY: return "KX_ERROR_OUT_OF_POOL_MEMORY";
    case KX_ERROR_INVALID_EXTERNAL_HANDLE: return "KX_ERROR_INVALID_EXTERNAL_HANDLE";
    case KX_ERROR_INVALID_ARGUMENT: return "KX_ERROR_INVALID_ARGUMENT";
    case KX_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "KX_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case KX_ERROR_VALIDATION_FAILED_EXT: return "KX_ERROR_VALIDATION_FAILED_EXT";
    default: return "KX_UNKNOWN_RESULT";
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif