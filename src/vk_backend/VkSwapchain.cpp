/*
 * VkSwapchain.cpp
 *
 * Vulkan swapchain, semaphore, fence, and queue implementation.
 */

#include "VkSwapchain.h"

#include <vector>
#include <algorithm>

namespace kx {

// ============================================================================
// VkSwapchainImpl
// ============================================================================

VkSwapchainImpl::~VkSwapchainImpl() {
    if (swapchain_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkSwapchainImpl::VkSwapchainImpl(VkSwapchainImpl&& other) noexcept
    : swapchain_(other.swapchain_)
    , device_(other.device_)
    , surface_(other.surface_)
    , image_format_(other.image_format_)
    , extent_(other.extent_)
    , images_(std::move(other.images_))
    , image_views_(std::move(other.image_views_))
    , image_count_(other.image_count_) {
    other.swapchain_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.surface_ = VK_NULL_HANDLE;
    other.image_format_ = VK_FORMAT_UNDEFINED;
    other.extent_ = {0, 0};
    other.image_count_ = 0;
}

VkSwapchainImpl& VkSwapchainImpl::operator=(VkSwapchainImpl&& other) noexcept {
    if (this != &other) {
        swapchain_ = other.swapchain_;
        device_ = other.device_;
        surface_ = other.surface_;
        image_format_ = other.image_format_;
        extent_ = other.extent_;
        images_ = std::move(other.images_);
        image_views_ = std::move(other.image_views_);
        image_count_ = other.image_count_;
        other.swapchain_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.surface_ = VK_NULL_HANDLE;
        other.image_format_ = VK_FORMAT_UNDEFINED;
        other.extent_ = {0, 0};
        other.image_count_ = 0;
    }
    return *this;
}

bool VkSwapchainImpl::initialize(VkFunctionTable& functions,
                                 VkDevice device,
                                 VkPhysicalDevice physical_device,
                                 VkSurfaceKHR surface,
                                 const KxSwapchainCreateInfo* create_info) {
    device_ = device;
    surface_ = surface;

    // Get surface capabilities
    VkSurfaceCapabilitiesKHR caps{};
    VkResult result = functions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
    if (result != VK_SUCCESS) return false;

    // Get surface formats
    uint32_t format_count = 0;
    result = functions.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (result != VK_SUCCESS || format_count == 0) return false;

    std::vector<VkSurfaceFormatKHR> formats(format_count);
    result = functions.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());
    if (result != VK_SUCCESS) return false;

    // Choose format
    VkSurfaceFormatKHR chosen_format = formats[0];
    for (const auto& fmt : formats) {
        if (fmt.format == static_cast<VkFormat>(create_info->imageFormat) &&
            fmt.colorSpace == static_cast<VkColorSpaceKHR>(create_info->imageColorSpace)) {
            chosen_format = fmt;
            break;
        }
    }
    image_format_ = chosen_format.format;

    // Get present modes
    uint32_t present_mode_count = 0;
    result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (result != VK_SUCCESS || present_mode_count == 0) return false;

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
    if (result != VK_SUCCESS) return false;

    // Choose present mode
    VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : present_modes) {
        if (mode == static_cast<VkPresentModeKHR>(create_info->presentMode)) {
            chosen_present_mode = mode;
            break;
        }
    }

    // Determine image count
    uint32_t image_count = create_info->minImageCount;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }
    if (image_count < caps.minImageCount) {
        image_count = caps.minImageCount;
    }

