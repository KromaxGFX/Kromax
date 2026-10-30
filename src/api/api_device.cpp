/*
 * api_device.cpp
 *
 * Device creation and queue C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"
#include "Queue.h"
#include "DriverRegistry.h"
#include "IDriver.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    kx::Queue* queue = dev->get_queue(queueFamilyIndex, queueIndex);
    if (!queue) return KX_ERROR_INITIALIZATION_FAILED;
    *pQueue = reinterpret_cast<KxQueue>(queue);
    return KX_SUCCESS;
}

#ifdef __cplusplus
} /* extern "C" */
#endif