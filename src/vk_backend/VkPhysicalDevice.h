#ifndef KROMAX_SRC_VK_BACKEND_VKPHYSICALDEVICE_H
#define KROMAX_SRC_VK_BACKEND_VKPHYSICALDEVICE_H

/*
 * VkPhysicalDevice.h
 *
 * Vulkan physical device wrapper and enumeration.
 * Handles VkPhysicalDevice properties, features, queue families, and memory properties.
 */

#include "IRuntime.h"
#include "VkLoader.h"

#include <vector>

namespace kx {

// Vulkan physical device wrapper
class VkPhysicalDeviceImpl final {
public:
    VkPhysicalDeviceImpl() = default;
    explicit VkPhysicalDeviceImpl(void* handle, uint32_t index);

    void* getHandle() const noexcept { return handle_; }
    uint32_t getIndex() const noexcept { return index_; }
    const KxPhysicalDeviceProperties& getProperties() const noexcept { return properties_; }
    const KxPhysicalDeviceFeatures& getFeatures() const noexcept { return features_; }
    const std::vector<KxQueueFamilyProperties>& getQueueFamilies() const noexcept { return queueFamilies_; }
    const std::vector<KxMemoryHeap>& getMemoryHeaps() const noexcept { return memoryHeaps_; }
    const std::vector<KxMemoryType>& getMemoryTypes() const noexcept { return memoryTypes_; }

    // Initialize physical device properties from Vulkan
    bool initialize(VkFunctionTable& functions);

private:
    void* handle_ = nullptr;  // VkPhysicalDevice
    uint32_t index_ = 0;
    KxPhysicalDeviceProperties properties_{};
    KxPhysicalDeviceFeatures features_{};
    std::vector<KxQueueFamilyProperties> queueFamilies_;
    std::vector<KxMemoryHeap> memoryHeaps_;
    std::vector<KxMemoryType> memoryTypes_;
};

// Physical device enumeration and query utilities
namespace VkPhysicalDeviceUtil {

// Enumerate physical devices for an instance
KxResult enumeratePhysicalDevices(VkFunctionTable& functions,
                                   void* instanceHandle,
                                   uint32_t* pPhysicalDeviceCount,
                                   PhysicalDeviceInfo* pPhysicalDevices);

// Get physical device properties
KxResult getPhysicalDeviceProperties(VkFunctionTable& functions,
                                      void* physicalDeviceHandle,
                                      KxPhysicalDeviceProperties* pProperties);

// Get physical device features
KxResult getPhysicalDeviceFeatures(VkFunctionTable& functions,
                                    void* physicalDeviceHandle,
                                    KxPhysicalDeviceFeatures* pFeatures);

// Get physical device queue family properties
KxResult getPhysicalDeviceQueueFamilyProperties(VkFunctionTable& functions,
                                                 void* physicalDeviceHandle,
                                                 uint32_t* pQueueFamilyPropertyCount,
                                                 KxQueueFamilyProperties* pQueueFamilyProperties);

// Get physical device memory properties
KxResult getPhysicalDeviceMemoryProperties(VkFunctionTable& functions,
                                            void* physicalDeviceHandle,
                                            KxPhysicalDeviceMemoryProperties* pMemoryProperties);

} // namespace VkPhysicalDeviceUtil

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKPHYSICALDEVICE_H