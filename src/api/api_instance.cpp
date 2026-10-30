/*
 * api_instance.cpp
 *
 * Instance and PhysicalDevice C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "DriverRegistry.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "SoftwareDriver.h"

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

#ifdef __cplusplus
} /* extern "C" */
#endif