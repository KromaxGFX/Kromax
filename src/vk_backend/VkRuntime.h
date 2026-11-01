#ifndef KROMAX_SRC_VK_BACKEND_VKRUNTIME_H
#define KROMAX_SRC_VK_BACKEND_VKRUNTIME_H

/*
 * VkRuntime.h
 *
 * Vulkan backend implementation of IRuntime.
 * Implements the backend-agnostic interface using Vulkan API.
 * Delegates to modular components: VkLoader, VkInstance, VkPhysicalDevice, VkDevice.
 */

#include "IRuntime.h"
#include "VkLoader.h"
#include "VkInstance.h"
#include "VkPhysicalDevice.h"
#include "VkDevice.h"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

namespace kx {

// Forward declarations for remaining Vulkan objects
class VkShaderModuleImpl;
class VkPipelineImpl;
class VkDescriptorSetLayoutImpl;
class VkDescriptorSetImpl;
class VkCommandPoolImpl;
class VkCommandBufferImpl;
class VkSwapchainImpl;
class VkSemaphoreImpl;
class VkFenceImpl;
class VkQueueImpl;
class VkThreadPoolImpl;

// Vulkan runtime implementation
class VkRuntime final : public IRuntime {
public:
    VkRuntime();
    ~VkRuntime() override;

    // Static backend type for registration
    static BackendType getBackendTypeStatic() noexcept { return BackendType::VULKAN; }

    // IRuntime interface
    const char* getBackendName() const noexcept override;
    BackendType getBackendType() const noexcept override;
    uint32_t getBackendVersion() const noexcept override;

    KxResult createInstance(const InstanceCreateInfo& info, Instance** outInstance) override;
    void destroyInstance(Instance* instance) override;

    KxResult enumeratePhysicalDevices(Instance* instance,
                                       uint32_t* pPhysicalDeviceCount,
                                       PhysicalDeviceInfo* pPhysicalDevices) override;
    KxResult getPhysicalDeviceProperties(Instance* instance,
                                          uint64_t physicalDeviceHandle,
                                          KxPhysicalDeviceProperties* pProperties) override;
    KxResult getPhysicalDeviceFeatures(Instance* instance,
                                        uint64_t physicalDeviceHandle,
                                        KxPhysicalDeviceFeatures* pFeatures) override;
    KxResult getPhysicalDeviceQueueFamilyProperties(Instance* instance,
                                                     uint64_t physicalDeviceHandle,
                                                     uint32_t* pQueueFamilyPropertyCount,
                                                     KxQueueFamilyProperties* pQueueFamilyProperties) override;
    KxResult getPhysicalDeviceMemoryProperties(Instance* instance,
                                                uint64_t physicalDeviceHandle,
                                                KxPhysicalDeviceMemoryProperties* pMemoryProperties) override;

    KxResult createDevice(uint64_t physicalDeviceHandle,
                           const DeviceCreateInfo& info,
                           Device** outDevice) override;

    KxResult compileShader(const ShaderSourceCreateInfo& info, ShaderModule** outModule) override;
    KxResult compileShaderToSPIRV(const ShaderSourceCreateInfo& info, std::vector<uint32_t>* outSpirv) override;

    bool isExtensionSupported(const char* extensionName) const noexcept override;
    KxResult getExtensionProperties(uint32_t* pPropertyCount, void* pProperties) override;

    // Thread pool
    KxResult createThreadPool(const ThreadPoolCreateInfo& info, ThreadPool** outThreadPool) override;
    void destroyThreadPool(ThreadPool* threadPool) override;

    // Access to function table for advanced usage
    const VkFunctionTable& getFunctionTable() const noexcept { return loader_.getFunctionTable(); }
    VkFunctionTable& getFunctionTable() noexcept { return loader_.getFunctionTable(); }

private:
    VkLoader loader_;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKRUNTIME_H