/*
 * VkPhysicalDevice.cpp
 *
 * Vulkan physical device wrapper implementation.
 */

#include "VkPhysicalDevice.h"

#include <cstring>
#include <algorithm>
#include <vector>

namespace kx {

// ============================================================================
// VkPhysicalDeviceImpl implementation
// ============================================================================

VkPhysicalDeviceImpl::VkPhysicalDeviceImpl(void* handle, uint32_t index)
    : handle_(handle), index_(index) {}

bool VkPhysicalDeviceImpl::initialize(VkFunctionTable& functions) {
    VkPhysicalDevice vkPhysicalDevice = reinterpret_cast<VkPhysicalDevice>(handle_);

    // Properties
    VkPhysicalDeviceProperties vkProps{};
    functions.vkGetPhysicalDeviceProperties(vkPhysicalDevice, &vkProps);

    properties_.apiVersion = vkProps.apiVersion;
    properties_.driverVersion = vkProps.driverVersion;
    properties_.vendorID = vkProps.vendorID;
    properties_.deviceID = vkProps.deviceID;
    properties_.deviceType = static_cast<KxPhysicalDeviceType>(vkProps.deviceType);
    strncpy(properties_.deviceName, vkProps.deviceName, sizeof(properties_.deviceName) - 1);
    // KxPhysicalDeviceProperties doesn't have pipelineCacheUUID

    // Features - only copy fields that exist in KxPhysicalDeviceFeatures
    VkPhysicalDeviceFeatures vkFeatures{};
    functions.vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &vkFeatures);

    features_.shaderFloat64 = vkFeatures.shaderFloat64;
    features_.shaderInt64 = vkFeatures.shaderInt64;
    features_.geometryShader = vkFeatures.geometryShader;
    features_.tessellationShader = vkFeatures.tessellationShader;
    features_.textureCompressionBC = vkFeatures.textureCompressionBC;
    features_.textureCompressionETC2 = vkFeatures.textureCompressionETC2;
    features_.textureCompressionASTC_LDR = vkFeatures.textureCompressionASTC_LDR;
    features_.pipelineStatisticsQuery = vkFeatures.pipelineStatisticsQuery;
    features_.vertexPipelineStoresAndAtomics = vkFeatures.vertexPipelineStoresAndAtomics;

    // Queue families
    uint32_t queueFamilyCount = 0;
    functions.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> vkQueueFamilies(queueFamilyCount);
    functions.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, vkQueueFamilies.data());

    queueFamilies_.resize(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        queueFamilies_[i].queueFlags = vkQueueFamilies[i].queueFlags;
        queueFamilies_[i].queueCount = vkQueueFamilies[i].queueCount;
        queueFamilies_[i].timestampValidBits = vkQueueFamilies[i].timestampValidBits;
        // KxQueueFamilyProperties doesn't have minImageTransferGranularity
    }

    // Memory properties
    VkPhysicalDeviceMemoryProperties vkMemProps{};
    functions.vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkMemProps);

    memoryHeaps_.resize(vkMemProps.memoryHeapCount);
    for (uint32_t i = 0; i < vkMemProps.memoryHeapCount; ++i) {
        memoryHeaps_[i].size = vkMemProps.memoryHeaps[i].size;
        // KxMemoryHeap doesn't have flags, has memoryTypeIndex instead
        memoryHeaps_[i].memoryTypeIndex = 0; // Will be filled by memory types
    }

    memoryTypes_.resize(vkMemProps.memoryTypeCount);
    for (uint32_t i = 0; i < vkMemProps.memoryTypeCount; ++i) {
        memoryTypes_[i].heapIndex = vkMemProps.memoryTypes[i].heapIndex;
        memoryTypes_[i].propertyFlags = vkMemProps.memoryTypes[i].propertyFlags;
    }

    return true;
}

// ============================================================================
// VkPhysicalDeviceUtil implementation
// ============================================================================

