/*
 * VkSwapchain.cpp
 *
 * Vulkan swapchain, semaphore, fence, and queue implementation.
 */

#include "VkSwapchain.h"
#include "VkCommand.h"

#include <vector>
#include <algorithm>
#include <limits>

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
    , color_space_(other.color_space_)
    , extent_(other.extent_)
    , images_(std::move(other.images_))
    , image_views_(std::move(other.image_views_))
    , image_count_(other.image_count_)
    , present_mode_(other.present_mode_)
    , pre_transform_(other.pre_transform_)
    , composite_alpha_(other.composite_alpha_)
    , clipped_(other.clipped_)
    , min_image_count_(other.min_image_count_)
    , image_array_layers_(other.image_array_layers_)
    , image_usage_(other.image_usage_)
    , image_sharing_mode_(other.image_sharing_mode_)
    , queue_family_indices_(std::move(other.queue_family_indices_))
    , flags_(other.flags_)
    , old_swapchain_(other.old_swapchain_) {
    other.swapchain_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.surface_ = VK_NULL_HANDLE;
    other.image_format_ = VK_FORMAT_UNDEFINED;
    other.color_space_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    other.extent_ = {0, 0};
    other.image_count_ = 0;
    other.present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
    other.pre_transform_ = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    other.composite_alpha_ = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    other.clipped_ = true;
    other.min_image_count_ = 2;
    other.image_array_layers_ = 1;
    other.image_usage_ = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    other.image_sharing_mode_ = VK_SHARING_MODE_EXCLUSIVE;
    other.flags_ = 0;
    other.old_swapchain_ = VK_NULL_HANDLE;
}

VkSwapchainImpl& VkSwapchainImpl::operator=(VkSwapchainImpl&& other) noexcept {
    if (this != &other) {
        swapchain_ = other.swapchain_;
        device_ = other.device_;
        surface_ = other.surface_;
        image_format_ = other.image_format_;
        color_space_ = other.color_space_;
        extent_ = other.extent_;
        images_ = std::move(other.images_);
        image_views_ = std::move(other.image_views_);
        image_count_ = other.image_count_;
        present_mode_ = other.present_mode_;
        pre_transform_ = other.pre_transform_;
        composite_alpha_ = other.composite_alpha_;
        clipped_ = other.clipped_;
        min_image_count_ = other.min_image_count_;
        image_array_layers_ = other.image_array_layers_;
        image_usage_ = other.image_usage_;
        image_sharing_mode_ = other.image_sharing_mode_;
        queue_family_indices_ = std::move(other.queue_family_indices_);
        flags_ = other.flags_;
        old_swapchain_ = other.old_swapchain_;
        other.swapchain_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.surface_ = VK_NULL_HANDLE;
        other.image_format_ = VK_FORMAT_UNDEFINED;
        other.color_space_ = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        other.extent_ = {0, 0};
        other.image_count_ = 0;
        other.present_mode_ = VK_PRESENT_MODE_FIFO_KHR;
        other.pre_transform_ = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        other.composite_alpha_ = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        other.clipped_ = true;
        other.min_image_count_ = 2;
        other.image_array_layers_ = 1;
        other.image_usage_ = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        other.image_sharing_mode_ = VK_SHARING_MODE_EXCLUSIVE;
        other.flags_ = 0;
        other.old_swapchain_ = VK_NULL_HANDLE;
    }
    return *this;
}

void VkSwapchainImpl::cleanup_old_resources(VkFunctionTable& functions) {
    // Destroy old image views
    for (auto view : image_views_) {
        if (view != VK_NULL_HANDLE) {
            functions.vkDestroyImageView(device_, view, nullptr);
        }
    }
    image_views_.clear();
    images_.clear();
    
    // Destroy old swapchain if exists
    if (old_swapchain_ != VK_NULL_HANDLE) {
        functions.vkDestroySwapchainKHR(device_, old_swapchain_, nullptr);
        old_swapchain_ = VK_NULL_HANDLE;
    }
}

