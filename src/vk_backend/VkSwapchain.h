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
#include <optional>

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

    // Recreate swapchain (e.g., on window resize)
    bool recreate(VkFunctionTable& functions,
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
    VkSurfaceKHR surface() const noexcept { return surface_; }
    VkPresentModeKHR present_mode() const noexcept { return present_mode_; }
    VkSurfaceTransformFlagBitsKHR pre_transform() const noexcept { return pre_transform_; }
    VkCompositeAlphaFlagBitsKHR composite_alpha() const noexcept { return composite_alpha_; }
    bool clipped() const noexcept { return clipped_; }

    // Acquire next image
    VkResult acquire_next_image(VkFunctionTable& functions,
                                uint64_t timeout,
                                VkSemaphore semaphore,
                                VkFence fence,
                                uint32_t* p_image_index);

    // Present with optional wait semaphore
    VkResult present(VkFunctionTable& functions,
                     VkQueue queue,
                     uint32_t image_index,
                     VkSemaphore wait_semaphore);

    // Get current surface capabilities (for resize handling)
    bool get_surface_capabilities(VkFunctionTable& functions,
                                  VkPhysicalDevice physical_device,
                                  VkSurfaceCapabilitiesKHR* caps);

    // Query surface formats and present modes
    bool get_surface_formats(VkFunctionTable& functions,
                             VkPhysicalDevice physical_device,
                             std::vector<VkSurfaceFormatKHR>* formats);
    bool get_surface_present_modes(VkFunctionTable& functions,
                                   VkPhysicalDevice physical_device,
                                   std::vector<VkPresentModeKHR>* present_modes);

private:
    void cleanup_old_resources(VkFunctionTable& functions);

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkFormat image_format_ = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR color_space_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    VkExtent2D extent_ = {0, 0};
    std::vector<VkImage> images_;
    std::vector<VkImageView> image_views_;
    uint32_t image_count_ = 0;
    
    // Swapchain creation parameters (stored for recreation)
    VkPresentModeKHR present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
    VkSurfaceTransformFlagBitsKHR pre_transform_ = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkCompositeAlphaFlagBitsKHR composite_alpha_ = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    bool clipped_ = true;
    uint32_t min_image_count_ = 2;
    uint32_t image_array_layers_ = 1;
    VkImageUsageFlags image_usage_ = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkSharingMode image_sharing_mode_ = VK_SHARING_MODE_EXCLUSIVE;
    std::vector<uint32_t> queue_family_indices_;
    VkSwapchainCreateFlagsKHR flags_ = 0;
    VkSwapchainKHR old_swapchain_ = VK_NULL_HANDLE;
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
    bool is_timeline() const noexcept { return is_timeline_; }

    // Timeline semaphore operations
    VkResult get_counter_value(VkFunctionTable& functions, uint64_t* value);
    VkResult wait(VkFunctionTable& functions, uint64_t value, uint64_t timeout);
    VkResult signal(VkFunctionTable& functions, uint64_t value);

private:
    VkSemaphore semaphore_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    uint64_t current_value_ = 0;
    bool is_timeline_ = false;
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

    // Submit helpers for timeline semaphore support
    VkResult submit_legacy(VkFunctionTable& functions,
                           uint32_t submit_count,
                           const KxSubmitInfo* submits,
                           VkFence fence);
    VkResult submit_v2(VkFunctionTable& functions,
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