    // Choose extent
    VkExtent2D extent = create_info->imageExtent;
    if (caps.currentExtent.width != 0xFFFFFFFF) {
        extent = caps.currentExtent;
    } else {
        extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    extent_ = extent;

    // Choose transform
    VkSurfaceTransformFlagBitsKHR transform = create_info->preTransform;
    if (!(caps.supportedTransforms & transform)) {
        transform = caps.currentTransform;
    }

    // Choose composite alpha
    VkCompositeAlphaFlagBitsKHR composite_alpha = static_cast<VkCompositeAlphaFlagBitsKHR>(create_info->compositeAlpha);
    if (!(caps.supportedCompositeAlpha & composite_alpha)) {
        composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    // Create swapchain
    VkSwapchainCreateInfoKHR vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vk_info.flags = static_cast<VkSwapchainCreateFlagsKHR>(create_info->flags);
    vk_info.surface = surface;
    vk_info.minImageCount = image_count;
    vk_info.imageFormat = image_format_;
    vk_info.imageColorSpace = chosen_format.colorSpace;
    vk_info.imageExtent = extent_;
    vk_info.imageArrayLayers = create_info->imageArrayLayers;
    vk_info.imageUsage = static_cast<VkImageUsageFlags>(create_info->imageUsage);
    vk_info.imageSharingMode = static_cast<VkSharingMode>(create_info->imageSharingMode);
    vk_info.queueFamilyIndexCount = create_info->queueFamilyIndexCount;
    vk_info.pQueueFamilyIndices = create_info->pQueueFamilyIndices;
    vk_info.preTransform = transform;
    vk_info.compositeAlpha = composite_alpha;
    vk_info.presentMode = chosen_present_mode;
    vk_info.clipped = create_info->clipped;
    vk_info.oldSwapchain = static_cast<VkSwapchainKHR>(create_info->oldSwapchain);

    result = functions.vkCreateSwapchainKHR(device, &vk_info, nullptr, &swapchain_);
    if (result != VK_SUCCESS) return false;

    // Get swapchain images
    uint32_t actual_count = 0;
    result = functions.vkGetSwapchainImagesKHR(device, swapchain_, &actual_count, nullptr);
    if (result != VK_SUCCESS) return false;

    images_.resize(actual_count);
    result = functions.vkGetSwapchainImagesKHR(device, swapchain_, &actual_count, images_.data());
    if (result != VK_SUCCESS) return false;

    // Create image views for swapchain images
    image_views_.resize(actual_count);
    for (uint32_t i = 0; i < actual_count; ++i) {
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = images_[i];
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = image_format_;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        result = functions.vkCreateImageView(device, &view_info, nullptr, &image_views_[i]);
        if (result != VK_SUCCESS) return false;
    }

    image_count_ = actual_count;
    return true;
}

VkResult VkSwapchainImpl::acquire_next_image(VkFunctionTable& functions,
                                             uint64_t timeout,
                                             VkSemaphore semaphore,
                                             VkFence fence,
                                             uint32_t* p_image_index) {
    return functions.vkAcquireNextImageKHR(device_, swapchain_, timeout, semaphore, fence, p_image_index);
}

VkResult VkSwapchainImpl::present(VkFunctionTable& functions,
                                  VkQueue queue,
                                  uint32_t image_index,
                                  VkSemaphore wait_semaphore) {
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = wait_semaphore != VK_NULL_HANDLE ? 1 : 0;
    present_info.pWaitSemaphores = wait_semaphore != VK_NULL_HANDLE ? &wait_semaphore : nullptr;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain_;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    return functions.vkQueuePresentKHR(queue, &present_info);
}

// ============================================================================
// VkSemaphoreImpl
// ============================================================================

VkSemaphoreImpl::~VkSemaphoreImpl() {
    if (semaphore_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkSemaphoreImpl::VkSemaphoreImpl(VkSemaphoreImpl&& other) noexcept
    : semaphore_(other.semaphore_)
    , device_(other.device_)
    , current_value_(other.current_value_) {
    other.semaphore_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.current_value_ = 0;
}

VkSemaphoreImpl& VkSemaphoreImpl::operator=(VkSemaphoreImpl&& other) noexcept {
    if (this != &other) {
        semaphore_ = other.semaphore_;
        device_ = other.device_;
        current_value_ = other.current_value_;
        other.semaphore_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.current_value_ = 0;
    }
    return *this;
}

bool VkSemaphoreImpl::initialize(VkFunctionTable& functions,
                                 VkDevice device,
                                 const KxSemaphoreCreateInfo* create_info) {
    device_ = device;
    current_value_ = create_info->initialValue;

    VkSemaphoreCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vk_info.flags = static_cast<VkSemaphoreCreateFlags>(create_info->flags);

    // Timeline semaphore support
    VkSemaphoreTypeCreateInfo timeline_info{};
    if (current_value_ > 0) {
        timeline_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timeline_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timeline_info.initialValue = current_value_;
        vk_info.pNext = &timeline_info;
    }

    VkResult result = functions.vkCreateSemaphore(device, &vk_info, nullptr, &semaphore_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkFenceImpl
// ============================================================================

VkFenceImpl::~VkFenceImpl() {
    if (fence_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkFenceImpl::VkFenceImpl(VkFenceImpl&& other) noexcept
    : fence_(other.fence_)
    , device_(other.device_) {
    other.fence_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkFenceImpl& VkFenceImpl::operator=(VkFenceImpl&& other) noexcept {
    if (this != &other) {
        fence_ = other.fence_;
        device_ = other.device_;
        other.fence_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkFenceImpl::initialize(VkFunctionTable& functions,
                             VkDevice device,
                             const KxFenceCreateInfo* create_info) {
    device_ = device;

    VkFenceCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vk_info.flags = static_cast<VkFenceCreateFlags>(create_info->flags);

    VkResult result = functions.vkCreateFence(device, &vk_info, nullptr, &fence_);
    return result == VK_SUCCESS;
}

VkResult VkFenceImpl::wait(VkFunctionTable& functions, uint64_t timeout) {
    return functions.vkWaitForFences(device_, 1, &fence_, VK_TRUE, timeout);
}

VkResult VkFenceImpl::reset(VkFunctionTable& functions) {
    return functions.vkResetFences(device_, 1, &fence_);
}

VkResult VkFenceImpl::get_status(VkFunctionTable& functions) {
    return functions.vkGetFenceStatus(device_, fence_);
}

// ============================================================================
// VkQueueImpl
// ============================================================================

VkQueueImpl::VkQueueImpl(VkQueueImpl&& other) noexcept
    : queue_(other.queue_)
    , family_index_(other.family_index_)
    , index_(other.index_) {
    other.queue_ = VK_NULL_HANDLE;
    other.family_index_ = 0;
    other.index_ = 0;
}

VkQueueImpl& VkQueueImpl::operator=(VkQueueImpl&& other) noexcept {
    if (this != &other) {
        queue_ = other.queue_;
        family_index_ = other.family_index_;
        index_ = other.index_;
        other.queue_ = VK_NULL_HANDLE;
        other.family_index_ = 0;
        other.index_ = 0;
    }
    return *this;
}

void VkQueueImpl::initialize(VkQueue queue, uint32_t family_index, uint32_t index) {
    queue_ = queue;
    family_index_ = family_index;
    index_ = index;
}

VkResult VkQueueImpl::submit(VkFunctionTable& functions,
                             uint32_t submit_count,
                             const KxSubmitInfo* submits,
                             VkFence fence) {
    // Convert KxSubmitInfo to VkSubmitInfo
    std::vector<VkSubmitInfo> vk_submits;
    vk_submits.reserve(submit_count);

    // We need temporary storage for semaphores and command buffers
    std::vector<VkSemaphore> wait_semaphores;
    std::vector<VkPipelineStageFlags> wait_stages;
    std::vector<VkCommandBuffer> command_buffers;
    std::vector<VkSemaphore> signal_semaphores;

    for (uint32_t i = 0; i < submit_count; ++i) {
        const KxSubmitInfo& kx_submit = submits[i];
        VkSubmitInfo vk_submit{};
        vk_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        // Wait semaphores
        if (kx_submit.waitSemaphoreCount > 0) {
            wait_semaphores.reserve(wait_semaphores.size() + kx_submit.waitSemaphoreCount);
            wait_stages.reserve(wait_stages.size() + kx_submit.waitSemaphoreCount);
            for (uint32_t j = 0; j < kx_submit.waitSemaphoreCount; ++j) {
                wait_semaphores.push_back(static_cast<VkSemaphore>(kx_submit.pWaitSemaphores[j]));
                wait_stages.push_back(static_cast<VkPipelineStageFlags>(kx_submit.pWaitDstStageMask[j]));
            }
            vk_submit.waitSemaphoreCount = kx_submit.waitSemaphoreCount;
            vk_submit.pWaitSemaphores = &wait_semaphores[wait_semaphores.size() - kx_submit.waitSemaphoreCount];
            vk_submit.pWaitDstStageMask = &wait_stages[wait_stages.size() - kx_submit.waitSemaphoreCount];
        }

        // Command buffers
        if (kx_submit.commandBufferCount > 0) {
            command_buffers.reserve(command_buffers.size() + kx_submit.commandBufferCount);
            for (uint32_t j = 0; j < kx_submit.commandBufferCount; ++j) {
                auto* cmd_impl = reinterpret_cast<VkCommandBufferImpl*>(kx_submit.pCommandBuffers[j]);
                if (cmd_impl) {
                    command_buffers.push_back(cmd_impl->handle());
                }
            }
            vk_submit.commandBufferCount = kx_submit.commandBufferCount;
            vk_submit.pCommandBuffers = &command_buffers[command_buffers.size() - kx_submit.commandBufferCount];
        }

        // Signal semaphores
        if (kx_submit.signalSemaphoreCount > 0) {
            signal_semaphores.reserve(signal_semaphores.size() + kx_submit.signalSemaphoreCount);
            for (uint32_t j = 0; j < kx_submit.signalSemaphoreCount; ++j) {
                signal_semaphores.push_back(static_cast<VkSemaphore>(kx_submit.pSignalSemaphores[j]));
            }
            vk_submit.signalSemaphoreCount = kx_submit.signalSemaphoreCount;
            vk_submit.pSignalSemaphores = &signal_semaphores[signal_semaphores.size() - kx_submit.signalSemaphoreCount];
        }

        vk_submits.push_back(vk_submit);
    }

    VkFence vk_fence = fence ? static_cast<VkFence>(fence) : VK_NULL_HANDLE;
    return functions.vkQueueSubmit(queue_, static_cast<uint32_t>(vk_submits.size()), vk_submits.data(), vk_fence);
}

VkResult VkQueueImpl::present(VkFunctionTable& functions,
                              const KxPresentInfoKHR* present_info) {
    VkPresentInfoKHR vk_present{};
    vk_present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Wait semaphores
    if (present_info->waitSemaphoreCount > 0) {
        std::vector<VkSemaphore> wait_semaphores(present_info->waitSemaphoreCount);
        for (uint32_t i = 0; i < present_info->waitSemaphoreCount; ++i) {
            wait_semaphores[i] = static_cast<VkSemaphore>(present_info->pWaitSemaphores[i]);
        }
        vk_present.waitSemaphoreCount = present_info->waitSemaphoreCount;
        vk_present.pWaitSemaphores = wait_semaphores.data();
    }

    // Swapchains
    std::vector<VkSwapchainKHR> swapchains(present_info->swapchainCount);
    for (uint32_t i = 0; i < present_info->swapchainCount; ++i) {
        swapchains[i] = static_cast<VkSwapchainKHR>(present_info->pSwapchains[i]);
    }
    vk_present.swapchainCount = present_info->swapchainCount;
    vk_present.pSwapchains = swapchains.data();

    // Image indices
    std::vector<uint32_t> image_indices(present_info->swapchainCount);
    for (uint32_t i = 0; i < present_info->swapchainCount; ++i) {
        image_indices[i] = present_info->pImageIndices[i];
    }
    vk_present.pImageIndices = image_indices.data();

    vk_present.pResults = nullptr; // TODO: Handle results

    return functions.vkQueuePresentKHR(queue_, &vk_present);
}

VkResult VkQueueImpl::wait_idle(VkFunctionTable& functions) {
    return functions.vkQueueWaitIdle(queue_);
}

} // namespace kx