bool VkSwapchainImpl::initialize(VkFunctionTable& functions,
                                 VkDevice device,
                                 VkPhysicalDevice physical_device,
                                 VkSurfaceKHR surface,
                                 const KxSwapchainCreateInfoKHR* create_info) {
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
    color_space_ = chosen_format.colorSpace;

    // Get present modes
    uint32_t present_mode_count = 0;
    result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (result != VK_SUCCESS || present_mode_count == 0) return false;

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
    if (result != VK_SUCCESS) return false;

    // Choose present mode (prefer mailbox for low latency, fallback to FIFO)
    VkPresentModeKHR chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosen_present_mode = mode;
            break;
        } else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR && chosen_present_mode == VK_PRESENT_MODE_FIFO_KHR) {
            chosen_present_mode = mode;
        }
    }
    present_mode_ = chosen_present_mode;

    // Determine image count
    uint32_t image_count = create_info->minImageCount;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
        image_count = caps.maxImageCount;
    }
    if (image_count < caps.minImageCount) {
        image_count = caps.minImageCount;
    }
    min_image_count_ = image_count;

    // Choose extent
    VkExtent2D extent;
    extent.width = create_info->imageExtent.width;
    extent.height = create_info->imageExtent.height;
    if (caps.currentExtent.width != 0xFFFFFFFF) {
        extent = caps.currentExtent;
    } else {
        extent.width = std::clamp(extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    extent_ = extent;

    // Choose transform
    VkSurfaceTransformFlagBitsKHR transform = static_cast<VkSurfaceTransformFlagBitsKHR>(create_info->preTransform);
    if (!(caps.supportedTransforms & transform)) {
        transform = caps.currentTransform;
    }
    pre_transform_ = transform;

    // Choose composite alpha
    VkCompositeAlphaFlagBitsKHR composite_alpha = static_cast<VkCompositeAlphaFlagBitsKHR>(create_info->compositeAlpha);
    if (!(caps.supportedCompositeAlpha & composite_alpha)) {
        composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }
    composite_alpha_ = composite_alpha;

    clipped_ = create_info->clipped;
    image_array_layers_ = create_info->imageArrayLayers;
    image_usage_ = static_cast<VkImageUsageFlags>(create_info->imageUsage);
    image_sharing_mode_ = static_cast<VkSharingMode>(create_info->imageSharingMode);
    flags_ = static_cast<VkSwapchainCreateFlagsKHR>(create_info->flags);

    if (create_info->queueFamilyIndexCount > 0) {
        queue_family_indices_.assign(create_info->pQueueFamilyIndices, 
                                     create_info->pQueueFamilyIndices + create_info->queueFamilyIndexCount);
    }

    old_swapchain_ = create_info->oldSwapchain ? reinterpret_cast<VkSwapchainKHR>(create_info->oldSwapchain) : VK_NULL_HANDLE;

    // Create swapchain
    VkSwapchainCreateInfoKHR vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vk_info.flags = flags_;
    vk_info.surface = surface;
    vk_info.minImageCount = image_count;
    vk_info.imageFormat = image_format_;
    vk_info.imageColorSpace = color_space_;
    vk_info.imageExtent = extent_;
    vk_info.imageArrayLayers = image_array_layers_;
    vk_info.imageUsage = image_usage_;
    vk_info.imageSharingMode = image_sharing_mode_;
    vk_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices_.size());
    vk_info.pQueueFamilyIndices = queue_family_indices_.empty() ? nullptr : queue_family_indices_.data();
    vk_info.preTransform = pre_transform_;
    vk_info.compositeAlpha = composite_alpha_;
    vk_info.presentMode = present_mode_;
    vk_info.clipped = clipped_ ? VK_TRUE : VK_FALSE;
    vk_info.oldSwapchain = old_swapchain_;

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

bool VkSwapchainImpl::recreate(VkFunctionTable& functions,
                               VkDevice device,
                               VkPhysicalDevice physical_device,
                               VkSurfaceKHR surface,
                               const KxSwapchainCreateInfoKHR* create_info) {
    // Store old swapchain for recreation
    old_swapchain_ = swapchain_;
    
    // Cleanup old image views (but keep swapchain for VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR if needed)
    cleanup_old_resources(functions);
    
    // Re-initialize with new parameters
    return initialize(functions, device, physical_device, surface, create_info);
}

bool VkSwapchainImpl::get_surface_capabilities(VkFunctionTable& functions,
                                               VkPhysicalDevice physical_device,
                                               VkSurfaceCapabilitiesKHR* caps) {
    if (!caps) return false;
    VkResult result = functions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface_, caps);
    return result == VK_SUCCESS;
}

