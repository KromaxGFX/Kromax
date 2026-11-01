#ifndef KROMAX_SRC_CPU_BACKEND_CPURUNTIME_H
#define KROMAX_SRC_CPU_BACKEND_CPURUNTIME_H

/*
 * CpuRuntime.h
 *
 * CPU backend runtime implementation.
 */

#include "IRuntime.h"
#include "IBackend.h"
#include "PhysicalDevice.h"  // For PhysicalDevice data structure

#include <memory>
#include <vector>
#include <string>

namespace kx {

// Forward declarations
class CpuDevice;
class CpuShaderModule;
class CpuPipeline;
class CpuDescriptorSetLayout;
class CpuDescriptorSet;
class CpuCommandPool;
class CpuCommandBuffer;
class CpuSwapchain;
class CpuSemaphore;
class CpuFence;
class CpuQueue;

// CpuInstance - lightweight instance tracking
class CpuInstance final : public Instance {
public:
    explicit CpuInstance(const InstanceCreateInfo& info);
    ~CpuInstance() override = default;

    const KxApplicationInfo* getAppInfo() const noexcept { return &appInfo_; }
    const std::vector<std::string>& getEnabledExtensions() const noexcept { return enabledExtensions_; }
    const std::vector<std::string>& getEnabledLayers() const noexcept { return enabledLayers_; }

private:
    KxApplicationInfo appInfo_{};
    std::vector<std::string> enabledExtensions_;
    std::vector<std::string> enabledLayers_;
};

// CpuPhysicalDevice - singleton for CPU physical device properties
class CpuPhysicalDevice {
public:
    static CpuPhysicalDevice& get() noexcept {
        static CpuPhysicalDevice instance;
        return instance;
    }

    PhysicalDeviceInfo getPhysicalDeviceData() const noexcept {
        PhysicalDeviceInfo pd;
        pd.handle = 1;
        pd.properties = properties_;
        pd.features = features_;
        pd.queueFamilies = queueFamilies_;
        pd.memoryHeaps = memoryHeaps_;
        pd.memoryTypes = memoryTypes_;
        return pd;
    }

    const KxPhysicalDeviceProperties& getProperties() const noexcept { return properties_; }
    const KxPhysicalDeviceFeatures& getFeatures() const noexcept { return features_; }
    const std::vector<KxQueueFamilyProperties>& getQueueFamilies() const noexcept { return queueFamilies_; }
    const KxPhysicalDeviceMemoryProperties getMemoryProperties() const noexcept {
        KxPhysicalDeviceMemoryProperties mp{};
        mp.memoryHeapCount = static_cast<uint32_t>(memoryHeaps_.size());
        for (size_t i = 0; i < memoryHeaps_.size() && i < 16; ++i) {
            mp.memoryHeaps[i] = memoryHeaps_[i];
        }
        mp.memoryTypeCount = static_cast<uint32_t>(memoryTypes_.size());
        for (size_t i = 0; i < memoryTypes_.size() && i < 32; ++i) {
            mp.memoryTypes[i] = memoryTypes_[i];
        }
        return mp;
    }

private:
    CpuPhysicalDevice() {
        // Properties
        properties_.apiVersion = KX_API_VERSION_0_1;
        properties_.driverVersion = KX_MAKE_VERSION(0, 1, 0);
        properties_.vendorID = 0x1000;
        properties_.deviceID = 0x0001;
        properties_.deviceType = KX_PHYSICAL_DEVICE_TYPE_CPU;
        std::snprintf(properties_.deviceName, sizeof(properties_.deviceName), "Kromax Software CPU");
        std::snprintf(properties_.driverName, sizeof(properties_.driverName), "kromax-software");

        // Features
        features_.shaderFloat64 = KX_TRUE;
        features_.shaderInt64 = KX_TRUE;
        features_.geometryShader = KX_FALSE;
        features_.tessellationShader = KX_FALSE;
        features_.textureCompressionBC = KX_FALSE;
        features_.textureCompressionETC2 = KX_FALSE;
        features_.textureCompressionASTC_LDR = KX_FALSE;
        features_.pipelineStatisticsQuery = KX_FALSE;
        features_.vertexPipelineStoresAndAtomics = KX_FALSE;

        // Queue families
        KxQueueFamilyProperties qf{};
        qf.queueFlags = KX_QUEUE_GRAPHICS_BIT | KX_QUEUE_COMPUTE_BIT | KX_QUEUE_TRANSFER_BIT;
        qf.queueCount = 1;
        qf.timestampValidBits = 64;
        queueFamilies_.push_back(qf);

        // Memory: 1 heap, 1 type (host visible + coherent)
        KxMemoryHeap heap{};
        heap.size = 1ull << 30;  // 1 GiB
        memoryHeaps_.push_back(heap);

        KxMemoryType type{};
        type.heapIndex = 0;
        type.propertyFlags = KX_MEMORY_PROPERTY_HOST_VISIBLE_BIT | KX_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        memoryTypes_.push_back(type);
    }

    KxPhysicalDeviceProperties properties_{};
    KxPhysicalDeviceFeatures features_{};
    std::vector<KxQueueFamilyProperties> queueFamilies_;
    std::vector<KxMemoryHeap> memoryHeaps_;
    std::vector<KxMemoryType> memoryTypes_;
};

// CpuRuntime - main backend implementation
class CpuRuntime final : public IRuntime {
public:
    CpuRuntime() = default;
    ~CpuRuntime() override = default;

    // Static backend type for registration
    static BackendType getBackendTypeStatic() noexcept { return BackendType::CPU; }

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
    KxResult getExtensionProperties(uint32_t* pPropertyCount, KxExtensionProperties* pProperties) override;
};

} // namespace kx

#endif // KROMAX_SRC_CPU_BACKEND_CPURUNTIME_H