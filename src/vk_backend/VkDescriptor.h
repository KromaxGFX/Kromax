#ifndef KROMAX_SRC_VK_BACKEND_VKDESCRIPTOR_H
#define KROMAX_SRC_VK_BACKEND_VKDESCRIPTOR_H

/*
 * VkDescriptor.h
 *
 * Vulkan descriptor set layout, pool, and set wrappers.
 */

#include "VkLoader.h"
#include <kx_core.h>

#include <vector>
#include <unordered_map>

namespace kx {

// Descriptor set layout
class VkDescriptorSetLayoutImpl final {
public:
    VkDescriptorSetLayoutImpl() = default;
    ~VkDescriptorSetLayoutImpl();

    VkDescriptorSetLayoutImpl(const VkDescriptorSetLayoutImpl&) = delete;
    VkDescriptorSetLayoutImpl& operator=(const VkDescriptorSetLayoutImpl&) = delete;

    VkDescriptorSetLayoutImpl(VkDescriptorSetLayoutImpl&& other) noexcept;
    VkDescriptorSetLayoutImpl& operator=(VkDescriptorSetLayoutImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxDescriptorSetLayoutCreateInfo* create_info);

    VkDescriptorSetLayout handle() const noexcept { return layout_; }
    const std::vector<VkDescriptorSetLayoutBinding>& bindings() const noexcept { return bindings_; }

private:
    VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
};

// Descriptor pool
class VkDescriptorPoolImpl final {
public:
    VkDescriptorPoolImpl() = default;
    ~VkDescriptorPoolImpl();

    VkDescriptorPoolImpl(const VkDescriptorPoolImpl&) = delete;
    VkDescriptorPoolImpl& operator=(const VkDescriptorPoolImpl&) = delete;

    VkDescriptorPoolImpl(VkDescriptorPoolImpl&& other) noexcept;
    VkDescriptorPoolImpl& operator=(VkDescriptorPoolImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxDescriptorPoolCreateInfo* create_info);

    VkDescriptorPool handle() const noexcept { return pool_; }
    VkDevice device() const noexcept { return device_; }

private:
    VkDescriptorPool pool_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
};

// Descriptor set
class VkDescriptorSetImpl final {
public:
    VkDescriptorSetImpl() = default;
    ~VkDescriptorSetImpl();

    VkDescriptorSetImpl(const VkDescriptorSetImpl&) = delete;
    VkDescriptorSetImpl& operator=(const VkDescriptorSetImpl&) = delete;

    VkDescriptorSetImpl(VkDescriptorSetImpl&& other) noexcept;
    VkDescriptorSetImpl& operator=(VkDescriptorSetImpl&& other) noexcept;

    // Update descriptor set contents
    void update(VkFunctionTable& functions,
                uint32_t write_count,
                const KxWriteDescriptorSet* writes,
                uint32_t copy_count,
                const KxCopyDescriptorSet* copies);

    VkDescriptorSet handle() const noexcept { return set_; }
    VkDescriptorSetLayout layout() const noexcept { return layout_; }
    VkDescriptorPool pool() const noexcept { return pool_; }

    void set_layout(VkDescriptorSetLayout l) noexcept { layout_ = l; }
    void set_pool(VkDescriptorPool p) noexcept { pool_ = p; }

private:
    VkDescriptorSet set_ = VK_NULL_HANDLE;
    VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
    VkDescriptorPool pool_ = VK_NULL_HANDLE;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKDESCRIPTOR_H