bool VkSwapchainImpl::get_surface_formats(VkFunctionTable& functions,
                                          VkPhysicalDevice physical_device,
                                          std::vector<VkSurfaceFormatKHR>* formats) {
    if (!formats) return false;
    
    uint32_t format_count = 0;
    VkResult result = functions.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, nullptr);
    if (result != VK_SUCCESS || format_count == 0) return false;

    formats->resize(format_count);
    result = functions.vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface_, &format_count, formats->data());
    return result == VK_SUCCESS;
}

bool VkSwapchainImpl::get_surface_present_modes(VkFunctionTable& functions,
                                                VkPhysicalDevice physical_device,
                                                std::vector<VkPresentModeKHR>* present_modes) {
    if (!present_modes) return false;
    
    uint32_t present_mode_count = 0;
    VkResult result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, nullptr);
    if (result != VK_SUCCESS || present_mode_count == 0) return false;

    present_modes->resize(present_mode_count);
    result = functions.vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface_, &present_mode_count, present_modes->data());
    return result == VK_SUCCESS;
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
    , current_value_(other.current_value_)
    , is_timeline_(other.is_timeline_) {
    other.semaphore_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.current_value_ = 0;
    other.is_timeline_ = false;
}

VkSemaphoreImpl& VkSemaphoreImpl::operator=(VkSemaphoreImpl&& other) noexcept {
    if (this != &other) {
        semaphore_ = other.semaphore_;
        device_ = other.device_;
        current_value_ = other.current_value_;
        is_timeline_ = other.is_timeline_;
        other.semaphore_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.current_value_ = 0;
        other.is_timeline_ = false;
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
    if (create_info->initialValue > 0 || (create_info->flags & VK_SEMAPHORE_TYPE_TIMELINE_KHR)) {
        is_timeline_ = true;
        timeline_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timeline_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timeline_info.initialValue = create_info->initialValue;
        vk_info.pNext = &timeline_info;
    }

    VkResult result = functions.vkCreateSemaphore(device, &vk_info, nullptr, &semaphore_);
    return result == VK_SUCCESS;
}

VkResult VkSemaphoreImpl::get_counter_value(VkFunctionTable& functions, uint64_t* value) {
    if (!is_timeline_ || !functions.vkGetSemaphoreCounterValueKHR) return VK_ERROR_FEATURE_NOT_PRESENT;
    VkResult result = functions.vkGetSemaphoreCounterValueKHR(device_, semaphore_, value);
    if (result == VK_SUCCESS) current_value_ = *value;
    return result;
}

VkResult VkSemaphoreImpl::wait(VkFunctionTable& functions, uint64_t value, uint64_t timeout) {
    if (!is_timeline_ || !functions.vkWaitSemaphoresKHR) return VK_ERROR_FEATURE_NOT_PRESENT;
    
    VkSemaphoreWaitInfoKHR wait_info{};
    wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore_;
    wait_info.pValues = &value;
    
    VkResult result = functions.vkWaitSemaphoresKHR(device_, &wait_info, timeout);
    if (result == VK_SUCCESS) current_value_ = value;
    return result;
}

VkResult VkSemaphoreImpl::signal(VkFunctionTable& functions, uint64_t value) {
    if (!is_timeline_ || !functions.vkSignalSemaphoreKHR) return VK_ERROR_FEATURE_NOT_PRESENT;
    
    VkSemaphoreSignalInfoKHR signal_info{};
    signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR;
    signal_info.semaphore = semaphore_;
    signal_info.value = value;
    
    VkResult result = functions.vkSignalSemaphoreKHR(device_, &signal_info);
    if (result == VK_SUCCESS) current_value_ = value;
    return result;
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
    // Check if any semaphore is timeline - if so, use VkSubmitInfo2KHR
    bool use_submit_v2 = false;
    for (uint32_t i = 0; i < submit_count && !use_submit_v2; ++i) {
        const KxSubmitInfo& kx_submit = submits[i];
        for (uint32_t j = 0; j < kx_submit.waitSemaphoreCount; ++j) {
            auto* sem = reinterpret_cast<VkSemaphoreImpl*>(kx_submit.pWaitSemaphores[j]);
            if (sem && sem->is_timeline()) { use_submit_v2 = true; break; }
        }
        for (uint32_t j = 0; j < kx_submit.signalSemaphoreCount; ++j) {
            auto* sem = reinterpret_cast<VkSemaphoreImpl*>(kx_submit.pSignalSemaphores[j]);
            if (sem && sem->is_timeline()) { use_submit_v2 = true; break; }
        }
    }

    if (use_submit_v2 && functions.vkQueueSubmit2KHR) {
        return submit_v2(functions, submit_count, submits, fence);
    }

    // Fallback to legacy submit (binary semaphores only)
    return submit_legacy(functions, submit_count, submits, fence);
}

VkResult VkQueueImpl::submit_legacy(VkFunctionTable& functions,
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
                wait_semaphores.push_back(reinterpret_cast<VkSemaphore>(kx_submit.pWaitSemaphores[j]));
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
                signal_semaphores.push_back(reinterpret_cast<VkSemaphore>(kx_submit.pSignalSemaphores[j]));
            }
            vk_submit.signalSemaphoreCount = kx_submit.signalSemaphoreCount;
            vk_submit.pSignalSemaphores = &signal_semaphores[signal_semaphores.size() - kx_submit.signalSemaphoreCount];
        }

        vk_submits.push_back(vk_submit);
    }

    VkFence vk_fence = fence ? reinterpret_cast<VkFence>(fence) : VK_NULL_HANDLE;
    return functions.vkQueueSubmit(queue_, static_cast<uint32_t>(vk_submits.size()), vk_submits.data(), vk_fence);
}

