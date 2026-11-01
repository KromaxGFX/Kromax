#ifndef KROMAX_SRC_VK_BACKEND_VKSWAPCHAIN_H
#define KROMAX_SRC_VK_BACKEND_VKSWAPCHAIN_H

/*
 * VkSwapchain.h
 *
 * Vulkan swapchain, semaphore, fence, and queue wrappers.
 */

#include "VkLoader.h"
#include <kx_core.h>

#include <vector>
#include <cstdint>

namespace kx {

// Swapchain
class VkSwapchainImpl final {
public:
    VkSwapchainImpl() = default;
    ~VkSwapchainImpl();

    VkSwapchainImpl(const VkSwapchainImpl&) = delete;
    VkSwapchainImpl& operator=(const VkSwapchainImpl&) = delete;

    VkSwapchainImpl(VkSwapchainImpl&& other) noexcept;
    VkSwapchainImpl& operator=(VkSwapchainImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    VkPhysicalDevice physical_device,
                    VkSurfaceKHR surface,
                    const KxSwapchainCreateInfoKHR* create_info);

    VkSwapchainKHR handle() const noexcept { return swapchain_; }
    const std::vector<VkImage>& images() const noexcept { return images_; }
    const std::vector<VkImageView>& image_views() const noexcept { return image_views_; }
    VkFormat image_format() const noexcept { return image_format_; }
    VkExtent2D extent() const noexcept { return extent_; }
    uint32_t image_count() const noexcept { return static_cast<uint32_t>(images_.size()); }

    // Acquire next image
    VkResult acquire_next_image(VkFunctionTable& functions,
                                uint64_t timeout,
                                VkSemaphore semaphore,
                                VkFence fence,
                                uint32_t* p_image_index);

    // Present
    VkResult present(VkFunctionTable& functions,
                     VkQueue queue,
                     uint32_t image_index,
                     VkSemaphore wait_semaphore);

private:
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkFormat image_format_ = VK_FORMAT_UNDEFINED;
    VkExtent2D extent_ = {0, 0};
    std::vector<VkImage> images_;
    std::vector<VkImageView> image_views_;
    uint32_t image_count_ = 0;
};

// Semaphore
class VkSemaphoreImpl final {
public:
    VkSemaphoreImpl() = default;
    ~VkSemaphoreImpl();

    VkSemaphoreImpl(const VkSemaphoreImpl&) = delete;
    VkSemaphoreImpl& operator=(const VkSemaphoreImpl&) = delete;

    VkSemaphoreImpl(VkSemaphoreImpl&& other) noexcept;
    VkSemaphoreImpl& operator=(VkSemaphoreImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxSemaphoreCreateInfo* create_info);

    VkSemaphore handle() const noexcept { return semaphore_; }
    uint64_t current_value() const noexcept { return current_value_; }
    void set_current_value(uint64_t value) noexcept { current_value_ = value; }

private:
    VkSemaphore semaphore_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    uint64_t current_value_ = 0;
};

// Fence
class VkFenceImpl final {
public:
    VkFenceImpl() = default;
    ~VkFenceImpl();

    VkFenceImpl(const VkFenceImpl&) = delete;
    VkFenceImpl& operator=(const VkFenceImpl&) = delete;

    VkFenceImpl(VkFenceImpl&& other) noexcept;
    VkFenceImpl& operator=(VkFenceImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxFenceCreateInfo* create_info);

    VkFence handle() const noexcept { return fence_; }
    VkResult wait(VkFunctionTable& functions, uint64_t timeout);
    VkResult reset(VkFunctionTable& functions);
    VkResult get_status(VkFunctionTable& functions);

private:
    VkFence fence_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
};

// Queue
class VkQueueImpl final {
public:
    VkQueueImpl() = default;
    ~VkQueueImpl() = default;

    VkQueueImpl(const VkQueueImpl&) = delete;
    VkQueueImpl& operator=(const VkQueueImpl&) = delete;

    VkQueueImpl(VkQueueImpl&& other) noexcept;
    VkQueueImpl& operator=(VkQueueImpl&& other) noexcept;

    void initialize(VkQueue queue, uint32_t family_index, uint32_t index);

    VkQueue handle() const noexcept { return queue_; }
    uint32_t family_index() const noexcept { return family_index_; }
    uint32_t index() const noexcept { return index_; }

    VkResult submit(VkFunctionTable& functions,
                    uint32_t submit_count,
                    const KxSubmitInfo* submits,
                    VkFence fence);

    VkResult present(VkFunctionTable& functions,
                     const KxPresentInfoKHR* present_info);

    VkResult wait_idle(VkFunctionTable& functions);

private:
    VkQueue queue_ = VK_NULL_HANDLE;
    uint32_t family_index_ = 0;
    uint32_t index_ = 0;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKSWAPCHAIN_H