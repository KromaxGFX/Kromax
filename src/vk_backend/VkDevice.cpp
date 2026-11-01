/*
 * VkDevice.cpp
 *
 * Vulkan logical device wrapper implementation.
 */

#include "VkDevice.h"
#include "VkDriverDevice.h"

#include <vector>

namespace kx {

// ============================================================================
// VkDeviceImpl implementation
// ============================================================================

VkDeviceImpl::~VkDeviceImpl() {
    // The driver device will clean up its objects
    // The actual vkDestroyDevice is handled by VkDeviceUtil::destroyDevice
}

VkDeviceImpl::VkDeviceImpl(VkDeviceImpl&& other) noexcept
    : handle_(other.handle_)
    , physicalDeviceHandle_(other.physicalDeviceHandle_)
    , queues_(std::move(other.queues_))
    , createInfo_(std::move(other.createInfo_))
    , function_table_(other.function_table_)
    , driver_device_(std::move(other.driver_device_)) {
    other.handle_ = nullptr;
    other.physicalDeviceHandle_ = nullptr;
}

VkDeviceImpl& VkDeviceImpl::operator=(VkDeviceImpl&& other) noexcept {
    if (this != &other) {
        handle_ = other.handle_;
        physicalDeviceHandle_ = other.physicalDeviceHandle_;
        queues_ = std::move(other.queues_);
        createInfo_ = std::move(other.createInfo_);
        function_table_ = other.function_table_;
        driver_device_ = std::move(other.driver_device_);
        other.handle_ = nullptr;
        other.physicalDeviceHandle_ = nullptr;
    }
    return *this;
}

uint32_t VkDeviceImpl::getQueueFamilyIndex(uint32_t queueType) const noexcept {
    // Map Kx queue types to queue family indices
    if (queueType & KX_QUEUE_GRAPHICS_BIT) return 0;
    if (queueType & KX_QUEUE_COMPUTE_BIT) return 1;
    if (queueType & KX_QUEUE_TRANSFER_BIT) return 2;
    if (queueType & KX_QUEUE_SPARSE_BINDING_BIT) return 3;
    return 0;
}

bool VkDeviceImpl::initialize(VkFunctionTable& functions,
                               void* physicalDeviceHandle,
                               const DeviceCreateInfo& info) {
    physicalDeviceHandle_ = physicalDeviceHandle;
    createInfo_ = info;
    function_table_ = functions; // Copy the function table

    // Build queue create infos
    std::vector<VkDeviceQueueCreateInfo> vkQueueCreateInfos;
    vkQueueCreateInfos.reserve(info.queueCreateInfoCount);

    for (uint32_t i = 0; i < info.queueCreateInfoCount; ++i) {
        VkDeviceQueueCreateInfo qci{};
        qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qci.queueFamilyIndex = info.pQueueCreateInfos[i].queueFamilyIndex;
        qci.queueCount = info.pQueueCreateInfos[i].queueCount;
        qci.pQueuePriorities = info.pQueueCreateInfos[i].pQueuePriorities;
        vkQueueCreateInfos.push_back(qci);
    }

    // Required device extensions
    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    extensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
    extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    extensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    extensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

    // Add user-specified extensions
    for (uint32_t i = 0; i < info.enabledExtensionCount; ++i) {
        extensions.push_back(info.ppEnabledExtensionNames[i]);
    }

    // Features chain
    VkPhysicalDeviceFeatures2 features2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceVulkan11Features vk11Features{};
    vk11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features2.pNext = &vk11Features;

    VkPhysicalDeviceVulkan12Features vk12Features{};
    vk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vk11Features.pNext = &vk12Features;

    VkPhysicalDeviceVulkan13Features vk13Features{};
    vk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vk12Features.pNext = &vk13Features;

    // Enable required features
    vk11Features.shaderDrawParameters = VK_TRUE;
    vk12Features.bufferDeviceAddress = VK_TRUE;
    vk12Features.descriptorIndexing = VK_TRUE;
    vk12Features.runtimeDescriptorArray = VK_TRUE;
    vk12Features.descriptorBindingVariableDescriptorCount = VK_TRUE;
    vk12Features.descriptorBindingPartiallyBound = VK_TRUE;
    vk12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    vk12Features.timelineSemaphore = VK_TRUE;
    vk13Features.synchronization2 = VK_TRUE;
    vk13Features.dynamicRendering = VK_TRUE;

    if (info.pEnabledFeatures) {
        // Map KxPhysicalDeviceFeatures to Vulkan features
        // This is a simplified mapping - could be expanded
    }

    VkDeviceCreateInfo vkCreateInfo{};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkCreateInfo.flags = info.flags;
    vkCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vkQueueCreateInfos.size());
    vkCreateInfo.pQueueCreateInfos = vkQueueCreateInfos.data();
    vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vkCreateInfo.ppEnabledExtensionNames = extensions.data();
    vkCreateInfo.pEnabledFeatures = nullptr; // Using features2 instead
    vkCreateInfo.pNext = &features2;

    VkResult result = functions.vkCreateDevice(
        reinterpret_cast<VkPhysicalDevice>(physicalDeviceHandle_),
        &vkCreateInfo, nullptr,
        reinterpret_cast<VkDevice*>(&handle_));

    if (result != VK_SUCCESS) {
        return false;
    }

    // Get queue handles
    queues_.resize(info.queueCreateInfoCount);
    for (uint32_t i = 0; i < info.queueCreateInfoCount; ++i) {
        VkQueue queue = VK_NULL_HANDLE;
        functions.vkGetDeviceQueue(
            reinterpret_cast<VkDevice>(handle_),
            info.pQueueCreateInfos[i].queueFamilyIndex, 0, &queue);
        queues_[i] = queue;
    }

    // Create the driver device
    driver_device_ = std::make_unique<VkDriverDevice>(this);

    return true;
}

// ============================================================================
// VkDeviceUtil implementation
// ============================================================================

namespace VkDeviceUtil {

KxResult createDevice(VkFunctionTable& functions,
                       void* physicalDeviceHandle,
                       const DeviceCreateInfo& info,
                       VkDeviceImpl** outDevice) {
    if (!outDevice) return KX_ERROR_INVALID_ARGUMENT;
    if (!functions.vkCreateDevice) return KX_ERROR_INITIALIZATION_FAILED;

    auto* device = new VkDeviceImpl();
    if (!device->initialize(functions, physicalDeviceHandle, info)) {
        delete device;
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    *outDevice = device;
    return KX_SUCCESS;
}

void destroyDevice(VkDeviceImpl* device, VkFunctionTable& functions) {
    if (!device) return;

    if (device->getHandle() && functions.vkDestroyDevice) {
        functions.vkDestroyDevice(reinterpret_cast<VkDevice>(device->getHandle()), nullptr);
    }
    delete device;
}

} // namespace VkDeviceUtil

} // namespace kx