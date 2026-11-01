#ifndef KROMAX_SRC_RUNTIME_IRUNTIME_H
#define KROMAX_SRC_RUNTIME_IRUNTIME_H

/*
 * IRuntime.h
 *
 * Backend-agnostic runtime interface. Each backend (CPU, Vulkan, Metal, DX12)
 * implements this interface. The frontend depends only on this abstraction.
 *
 * SOLID: Dependency Inversion Principle - high-level frontend depends on
 * abstraction, not concrete backend implementations.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <optional>

#include "kx_core.h"
#include "Expected.h"

namespace kx {

// Abstract base classes for opaque types (backend implements these)
class Instance {
public:
    virtual ~Instance() = default;
};

class Device {
public:
    virtual ~Device() = default;
};

class ShaderModule {
public:
    virtual ~ShaderModule() = default;
};

class Pipeline {
public:
    virtual ~Pipeline() = default;
};

class DescriptorSetLayout {
public:
    virtual ~DescriptorSetLayout() = default;
};

class DescriptorSet {
public:
    virtual ~DescriptorSet() = default;
};

class CommandPool {
public:
    virtual ~CommandPool() = default;
};

class CommandBuffer {
public:
    virtual ~CommandBuffer() = default;
};

class Swapchain {
public:
    virtual ~Swapchain() = default;
};

class Semaphore {
public:
    virtual ~Semaphore() = default;
};

class Fence {
public:
    virtual ~Fence() = default;
};

class Queue {
public:
    virtual ~Queue() = default;
};

class ThreadPool {
public:
    virtual ~ThreadPool() = default;
};

enum class BackendType : uint32_t {
    UNKNOWN = 0,
    CPU     = 1,
    VULKAN  = 2,
    METAL   = 3,
    DX12    = 4,
};

struct InstanceCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    const void* pNext = nullptr;
    KxInstanceCreateFlags flags = 0;
    const KxApplicationInfo* pApplicationInfo = nullptr;
    uint32_t enabledLayerCount = 0;
    const char* const* ppEnabledLayerNames = nullptr;
    uint32_t enabledExtensionCount = 0;
    const char* const* ppEnabledExtensionNames = nullptr;
};

struct DeviceCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    const void* pNext = nullptr;
    KxDeviceCreateFlags flags = 0;
    uint32_t queueCreateInfoCount = 0;
    const KxDeviceQueueCreateInfo* pQueueCreateInfos = nullptr;
    uint32_t enabledExtensionCount = 0;
    const char* const* ppEnabledExtensionNames = nullptr;
    const KxPhysicalDeviceFeatures* pEnabledFeatures = nullptr;
};

struct ShaderSourceCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
    const void* pNext = nullptr;
    KxShaderStageFlags stage;
    KxShaderLanguage language;
    const char* pSource;
    size_t sourceSize;
    const char* pEntryPoint;
    uint32_t defineCount = 0;
    const void* pDefines = nullptr;  // Opaque pointer to shader defines
};

struct PipelineCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineCreateFlags flags = 0;
    uint32_t stageCount = 0;
    const void* pStages = nullptr;  // Pipeline shader stage create infos
    const void* pVertexInputState = nullptr;
    const void* pInputAssemblyState = nullptr;
    const void* pTessellationState = nullptr;
    const void* pViewportState = nullptr;
    const void* pRasterizationState = nullptr;
    const void* pMultisampleState = nullptr;
    const void* pDepthStencilState = nullptr;
    const void* pColorBlendState = nullptr;
    const void* pDynamicState = nullptr;
    void* layout = nullptr;
    void* renderPass = nullptr;
    uint32_t subpass = 0;
    void* basePipelineHandle = nullptr;
    int32_t basePipelineIndex = -1;
};

struct SwapchainCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_EXT;
    const void* pNext = nullptr;
    uint32_t flags = 0;
    void* surface = nullptr;
    uint32_t minImageCount = 2;
    KxFormat imageFormat = KX_FORMAT_B8G8R8A8_SRGB;
    uint32_t imageColorSpace = 0;
    KxExtent2D imageExtent = {0, 0};
    uint32_t imageArrayLayers = 1;
    KxImageUsageFlags imageUsage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    KxSharingMode imageSharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    uint32_t preTransform = 0;
    uint32_t compositeAlpha = 0;
    uint32_t presentMode = 0;
    KxBool32 clipped = true;
    void* oldSwapchain = nullptr;
};

struct SemaphoreCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    const void* pNext = nullptr;
    KxSemaphoreCreateFlags flags = 0;
    uint64_t initialValue = 0;
};

struct ThreadPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    uint32_t threadCount = 0;
};

struct PhysicalDeviceInfo {
    uint64_t handle = 0;
    KxPhysicalDeviceProperties properties{};
    KxPhysicalDeviceFeatures features{};
    std::vector<KxQueueFamilyProperties> queueFamilies;
    std::vector<KxMemoryHeap> memoryHeaps;
    std::vector<KxMemoryType> memoryTypes;
};

class IRuntime {
public:
    virtual ~IRuntime() = default;

    virtual const char* getBackendName() const noexcept = 0;
    virtual BackendType getBackendType() const noexcept = 0;
    virtual uint32_t getBackendVersion() const noexcept = 0;

    virtual KxResult createInstance(const InstanceCreateInfo& info, Instance** outInstance) = 0;
    virtual void destroyInstance(Instance* instance) = 0;

    virtual KxResult enumeratePhysicalDevices(Instance* instance,
                                               uint32_t* pPhysicalDeviceCount,
                                               PhysicalDeviceInfo* pPhysicalDevices) = 0;
    virtual KxResult getPhysicalDeviceProperties(Instance* instance,
                                                  uint64_t physicalDeviceHandle,
                                                  KxPhysicalDeviceProperties* pProperties) = 0;
    virtual KxResult getPhysicalDeviceFeatures(Instance* instance,
                                                uint64_t physicalDeviceHandle,
                                                KxPhysicalDeviceFeatures* pFeatures) = 0;
    virtual KxResult getPhysicalDeviceQueueFamilyProperties(Instance* instance,
                                                             uint64_t physicalDeviceHandle,
                                                             uint32_t* pQueueFamilyPropertyCount,
                                                             KxQueueFamilyProperties* pQueueFamilyProperties) = 0;
    virtual KxResult getPhysicalDeviceMemoryProperties(Instance* instance,
                                                        uint64_t physicalDeviceHandle,
                                                        KxPhysicalDeviceMemoryProperties* pMemoryProperties) = 0;

    virtual KxResult createDevice(uint64_t physicalDeviceHandle,
                                   const DeviceCreateInfo& info,
                                   Device** outDevice) = 0;

    virtual KxResult compileShader(const ShaderSourceCreateInfo& info, ShaderModule** outModule) = 0;
    virtual KxResult compileShaderToSPIRV(const ShaderSourceCreateInfo& info, std::vector<uint32_t>* outSpirv) = 0;

    // Thread pool
    virtual KxResult createThreadPool(const ThreadPoolCreateInfo& info, ThreadPool** outThreadPool) = 0;
    virtual void destroyThreadPool(ThreadPool* threadPool) = 0;

    virtual bool isExtensionSupported(const char* extensionName) const noexcept = 0;
    virtual KxResult getExtensionProperties(uint32_t* pPropertyCount, void* pProperties) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IRUNTIME_H