namespace VkPhysicalDeviceUtil {

KxResult enumeratePhysicalDevices(VkFunctionTable& functions,
                                   void* instanceHandle,
                                   uint32_t* pPhysicalDeviceCount,
                                   PhysicalDeviceInfo* pPhysicalDevices) {
    if (!instanceHandle || !pPhysicalDeviceCount) return KX_ERROR_INVALID_ARGUMENT;

    uint32_t count = 0;
    VkResult result = functions.vkEnumeratePhysicalDevices(
        reinterpret_cast<VkInstance>(instanceHandle), &count, nullptr);
    if (result != VK_SUCCESS) {
        return static_cast<KxResult>(result);
    }

    if (!pPhysicalDevices) {
        *pPhysicalDeviceCount = count;
        return KX_SUCCESS;
    }

    std::vector<VkPhysicalDevice> vkDevices(count);
    result = functions.vkEnumeratePhysicalDevices(
        reinterpret_cast<VkInstance>(instanceHandle), &count, vkDevices.data());
    if (result != VK_SUCCESS) {
        return static_cast<KxResult>(result);
    }

    *pPhysicalDeviceCount = std::min(*pPhysicalDeviceCount, count);

    for (uint32_t i = 0; i < *pPhysicalDeviceCount; ++i) {
        VkPhysicalDeviceImpl wrapper(vkDevices[i], i);
        wrapper.initialize(functions);

        pPhysicalDevices[i].handle = reinterpret_cast<uint64_t>(vkDevices[i]);
        pPhysicalDevices[i].properties = wrapper.getProperties();
        pPhysicalDevices[i].features = wrapper.getFeatures();
        pPhysicalDevices[i].queueFamilies = wrapper.getQueueFamilies();
        pPhysicalDevices[i].memoryHeaps = wrapper.getMemoryHeaps();
        pPhysicalDevices[i].memoryTypes = wrapper.getMemoryTypes();
    }

    return KX_SUCCESS;
}

KxResult getPhysicalDeviceProperties(VkFunctionTable& functions,
                                      void* physicalDeviceHandle,
                                      KxPhysicalDeviceProperties* pProperties) {
    if (!physicalDeviceHandle || !pProperties) return KX_ERROR_INVALID_ARGUMENT;

    VkPhysicalDeviceImpl wrapper(physicalDeviceHandle, 0);
    if (!wrapper.initialize(functions)) {
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    *pProperties = wrapper.getProperties();
    return KX_SUCCESS;
}

KxResult getPhysicalDeviceFeatures(VkFunctionTable& functions,
                                    void* physicalDeviceHandle,
                                    KxPhysicalDeviceFeatures* pFeatures) {
    if (!physicalDeviceHandle || !pFeatures) return KX_ERROR_INVALID_ARGUMENT;

    VkPhysicalDeviceImpl wrapper(physicalDeviceHandle, 0);
    if (!wrapper.initialize(functions)) {
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    *pFeatures = wrapper.getFeatures();
    return KX_SUCCESS;
}

KxResult getPhysicalDeviceQueueFamilyProperties(VkFunctionTable& functions,
                                                 void* physicalDeviceHandle,
                                                 uint32_t* pQueueFamilyPropertyCount,
                                                 KxQueueFamilyProperties* pQueueFamilyProperties) {
    if (!physicalDeviceHandle || !pQueueFamilyPropertyCount) return KX_ERROR_INVALID_ARGUMENT;

    VkPhysicalDeviceImpl wrapper(physicalDeviceHandle, 0);
    if (!wrapper.initialize(functions)) {
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    const auto& queueFamilies = wrapper.getQueueFamilies();
    *pQueueFamilyPropertyCount = static_cast<uint32_t>(queueFamilies.size());

    if (pQueueFamilyProperties) {
        uint32_t count = std::min(*pQueueFamilyPropertyCount, static_cast<uint32_t>(queueFamilies.size()));
        for (uint32_t i = 0; i < count; ++i) {
            pQueueFamilyProperties[i] = queueFamilies[i];
        }
    }

    return KX_SUCCESS;
}

KxResult getPhysicalDeviceMemoryProperties(VkFunctionTable& functions,
                                            void* physicalDeviceHandle,
                                            KxPhysicalDeviceMemoryProperties* pMemoryProperties) {
    if (!physicalDeviceHandle || !pMemoryProperties) return KX_ERROR_INVALID_ARGUMENT;

    VkPhysicalDeviceImpl wrapper(physicalDeviceHandle, 0);
    if (!wrapper.initialize(functions)) {
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    const auto& heaps = wrapper.getMemoryHeaps();
    const auto& types = wrapper.getMemoryTypes();

    pMemoryProperties->memoryHeapCount = static_cast<uint32_t>(heaps.size());
    pMemoryProperties->memoryTypeCount = static_cast<uint32_t>(types.size());

    for (uint32_t i = 0; i < pMemoryProperties->memoryHeapCount; ++i) {
        pMemoryProperties->memoryHeaps[i] = heaps[i];
    }
    for (uint32_t i = 0; i < pMemoryProperties->memoryTypeCount; ++i) {
        pMemoryProperties->memoryTypes[i] = types[i];
    }

    return KX_SUCCESS;
}

} // namespace VkPhysicalDeviceUtil

} // namespace kx