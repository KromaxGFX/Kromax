#ifndef KROMAX_SRC_VK_BACKEND_VKLOADER_H
#define KROMAX_SRC_VK_BACKEND_VKLOADER_H

/*
 * VkLoader.h
 *
 * Vulkan dynamic library loader and function pointer management.
 * Handles platform-specific library loading (vulkan-1.dll / libvulkan.so)
 * and retrieval of Vulkan API function pointers.
 */

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstddef>

namespace kx {

// Platform-specific dynamic library handle type (forward declaration)
#if defined(_WIN32)
using KX_DL_HANDLE = void*; // HMODULE
#else
using KX_DL_HANDLE = void*;
#endif

// ============================================================================
// Vulkan Function Pointer Table
// ============================================================================

struct VkFunctionTable {
    // Global functions (loaded from library directly)
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
    PFN_vkCreateInstance vkCreateInstance = nullptr;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;

    // Surface functions
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    // Surface creation (platform-specific)
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = nullptr;
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR = nullptr;
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR = nullptr;
#endif
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;

    // Instance functions (loaded via vkGetInstanceProcAddr)
    PFN_vkDestroyInstance vkDestroyInstance = nullptr;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkCreateDevice vkCreateDevice = nullptr;
    PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
    PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
    PFN_vkCreatePipelineCache vkCreatePipelineCache = nullptr;
    PFN_vkDestroyPipelineCache vkDestroyPipelineCache = nullptr;
    PFN_vkGetPipelineCacheData vkGetPipelineCacheData = nullptr;
    PFN_vkMergePipelineCaches vkMergePipelineCaches = nullptr;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
    PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
    PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
    PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
    // Timeline semaphores (VK_KHR_timeline_semaphore)
    PFN_vkGetSemaphoreCounterValueKHR vkGetSemaphoreCounterValueKHR = nullptr;
    PFN_vkWaitSemaphoresKHR vkWaitSemaphoresKHR = nullptr;
    PFN_vkSignalSemaphoreKHR vkSignalSemaphoreKHR = nullptr;
    PFN_vkCreateFence vkCreateFence = nullptr;
    PFN_vkDestroyFence vkDestroyFence = nullptr;
    PFN_vkWaitForFences vkWaitForFences = nullptr;
    PFN_vkResetFences vkResetFences = nullptr;
    PFN_vkGetFenceStatus vkGetFenceStatus = nullptr;
    PFN_vkQueueSubmit vkQueueSubmit = nullptr;
    PFN_vkQueueSubmit2KHR vkQueueSubmit2KHR = nullptr;
    PFN_vkQueueWaitIdle vkQueueWaitIdle = nullptr;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
    PFN_vkAllocateMemory vkAllocateMemory = nullptr;
    PFN_vkFreeMemory vkFreeMemory = nullptr;
    PFN_vkMapMemory vkMapMemory = nullptr;
    PFN_vkUnmapMemory vkUnmapMemory = nullptr;
    PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
    PFN_vkBindImageMemory vkBindImageMemory = nullptr;
    PFN_vkCreateBuffer vkCreateBuffer = nullptr;
    PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
    PFN_vkCreateImage vkCreateImage = nullptr;
    PFN_vkDestroyImage vkDestroyImage = nullptr;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
    PFN_vkCreateImageView vkCreateImageView = nullptr;
    PFN_vkDestroyImageView vkDestroyImageView = nullptr;
    PFN_vkCreateRenderPass vkCreateRenderPass = nullptr;
    PFN_vkDestroyRenderPass vkDestroyRenderPass = nullptr;
    PFN_vkCreateFramebuffer vkCreateFramebuffer = nullptr;
    PFN_vkDestroyFramebuffer vkDestroyFramebuffer = nullptr;
    PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
    PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
    PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
    PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;
    PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = nullptr;
    PFN_vkCmdPushConstants vkCmdPushConstants = nullptr;
    PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = nullptr;
    PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = nullptr;
    PFN_vkCmdDraw vkCmdDraw = nullptr;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
    PFN_vkCmdDrawIndirect vkCmdDrawIndirect = nullptr;
    PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect = nullptr;
    PFN_vkCmdDispatch vkCmdDispatch = nullptr;
    PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect = nullptr;
    PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = nullptr;
    PFN_vkCmdBeginRenderPass2KHR vkCmdBeginRenderPass2KHR = nullptr;
    PFN_vkCmdNextSubpass vkCmdNextSubpass = nullptr;
    PFN_vkCmdNextSubpass2KHR vkCmdNextSubpass2KHR = nullptr;
    PFN_vkCmdEndRenderPass vkCmdEndRenderPass = nullptr;
    PFN_vkCmdEndRenderPass2KHR vkCmdEndRenderPass2KHR = nullptr;
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = nullptr;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
    PFN_vkCmdCopyImage vkCmdCopyImage = nullptr;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = nullptr;
    PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer = nullptr;
    PFN_vkCmdBlitImage vkCmdBlitImage = nullptr;
    PFN_vkCmdResolveImage vkCmdResolveImage = nullptr;
    PFN_vkCmdClearColorImage vkCmdClearColorImage = nullptr;
    PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
    PFN_vkCmdFillBuffer vkCmdFillBuffer = nullptr;
    PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer = nullptr;
    PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
    PFN_vkCmdSetScissor vkCmdSetScissor = nullptr;
    PFN_vkCmdSetLineWidth vkCmdSetLineWidth = nullptr;
    PFN_vkCmdSetDepthBias vkCmdSetDepthBias = nullptr;
    PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants = nullptr;
    PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds = nullptr;
    PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask = nullptr;
    PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask = nullptr;
    PFN_vkCmdSetStencilReference vkCmdSetStencilReference = nullptr;
    // Extended dynamic state (VK_EXT_extended_dynamic_state)
    PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT = nullptr;
    PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT = nullptr;
    PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT = nullptr;
    PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT = nullptr;
    PFN_vkCmdSetScissorWithCountEXT vkCmdSetScissorWithCountEXT = nullptr;
    PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT = nullptr;
    PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT = nullptr;
    PFN_vkCmdSetDepthCompareOpEXT vkCmdSetDepthCompareOpEXT = nullptr;
    PFN_vkCmdSetDepthBoundsTestEnableEXT vkCmdSetDepthBoundsTestEnableEXT = nullptr;
    PFN_vkCmdSetStencilTestEnableEXT vkCmdSetStencilTestEnableEXT = nullptr;
    PFN_vkCmdSetStencilOpEXT vkCmdSetStencilOpEXT = nullptr;
    PFN_vkCmdSetPrimitiveRestartEnableEXT vkCmdSetPrimitiveRestartEnableEXT = nullptr;
    PFN_vkCmdSetRasterizerDiscardEnableEXT vkCmdSetRasterizerDiscardEnableEXT = nullptr;
    PFN_vkCmdSetDepthBiasEnableEXT vkCmdSetDepthBiasEnableEXT = nullptr;
    PFN_vkCmdSetLogicOpEXT vkCmdSetLogicOpEXT = nullptr;
    // Query
    PFN_vkCmdBeginQuery vkCmdBeginQuery = nullptr;
    PFN_vkCmdEndQuery vkCmdEndQuery = nullptr;
    PFN_vkCmdResetQueryPool vkCmdResetQueryPool = nullptr;
    PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp = nullptr;
    // Conditional rendering
    PFN_vkCmdBeginConditionalRenderingEXT vkCmdBeginConditionalRenderingEXT = nullptr;
    PFN_vkCmdEndConditionalRenderingEXT vkCmdEndConditionalRenderingEXT = nullptr;
    // Execute commands
    PFN_vkCmdExecuteCommands vkCmdExecuteCommands = nullptr;
    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR = nullptr;
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR = nullptr;
    // Synchronization 2 (VK_KHR_synchronization2)
    PFN_vkCmdPipelineBarrier2KHR vkCmdPipelineBarrier2KHR = nullptr;
    PFN_vkCmdWriteTimestamp2KHR vkCmdWriteTimestamp2KHR = nullptr;
    PFN_vkCmdSetEvent2KHR vkCmdSetEvent2KHR = nullptr;
    PFN_vkCmdResetEvent2KHR vkCmdResetEvent2KHR = nullptr;
    PFN_vkCmdWaitEvents2KHR vkCmdWaitEvents2KHR = nullptr;
    // Copy Commands v2 (VK_KHR_copy_commands2)
    PFN_vkCmdCopyBuffer2KHR vkCmdCopyBuffer2KHR = nullptr;
    PFN_vkCmdCopyImage2KHR vkCmdCopyImage2KHR = nullptr;
    PFN_vkCmdBlitImage2KHR vkCmdBlitImage2KHR = nullptr;
    PFN_vkCmdCopyBufferToImage2KHR vkCmdCopyBufferToImage2KHR = nullptr;
    PFN_vkCmdCopyImageToBuffer2KHR vkCmdCopyImageToBuffer2KHR = nullptr;
    PFN_vkCmdResolveImage2KHR vkCmdResolveImage2KHR = nullptr;
    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    PFN_vkCmdSetDepthClampEnableEXT vkCmdSetDepthClampEnableEXT = nullptr;
    PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT = nullptr;
    PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT = nullptr;
    PFN_vkCmdSetSampleMaskEXT vkCmdSetSampleMaskEXT = nullptr;
    PFN_vkCmdSetAlphaToCoverageEnableEXT vkCmdSetAlphaToCoverageEnableEXT = nullptr;
    PFN_vkCmdSetAlphaToOneEnableEXT vkCmdSetAlphaToOneEnableEXT = nullptr;
    PFN_vkCmdSetLogicOpEnableEXT vkCmdSetLogicOpEnableEXT = nullptr;
    PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT = nullptr;
    PFN_vkCmdSetColorBlendEquationEXT vkCmdSetColorBlendEquationEXT = nullptr;
    PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT = nullptr;
    PFN_vkCmdSetTessellationDomainOriginEXT vkCmdSetTessellationDomainOriginEXT = nullptr;
    PFN_vkCmdSetRasterizationStreamEXT vkCmdSetRasterizationStreamEXT = nullptr;
    PFN_vkCmdSetConservativeRasterizationModeEXT vkCmdSetConservativeRasterizationModeEXT = nullptr;
    PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT vkCmdSetExtraPrimitiveOverestimationSizeEXT = nullptr;
    PFN_vkCmdSetDepthClipEnableEXT vkCmdSetDepthClipEnableEXT = nullptr;
    PFN_vkCmdSetDepthClipNegativeOneToOneEXT vkCmdSetDepthClipNegativeOneToOneEXT = nullptr;
    // Viewport swizzle (VK_NV_viewport_swizzle)
    PFN_vkCmdSetViewportWScalingEnableNV vkCmdSetViewportWScalingEnableNV = nullptr;
    PFN_vkCmdSetViewportSwizzleNV vkCmdSetViewportSwizzleNV = nullptr;

    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool = nullptr;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = nullptr;
    PFN_vkResetDescriptorPool vkResetDescriptorPool = nullptr;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = nullptr;
    PFN_vkFreeDescriptorSets vkFreeDescriptorSets = nullptr;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = nullptr;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
    PFN_vkCreateComputePipelines vkCreateComputePipelines = nullptr;
    PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
    PFN_vkCreateSampler vkCreateSampler = nullptr;
    PFN_vkDestroySampler vkDestroySampler = nullptr;
    PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
    PFN_vkDestroyDevice vkDestroyDevice = nullptr;
};

// ============================================================================
// VkLoader class
// ============================================================================

class VkLoader {
public:
    VkLoader() = default;
    ~VkLoader();

    VkLoader(const VkLoader&) = delete;
    VkLoader& operator=(const VkLoader&) = delete;

    VkLoader(VkLoader&& other) noexcept;
    VkLoader& operator=(VkLoader&& other) noexcept;

    // Load Vulkan library and global function pointers
    bool load();

    // Load instance-level function pointers (requires valid VkInstance)
    bool loadInstanceFunctions(VkInstance instance);

    // Unload Vulkan library
    void unload();

    // Check if loader is initialized
    bool isLoaded() const noexcept { return libraryHandle_ != nullptr; }

    // Access function table
    const VkFunctionTable& getFunctionTable() const noexcept { return functions_; }
    VkFunctionTable& getFunctionTable() noexcept { return functions_; }

private:
    KX_DL_HANDLE libraryHandle_ = nullptr;
    VkFunctionTable functions_;
};

// ============================================================================
// Global loader instance accessor
// ============================================================================

VkLoader& getVkLoader();

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKLOADER_H