VkResult VkQueueImpl::submit_v2(VkFunctionTable& functions,
                                uint32_t submit_count,
                                const KxSubmitInfo* submits,
                                VkFence fence) {
    std::vector<VkSubmitInfo2KHR> vk_submits;
    vk_submits.reserve(submit_count);

    // Storage for semaphore submit infos
    std::vector<VkSemaphoreSubmitInfoKHR> wait_semaphore_infos;
    std::vector<VkSemaphoreSubmitInfoKHR> signal_semaphore_infos;
    std::vector<VkCommandBufferSubmitInfoKHR> command_buffer_infos;

    for (uint32_t i = 0; i < submit_count; ++i) {
        const KxSubmitInfo& kx_submit = submits[i];
        VkSubmitInfo2KHR vk_submit{};
        vk_submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR;

        // Wait semaphores
        if (kx_submit.waitSemaphoreCount > 0) {
            wait_semaphore_infos.reserve(wait_semaphore_infos.size() + kx_submit.waitSemaphoreCount);
            for (uint32_t j = 0; j < kx_submit.waitSemaphoreCount; ++j) {
                auto* sem = reinterpret_cast<VkSemaphoreImpl*>(kx_submit.pWaitSemaphores[j]);
                VkSemaphoreSubmitInfoKHR wait_info{};
                wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
                wait_info.semaphore = reinterpret_cast<VkSemaphore>(kx_submit.pWaitSemaphores[j]);
                wait_info.value = sem && sem->is_timeline() ? sem->current_value() : 0;
                wait_info.stageMask = static_cast<VkPipelineStageFlags2KHR>(kx_submit.pWaitDstStageMask[j]);
                wait_info.deviceIndex = 0;
                wait_semaphore_infos.push_back(wait_info);
            }
            vk_submit.waitSemaphoreInfoCount = kx_submit.waitSemaphoreCount;
            vk_submit.pWaitSemaphoreInfos = &wait_semaphore_infos[wait_semaphore_infos.size() - kx_submit.waitSemaphoreCount];
        }

        // Command buffers
        if (kx_submit.commandBufferCount > 0) {
            command_buffer_infos.reserve(command_buffer_infos.size() + kx_submit.commandBufferCount);
            for (uint32_t j = 0; j < kx_submit.commandBufferCount; ++j) {
                auto* cmd_impl = reinterpret_cast<VkCommandBufferImpl*>(kx_submit.pCommandBuffers[j]);
                if (cmd_impl) {
                    VkCommandBufferSubmitInfoKHR cmd_info{};
                    cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR;
                    cmd_info.commandBuffer = cmd_impl->handle();
                    cmd_info.deviceMask = 0;
                    command_buffer_infos.push_back(cmd_info);
                }
            }
            vk_submit.commandBufferInfoCount = kx_submit.commandBufferCount;
            vk_submit.pCommandBufferInfos = &command_buffer_infos[command_buffer_infos.size() - kx_submit.commandBufferCount];
        }

        // Signal semaphores
        if (kx_submit.signalSemaphoreCount > 0) {
            signal_semaphore_infos.reserve(signal_semaphore_infos.size() + kx_submit.signalSemaphoreCount);
            for (uint32_t j = 0; j < kx_submit.signalSemaphoreCount; ++j) {
                auto* sem = reinterpret_cast<VkSemaphoreImpl*>(kx_submit.pSignalSemaphores[j]);
                VkSemaphoreSubmitInfoKHR signal_info{};
                signal_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR;
                signal_info.semaphore = reinterpret_cast<VkSemaphore>(kx_submit.pSignalSemaphores[j]);
                signal_info.value = sem && sem->is_timeline() ? sem->current_value() : 0;
                signal_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
                signal_info.deviceIndex = 0;
                signal_semaphore_infos.push_back(signal_info);
            }
            vk_submit.signalSemaphoreInfoCount = kx_submit.signalSemaphoreCount;
            vk_submit.pSignalSemaphoreInfos = &signal_semaphore_infos[signal_semaphore_infos.size() - kx_submit.signalSemaphoreCount];
        }

        vk_submits.push_back(vk_submit);
    }

    VkFence vk_fence = fence ? reinterpret_cast<VkFence>(fence) : VK_NULL_HANDLE;
    return functions.vkQueueSubmit2KHR(queue_, static_cast<uint32_t>(vk_submits.size()), vk_submits.data(), vk_fence);
}

VkResult VkQueueImpl::present(VkFunctionTable& functions,
                              const KxPresentInfoKHR* present_info) {
    VkPresentInfoKHR vk_present{};
    vk_present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Wait semaphores
    if (present_info->waitSemaphoreCount > 0) {
        std::vector<VkSemaphore> wait_semaphores(present_info->waitSemaphoreCount);
        for (uint32_t i = 0; i < present_info->waitSemaphoreCount; ++i) {
            wait_semaphores[i] = reinterpret_cast<VkSemaphore>(present_info->pWaitSemaphores[i]);
        }
        vk_present.waitSemaphoreCount = present_info->waitSemaphoreCount;
        vk_present.pWaitSemaphores = wait_semaphores.data();
    }

    // Swapchains
    std::vector<VkSwapchainKHR> swapchains(present_info->swapchainCount);
    for (uint32_t i = 0; i < present_info->swapchainCount; ++i) {
        swapchains[i] = reinterpret_cast<VkSwapchainKHR>(present_info->pSwapchains[i]);
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