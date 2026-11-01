#ifndef KROMAX_SRC_VK_BACKEND_VKINSTANCE_H
#define KROMAX_SRC_VK_BACKEND_VKINSTANCE_H

/*
 * VkInstance.h
 *
 * Vulkan instance wrapper and management.
 * Handles VkInstance creation, extension/layer management, and destruction.
 */

#include "IRuntime.h"
#include "VkLoader.h"

#include <vector>
#include <string>

namespace kx {

// Vulkan instance wrapper
class VkInstanceImpl final : public Instance {
public:
    explicit VkInstanceImpl(const InstanceCreateInfo& info);
    ~VkInstanceImpl() override;

    VkInstanceImpl(const VkInstanceImpl&) = delete;
    VkInstanceImpl& operator=(const VkInstanceImpl&) = delete;

    VkInstanceImpl(VkInstanceImpl&&) noexcept;
    VkInstanceImpl& operator=(VkInstanceImpl&&) noexcept;

    // Instance interface
    const InstanceCreateInfo& getCreateInfo() const noexcept { return createInfo_; }
    void* getHandle() const noexcept { return handle_; }
    void setHandle(void* h) noexcept { handle_ = h; }
    const std::vector<std::string>& getExtensions() const noexcept { return enabledExtensions_; }
    const std::vector<std::string>& getLayers() const noexcept { return enabledLayers_; }

    // Build VkInstanceCreateInfo from InstanceCreateInfo
    static VkInstanceCreateInfo buildCreateInfo(const InstanceCreateInfo& info,
                                                 const std::vector<const char*>& extensions,
                                                 const std::vector<const char*>& layers,
                                                 VkApplicationInfo* outAppInfo);

    // Collect required platform surface extensions
    static void collectSurfaceExtensions(std::vector<const char*>& extensions);

private:
    InstanceCreateInfo createInfo_;
    void* handle_ = nullptr;  // VkInstance
    std::vector<std::string> enabledExtensions_;
    std::vector<std::string> enabledLayers_;
};

// VkInstance management functions
namespace VkInstanceUtil {

// Create a Vulkan instance
KxResult createInstance(const InstanceCreateInfo& info,
                         VkFunctionTable& functions,
                         VkInstanceImpl** outInstance);

// Destroy a Vulkan instance
void destroyInstance(VkInstanceImpl* instance, VkFunctionTable& functions);

} // namespace VkInstanceUtil

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKINSTANCE_H