#ifndef KROMAX_SRC_VK_BACKEND_VKDEVICE_H
#define KROMAX_SRC_VK_BACKEND_VKDEVICE_H

/*
 * VkDevice.h
 *
 * Vulkan logical device wrapper and management.
 * Handles VkDevice creation, queue management, and device-level operations.
 */

#include "IRuntime.h"
#include "VkLoader.h"

#include <vector>
#include <memory>

namespace kx {

// Forward declaration
class VkDriverDevice;

// Vulkan device wrapper
class VkDeviceImpl final : public Device {
public:
    VkDeviceImpl() = default;
    ~VkDeviceImpl() override;

    VkDeviceImpl(const VkDeviceImpl&) = delete;
    VkDeviceImpl& operator=(const VkDeviceImpl&) = delete;

    VkDeviceImpl(VkDeviceImpl&&) noexcept;
    VkDeviceImpl& operator=(VkDeviceImpl&&) noexcept;

    // Device interface
    void* getHandle() const noexcept { return handle_; }
    void* getPhysicalDeviceHandle() const noexcept { return physicalDeviceHandle_; }
    uint32_t getQueueFamilyIndex(uint32_t queueType) const noexcept;
    const std::vector<void*>& getQueues() const noexcept { return queues_; }
    VkDriverDevice* driver_device() const noexcept { return driver_device_.get(); }
    VkFunctionTable& getFunctionTable() noexcept { return function_table_; }
    const VkFunctionTable& getFunctionTable() const noexcept { return function_table_; }

    // Initialize logical device from physical device
    bool initialize(VkFunctionTable& functions,
                     void* physicalDeviceHandle,
                     const DeviceCreateInfo& info);

private:
    void* handle_ = nullptr;  // VkDevice
    void* physicalDeviceHandle_ = nullptr;
    std::vector<void*> queues_;  // VkQueue per family
    DeviceCreateInfo createInfo_;
    VkFunctionTable function_table_;
    std::unique_ptr<VkDriverDevice> driver_device_;
};

// Device creation and management utilities
namespace VkDeviceUtil {

// Create a logical device
KxResult createDevice(VkFunctionTable& functions,
                       void* physicalDeviceHandle,
                       const DeviceCreateInfo& info,
                       VkDeviceImpl** outDevice);

// Destroy a logical device
void destroyDevice(VkDeviceImpl* device, VkFunctionTable& functions);

} // namespace VkDeviceUtil

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKDEVICE_H