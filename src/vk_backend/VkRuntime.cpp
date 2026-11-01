/*
 * VkRuntime.cpp
 *
 * Vulkan backend implementation of IRuntime.
 * Delegates to modular components: VkLoader, VkInstance, VkPhysicalDevice, VkDevice.
 */

#include "VkRuntime.h"

#include <cstring>
#include <algorithm>

#include "BackendRegistry.h"

namespace kx {

// ============================================================================
// VkRuntime implementation
// ============================================================================

VkRuntime::VkRuntime() {
    // Load Vulkan library and global functions
    loader_.load();
}

VkRuntime::~VkRuntime() {
    // Unload Vulkan library
    loader_.unload();
}

const char* VkRuntime::getBackendName() const noexcept {
    return "Vulkan";
}

BackendType VkRuntime::getBackendType() const noexcept {
    return BackendType::VULKAN;
}

uint32_t VkRuntime::getBackendVersion() const noexcept {
    return 1; // Version 1
}

KxResult VkRuntime::createInstance(const InstanceCreateInfo& info, Instance** outInstance) {
    if (!outInstance) return KX_ERROR_INVALID_ARGUMENT;
    if (!loader_.isLoaded()) return KX_ERROR_INITIALIZATION_FAILED;

    auto& functions = loader_.getFunctionTable();
    return VkInstanceUtil::createInstance(info, functions, 
        reinterpret_cast<VkInstanceImpl**>(outInstance));
}

void VkRuntime::destroyInstance(Instance* instance) {
    if (!instance) return;

    auto* vkInstance = static_cast<VkInstanceImpl*>(instance);
    auto& functions = loader_.getFunctionTable();
    VkInstanceUtil::destroyInstance(vkInstance, functions);
}

KxResult VkRuntime::enumeratePhysicalDevices(Instance* instance,
                                              uint32_t* pPhysicalDeviceCount,
                                              PhysicalDeviceInfo* pPhysicalDevices) {
    if (!instance || !pPhysicalDeviceCount) return KX_ERROR_INVALID_ARGUMENT;

    auto* vkInstance = static_cast<VkInstanceImpl*>(instance);
    auto& functions = loader_.getFunctionTable();

    return VkPhysicalDeviceUtil::enumeratePhysicalDevices(
        functions, vkInstance->getHandle(), pPhysicalDeviceCount, pPhysicalDevices);
}

KxResult VkRuntime::getPhysicalDeviceProperties(Instance* instance,
                                                 uint64_t physicalDeviceHandle,
                                                 KxPhysicalDeviceProperties* pProperties) {
    if (!instance || !pProperties) return KX_ERROR_INVALID_ARGUMENT;

    auto& functions = loader_.getFunctionTable();
    return VkPhysicalDeviceUtil::getPhysicalDeviceProperties(
        functions, reinterpret_cast<void*>(physicalDeviceHandle), pProperties);
}

KxResult VkRuntime::getPhysicalDeviceFeatures(Instance* instance,
                                               uint64_t physicalDeviceHandle,
                                               KxPhysicalDeviceFeatures* pFeatures) {
    if (!instance || !pFeatures) return KX_ERROR_INVALID_ARGUMENT;

    auto& functions = loader_.getFunctionTable();
    return VkPhysicalDeviceUtil::getPhysicalDeviceFeatures(
        functions, reinterpret_cast<void*>(physicalDeviceHandle), pFeatures);
}

KxResult VkRuntime::getPhysicalDeviceQueueFamilyProperties(Instance* instance,
                                                            uint64_t physicalDeviceHandle,
                                                            uint32_t* pQueueFamilyPropertyCount,
                                                            KxQueueFamilyProperties* pQueueFamilyProperties) {
    if (!instance || !pQueueFamilyPropertyCount) return KX_ERROR_INVALID_ARGUMENT;

    auto& functions = loader_.getFunctionTable();
    return VkPhysicalDeviceUtil::getPhysicalDeviceQueueFamilyProperties(
        functions, reinterpret_cast<void*>(physicalDeviceHandle),
        pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

KxResult VkRuntime::getPhysicalDeviceMemoryProperties(Instance* instance,
                                                       uint64_t physicalDeviceHandle,
                                                       KxPhysicalDeviceMemoryProperties* pMemoryProperties) {
    if (!instance || !pMemoryProperties) return KX_ERROR_INVALID_ARGUMENT;

    auto& functions = loader_.getFunctionTable();
    return VkPhysicalDeviceUtil::getPhysicalDeviceMemoryProperties(
        functions, reinterpret_cast<void*>(physicalDeviceHandle), pMemoryProperties);
}

KxResult VkRuntime::createDevice(uint64_t physicalDeviceHandle,
                                  const DeviceCreateInfo& info,
                                  Device** outDevice) {
    if (!outDevice) return KX_ERROR_INVALID_ARGUMENT;
    if (!loader_.isLoaded()) return KX_ERROR_INITIALIZATION_FAILED;

    auto& functions = loader_.getFunctionTable();
    return VkDeviceUtil::createDevice(functions,
                                       reinterpret_cast<void*>(physicalDeviceHandle),
                                       info,
                                       reinterpret_cast<VkDeviceImpl**>(outDevice));
}

KxResult VkRuntime::compileShader(const ShaderSourceCreateInfo& info, ShaderModule** outModule) {
    if (!outModule) return KX_ERROR_INVALID_ARGUMENT;

    // For now, compile to SPIR-V and create shader module
    std::vector<uint32_t> spirv;
    KxResult result = compileShaderToSPIRV(info, &spirv);
    if (result != KX_SUCCESS) {
        return result;
    }

    // TODO: Create VkShaderModule wrapper
    return KX_SUCCESS;
}

KxResult VkRuntime::compileShaderToSPIRV(const ShaderSourceCreateInfo& info,
                                          std::vector<uint32_t>* outSpirv) {
    if (!outSpirv) return KX_ERROR_INVALID_ARGUMENT;

    // Use the existing shader compiler to compile to SPIR-V
    // This would integrate with the shader compiler from earlier phases
    (void)info;
    outSpirv->clear();
    return KX_SUCCESS;
}

bool VkRuntime::isExtensionSupported(const char* extensionName) const noexcept {
    static const char* supportedExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
    };

    for (const char* ext : supportedExtensions) {
        if (strcmp(extensionName, ext) == 0) return true;
    }
    return false;
}

KxResult VkRuntime::getExtensionProperties(uint32_t* pPropertyCount, void* pProperties) {
    if (!pPropertyCount) return KX_ERROR_INVALID_ARGUMENT;

    static const KxExtensionProperties props[] = {
        {VK_KHR_SWAPCHAIN_EXTENSION_NAME, 1},
        {VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, 1},
        {VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, 1},
        {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, 1},
        {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, 1},
        {VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, 1},
    };

    const uint32_t count = static_cast<uint32_t>(sizeof(props) / sizeof(props[0]));

    if (!pProperties) {
        *pPropertyCount = count;
        return KX_SUCCESS;
    }

    uint32_t copyCount = std::min(*pPropertyCount, count);
    std::memcpy(pProperties, props, copyCount * sizeof(KxExtensionProperties));
    *pPropertyCount = copyCount;

    return KX_SUCCESS;
}

KxResult VkRuntime::createThreadPool(const ThreadPoolCreateInfo& info, ThreadPool** outThreadPool) {
    (void)info;
    (void)outThreadPool;
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void VkRuntime::destroyThreadPool(ThreadPool* threadPool) {
    (void)threadPool;
}

// ============================================================================
// Backend registration
// ============================================================================

extern "C" KX_API void KX_CALL kxRegisterVulkanBackend() {
    BackendRegistry::instance().registerBackend<BackendType::VULKAN, VkRuntime>();
}

} // namespace kx