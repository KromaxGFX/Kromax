/*
 * api.cpp
 *
 * Public C API implementation. Main entry point and global registry.
 * Function implementations are in api_*.cpp modules.
 */

#include "api.h"
#include "api_internal.h"

#include <cstring>
#include <utility>

#include "IDriver.h"
#include "DriverRegistry.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "Queue.h"

#include "SoftwareDriver.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareDevice.h"

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

Device* find_device_locked_by_command_buffer(KxCommandBuffer cb) {
    for (auto& dev : g_devices()) {
        auto* sw_dev = static_cast<SoftwareDevice*>(dev->driver_device());
        if (sw_dev && sw_dev->find_command_buffer(cb)) return dev.get();
    }
    return nullptr;
}

Device* find_device_locked_by_queue(KxQueue queue) {
    for (auto& dev : g_devices()) {
        if (dev->owns_queue(queue)) return dev.get();
    }
    return nullptr;
}

} /* namespace kx */

// Function declarations from api_*.cpp modules
extern "C" {

// api_instance.cpp
KxResult KX_CALL kxCreateInstance(const KxInstanceCreateInfo*, const KxAllocationCallbacks*, KxInstance*);
void KX_CALL kxDestroyInstance(KxInstance, const KxAllocationCallbacks*);
KxResult KX_CALL kxEnumeratePhysicalDevices(KxInstance, uint32_t*, KxPhysicalDevice*);
KxResult KX_CALL kxGetPhysicalDeviceProperties(KxPhysicalDevice, KxPhysicalDeviceProperties*);
KxResult KX_CALL kxGetPhysicalDeviceFeatures(KxPhysicalDevice, KxPhysicalDeviceFeatures*);
KxResult KX_CALL kxGetPhysicalDeviceMemoryProperties(KxPhysicalDevice, KxPhysicalDeviceMemoryProperties*);
KxResult KX_CALL kxGetPhysicalDeviceQueueFamilyProperties(KxPhysicalDevice, uint32_t*, KxQueueFamilyProperties*);

// api_device.cpp
KxResult KX_CALL kxCreateDevice(KxPhysicalDevice, const KxDeviceCreateInfo*, const KxAllocationCallbacks*, KxDevice*);
void KX_CALL kxDestroyDevice(KxDevice, const KxAllocationCallbacks*);
KxResult KX_CALL kxGetDeviceQueue(KxDevice, uint32_t, uint32_t, KxQueue*);
KxResult KX_CALL kxAllocateMemory(KxDevice, const KxMemoryAllocateInfo*, const KxAllocationCallbacks*, KxDeviceMemory*);
void KX_CALL kxFreeMemory(KxDevice, KxDeviceMemory, const KxAllocationCallbacks*);
KxResult KX_CALL kxMapMemory(KxDevice, KxDeviceMemory, uint64_t, uint64_t, KxMemoryMapFlags, void**);
void KX_CALL kxUnmapMemory(KxDevice, KxDeviceMemory);
KxResult KX_CALL kxCreateBuffer(KxDevice, const KxBufferCreateInfo*, const KxAllocationCallbacks*, KxBuffer*);
void KX_CALL kxDestroyBuffer(KxDevice, KxBuffer, const KxAllocationCallbacks*);
KxResult KX_CALL kxBindBufferMemory(KxDevice, KxBuffer, KxDeviceMemory, uint64_t);
KxResult KX_CALL kxCreateImage(KxDevice, const KxImageCreateInfo*, const KxAllocationCallbacks*, KxImage*);
void KX_CALL kxDestroyImage(KxDevice, KxImage, const KxAllocationCallbacks*);
KxResult KX_CALL kxBindImageMemory(KxDevice, KxImage, KxDeviceMemory, uint64_t);

// api_command.cpp
KxResult KX_CALL kxCreateCommandPool(KxDevice, const KxCommandPoolCreateInfo*, const KxAllocationCallbacks*, KxCommandPool*);
void KX_CALL kxDestroyCommandPool(KxDevice, KxCommandPool, const KxAllocationCallbacks*);
KxResult KX_CALL kxResetCommandPool(KxDevice, KxCommandPool, KxCommandPoolResetFlags);
KxResult KX_CALL kxAllocateCommandBuffers(KxDevice, const KxCommandBufferAllocateInfo*, KxCommandBuffer*);
void KX_CALL kxFreeCommandBuffers(KxDevice, KxCommandPool, uint32_t, const KxCommandBuffer*);
KxResult KX_CALL kxBeginCommandBuffer(KxCommandBuffer, const KxCommandBufferBeginInfo*);
KxResult KX_CALL kxEndCommandBuffer(KxCommandBuffer);
KxResult KX_CALL kxResetCommandBuffer(KxCommandBuffer, KxCommandBufferResetFlags);
KxResult KX_CALL kxCreateFence(KxDevice, const KxFenceCreateInfo*, const KxAllocationCallbacks*, KxFence*);
void KX_CALL kxDestroyFence(KxDevice, KxFence, const KxAllocationCallbacks*);
KxResult KX_CALL kxWaitForFences(KxDevice, uint32_t, const KxFence*, KxBool32, uint64_t);
KxResult KX_CALL kxResetFences(KxDevice, uint32_t, const KxFence*);
KxResult KX_CALL kxGetFenceStatus(KxDevice, KxFence);
KxResult KX_CALL kxCreateSemaphore(KxDevice, const KxSemaphoreCreateInfo*, const KxAllocationCallbacks*, KxSemaphore*);
void KX_CALL kxDestroySemaphore(KxDevice, KxSemaphore, const KxAllocationCallbacks*);
KxResult KX_CALL kxQueueSubmit(KxQueue, uint32_t, const KxSubmitInfo*, KxFence);
KxResult KX_CALL kxQueueWaitIdle(KxQueue);
KxResult KX_CALL kxDeviceWaitIdle(KxDevice);

// api_renderpass.cpp
KxResult KX_CALL kxCreateImageView(KxDevice, const KxImageViewCreateInfo*, const KxAllocationCallbacks*, KxImageView*);
void KX_CALL kxDestroyImageView(KxDevice, KxImageView, const KxAllocationCallbacks*);
KxResult KX_CALL kxCreateRenderPass(KxDevice, const KxRenderPassCreateInfo*, const KxAllocationCallbacks*, KxRenderPass*);
void KX_CALL kxDestroyRenderPass(KxDevice, KxRenderPass, const KxAllocationCallbacks*);
KxResult KX_CALL kxCreateFramebuffer(KxDevice, const KxFramebufferCreateInfo*, const KxAllocationCallbacks*, KxFramebuffer*);
void KX_CALL kxDestroyFramebuffer(KxDevice, KxFramebuffer, const KxAllocationCallbacks*);
void KX_CALL kxCmdBeginRenderPass(KxCommandBuffer, const KxRenderPassBeginInfo*, KxSubpassContents);
void KX_CALL kxCmdEndRenderPass(KxCommandBuffer);
void KX_CALL kxCmdNextSubpass(KxCommandBuffer, KxSubpassContents);

// api_pipeline.cpp
KxResult KX_CALL kxCreateShaderModule(KxDevice, const KxShaderModuleCreateInfo*, const KxAllocationCallbacks*, KxShaderModule*);
void KX_CALL kxDestroyShaderModule(KxDevice, KxShaderModule, const KxAllocationCallbacks*);
KxResult KX_CALL kxCreatePipelineLayout(KxDevice, const KxPipelineLayoutCreateInfo*, const KxAllocationCallbacks*, KxPipelineLayout*);
void KX_CALL kxDestroyPipelineLayout(KxDevice, KxPipelineLayout, const KxAllocationCallbacks*);
KxResult KX_CALL kxCreatePipeline(KxDevice, KxPipelineCache, const KxPipelineCreateInfo*, const KxAllocationCallbacks*, KxPipeline*);
void KX_CALL kxDestroyPipeline(KxDevice, KxPipeline, const KxAllocationCallbacks*);
void KX_CALL kxCmdBindPipeline(KxCommandBuffer, KxPipelineBindPoint, KxPipeline);
void KX_CALL kxCmdBindVertexBuffers(KxCommandBuffer, uint32_t, uint32_t, const KxBuffer*, const uint64_t*);
void KX_CALL kxCmdBindDescriptors(KxCommandBuffer, KxPipelineBindPoint, KxPipelineLayout, uint32_t, uint32_t, const KxDescriptorSet*, uint32_t, const uint32_t*);
void KX_CALL kxCmdDraw(KxCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t);
void KX_CALL kxCmdDrawIndexed(KxCommandBuffer, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);
void KX_CALL kxCmdDispatch(KxCommandBuffer, uint32_t, uint32_t, uint32_t);
void KX_CALL kxCmdCopyBuffer(KxCommandBuffer, KxBuffer, KxBuffer, uint32_t, const void*);
void KX_CALL kxCmdCopyBufferToImage(KxCommandBuffer, KxBuffer, KxImage, KxImageLayout, uint32_t, const void*);
void KX_CALL kxCmdClearImage(KxCommandBuffer, KxImage, KxImageLayout, const void*, const void*, uint32_t, const void*);
void KX_CALL kxCmdPipelineBarrier(KxCommandBuffer, KxPipelineStageFlags, KxPipelineStageFlags, KxDependencyFlags, uint32_t, const void*, uint32_t, const void*, uint32_t, const void*);
KxPfnVoidFunction KX_CALL kxGetDeviceProcAddr(KxDevice, const char*);
KxPfnVoidFunction KX_CALL kxGetInstanceProcAddr(KxInstance, const char*);
const char* KX_CALL kxResultToString(KxResult);

} /* extern "C" */