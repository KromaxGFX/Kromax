/*
 * VkLoader.cpp
 *
 * Vulkan dynamic library loader and function pointer management implementation.
 */

#include "VkLoader.h"

#include <cstring>
#include <iostream>

// Platform-specific dynamic library handling
#if defined(_WIN32)
    #include <windows.h>
    #undef KX_DL_HANDLE
    #define KX_DL_HANDLE HMODULE
    #define KX_DL_LOAD(name) LoadLibraryA(name)
    #define KX_DL_SYM(handle, name) GetProcAddress(static_cast<HMODULE>(handle), name)
    #define KX_DL_CLOSE(handle) FreeLibrary(static_cast<HMODULE>(handle))
#else
    #include <dlfcn.h>
    #undef KX_DL_HANDLE
    #define KX_DL_HANDLE void*
    #define KX_DL_LOAD(name) dlopen(name, RTLD_NOW | RTLD_LOCAL)
    #define KX_DL_SYM(handle, name) dlsym(handle, name)
    #define KX_DL_CLOSE(handle) dlclose(handle)
#endif

namespace kx {

// ============================================================================
// VkLoader implementation
// ============================================================================

VkLoader::~VkLoader() {
    unload();
}

VkLoader::VkLoader(VkLoader&& other) noexcept
    : libraryHandle_(other.libraryHandle_)
    , functions_(std::move(other.functions_)) {
    other.libraryHandle_ = nullptr;
}

VkLoader& VkLoader::operator=(VkLoader&& other) noexcept {
    if (this != &other) {
        unload();
        libraryHandle_ = other.libraryHandle_;
        functions_ = std::move(other.functions_);
        other.libraryHandle_ = nullptr;
    }
    return *this;
}

bool VkLoader::load() {
    if (libraryHandle_) {
        return true; // Already loaded
    }

#if defined(_WIN32)
    libraryHandle_ = KX_DL_LOAD("vulkan-1.dll");
#else
    libraryHandle_ = KX_DL_LOAD("libvulkan.so.1");
    if (!libraryHandle_) {
        libraryHandle_ = KX_DL_LOAD("libvulkan.so");
    }
#endif

    if (!libraryHandle_) {
        std::cerr << "Failed to load Vulkan library" << std::endl;
        return false;
    }

    // Load global functions
    functions_.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        KX_DL_SYM(libraryHandle_, "vkGetInstanceProcAddr"));
    if (!functions_.vkGetInstanceProcAddr) {
        std::cerr << "Failed to load vkGetInstanceProcAddr" << std::endl;
        unload();
        return false;
    }

    functions_.vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        KX_DL_SYM(libraryHandle_, "vkGetDeviceProcAddr"));
    if (!functions_.vkGetDeviceProcAddr) {
        std::cerr << "Failed to load vkGetDeviceProcAddr" << std::endl;
        unload();
        return false;
    }

    functions_.vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
        KX_DL_SYM(libraryHandle_, "vkCreateInstance"));
    if (!functions_.vkCreateInstance) {
        std::cerr << "Failed to load vkCreateInstance" << std::endl;
        unload();
        return false;
    }

    functions_.vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
        KX_DL_SYM(libraryHandle_, "vkEnumerateInstanceExtensionProperties"));

    functions_.vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
        KX_DL_SYM(libraryHandle_, "vkEnumerateInstanceLayerProperties"));

    return true;
}

bool VkLoader::loadInstanceFunctions(VkInstance instance) {
    if (!instance || !functions_.vkGetInstanceProcAddr) {
        return false;
    }

    auto loadProc = [instance, this](const char* name) -> void* {
        return reinterpret_cast<void*>(functions_.vkGetInstanceProcAddr(instance, name));
    };

    functions_.vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(loadProc("vkDestroyInstance"));
    functions_.vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(loadProc("vkEnumeratePhysicalDevices"));
    functions_.vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(loadProc("vkGetPhysicalDeviceProperties"));
    functions_.vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(loadProc("vkGetPhysicalDeviceFeatures"));
    functions_.vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(loadProc("vkGetPhysicalDeviceQueueFamilyProperties"));
    functions_.vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(loadProc("vkGetPhysicalDeviceMemoryProperties"));
    functions_.vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(loadProc("vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    functions_.vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(loadProc("vkGetPhysicalDeviceSurfaceFormatsKHR"));
    functions_.vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(loadProc("vkGetPhysicalDeviceSurfacePresentModesKHR"));
    functions_.vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(loadProc("vkGetPhysicalDeviceSurfaceSupportKHR"));
    // Surface creation (platform-specific)
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    functions_.vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(loadProc("vkCreateWin32SurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    functions_.vkCreateXcbSurfaceKHR = reinterpret_cast<PFN_vkCreateXcbSurfaceKHR>(loadProc("vkCreateXcbSurfaceKHR"));
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    functions_.vkCreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(loadProc("vkCreateWaylandSurfaceKHR"));
#endif
    functions_.vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(loadProc("vkDestroySurfaceKHR"));
    functions_.vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(loadProc("vkCreateDevice"));
    functions_.vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(loadProc("vkCreateShaderModule"));
    functions_.vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(loadProc("vkDestroyShaderModule"));
    functions_.vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(loadProc("vkCreatePipelineCache"));
    functions_.vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(loadProc("vkDestroyPipelineCache"));
    functions_.vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(loadProc("vkGetPipelineCacheData"));
    functions_.vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(loadProc("vkMergePipelineCaches"));
    functions_.vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(loadProc("vkCreateSwapchainKHR"));
    functions_.vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(loadProc("vkDestroySwapchainKHR"));
    functions_.vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(loadProc("vkGetSwapchainImagesKHR"));
    functions_.vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(loadProc("vkAcquireNextImageKHR"));
    functions_.vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(loadProc("vkQueuePresentKHR"));
    functions_.vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(loadProc("vkCreateSemaphore"));
    functions_.vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(loadProc("vkDestroySemaphore"));
    // Timeline semaphores (VK_KHR_timeline_semaphore)
    functions_.vkGetSemaphoreCounterValueKHR = reinterpret_cast<PFN_vkGetSemaphoreCounterValueKHR>(loadProc("vkGetSemaphoreCounterValueKHR"));
    functions_.vkWaitSemaphoresKHR = reinterpret_cast<PFN_vkWaitSemaphoresKHR>(loadProc("vkWaitSemaphoresKHR"));
    functions_.vkSignalSemaphoreKHR = reinterpret_cast<PFN_vkSignalSemaphoreKHR>(loadProc("vkSignalSemaphoreKHR"));
    functions_.vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(loadProc("vkCreateFence"));
    functions_.vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(loadProc("vkDestroyFence"));
    functions_.vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(loadProc("vkWaitForFences"));
    functions_.vkResetFences = reinterpret_cast<PFN_vkResetFences>(loadProc("vkResetFences"));
    functions_.vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(loadProc("vkGetFenceStatus"));
    functions_.vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(loadProc("vkQueueSubmit"));
    functions_.vkQueueSubmit2KHR = reinterpret_cast<PFN_vkQueueSubmit2KHR>(loadProc("vkQueueSubmit2KHR"));
    functions_.vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(loadProc("vkQueueWaitIdle"));
    functions_.vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(loadProc("vkDeviceWaitIdle"));
    functions_.vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(loadProc("vkAllocateMemory"));
    functions_.vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(loadProc("vkFreeMemory"));
    functions_.vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(loadProc("vkMapMemory"));
    functions_.vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(loadProc("vkUnmapMemory"));
    functions_.vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(loadProc("vkBindBufferMemory"));
    functions_.vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(loadProc("vkBindImageMemory"));
    functions_.vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(loadProc("vkCreateBuffer"));
    functions_.vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(loadProc("vkDestroyBuffer"));
    functions_.vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(loadProc("vkCreateImage"));
    functions_.vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(loadProc("vkDestroyImage"));
    functions_.vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(loadProc("vkGetBufferMemoryRequirements"));
    functions_.vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(loadProc("vkGetImageMemoryRequirements"));
    functions_.vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(loadProc("vkCreateImageView"));
    functions_.vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(loadProc("vkDestroyImageView"));
    functions_.vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(loadProc("vkCreateRenderPass"));
    functions_.vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(loadProc("vkDestroyRenderPass"));
    functions_.vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(loadProc("vkCreateFramebuffer"));
    functions_.vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(loadProc("vkDestroyFramebuffer"));
    functions_.vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(loadProc("vkCreateCommandPool"));
    functions_.vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(loadProc("vkDestroyCommandPool"));
    functions_.vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(loadProc("vkAllocateCommandBuffers"));
    functions_.vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(loadProc("vkFreeCommandBuffers"));
    functions_.vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(loadProc("vkBeginCommandBuffer"));
    functions_.vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(loadProc("vkEndCommandBuffer"));
    functions_.vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(loadProc("vkResetCommandBuffer"));
    functions_.vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(loadProc("vkCmdBindPipeline"));
    functions_.vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(loadProc("vkCmdBindDescriptorSets"));
    functions_.vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(loadProc("vkCmdPushConstants"));
    functions_.vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(loadProc("vkCmdBindVertexBuffers"));
    functions_.vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(loadProc("vkCmdBindIndexBuffer"));
    functions_.vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(loadProc("vkCmdDraw"));
    functions_.vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(loadProc("vkCmdDrawIndexed"));
    functions_.vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(loadProc("vkCmdDrawIndirect"));
    functions_.vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(loadProc("vkCmdDrawIndexedIndirect"));
    functions_.vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(loadProc("vkCmdDispatch"));
    functions_.vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(loadProc("vkCmdDispatchIndirect"));
    functions_.vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(loadProc("vkCmdBeginRenderPass"));
    functions_.vkCmdBeginRenderPass2KHR = reinterpret_cast<PFN_vkCmdBeginRenderPass2KHR>(loadProc("vkCmdBeginRenderPass2KHR"));
    functions_.vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(loadProc("vkCmdNextSubpass"));
    functions_.vkCmdNextSubpass2KHR = reinterpret_cast<PFN_vkCmdNextSubpass2KHR>(loadProc("vkCmdNextSubpass2KHR"));
    functions_.vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(loadProc("vkCmdEndRenderPass"));
    functions_.vkCmdEndRenderPass2KHR = reinterpret_cast<PFN_vkCmdEndRenderPass2KHR>(loadProc("vkCmdEndRenderPass2KHR"));
    functions_.vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(loadProc("vkCmdPipelineBarrier"));
    functions_.vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(loadProc("vkCmdCopyBuffer"));
    functions_.vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(loadProc("vkCmdCopyImage"));
    functions_.vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(loadProc("vkCmdCopyBufferToImage"));
    functions_.vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(loadProc("vkCmdCopyImageToBuffer"));
    functions_.vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(loadProc("vkCmdBlitImage"));
    functions_.vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(loadProc("vkCmdResolveImage"));
    functions_.vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(loadProc("vkCmdClearColorImage"));
    functions_.vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(loadProc("vkCmdClearDepthStencilImage"));
    functions_.vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(loadProc("vkCmdFillBuffer"));
    functions_.vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(loadProc("vkCmdUpdateBuffer"));
    functions_.vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(loadProc("vkCmdSetViewport"));
    functions_.vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(loadProc("vkCmdSetScissor"));
    functions_.vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(loadProc("vkCmdSetLineWidth"));
    functions_.vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(loadProc("vkCmdSetDepthBias"));
    functions_.vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(loadProc("vkCmdSetBlendConstants"));
    functions_.vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(loadProc("vkCmdSetDepthBounds"));
    functions_.vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(loadProc("vkCmdSetStencilCompareMask"));
    functions_.vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(loadProc("vkCmdSetStencilWriteMask"));
    functions_.vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(loadProc("vkCmdSetStencilReference"));
    // Extended dynamic state (VK_EXT_extended_dynamic_state)
    functions_.vkCmdSetCullModeEXT = reinterpret_cast<PFN_vkCmdSetCullModeEXT>(loadProc("vkCmdSetCullModeEXT"));
    functions_.vkCmdSetFrontFaceEXT = reinterpret_cast<PFN_vkCmdSetFrontFaceEXT>(loadProc("vkCmdSetFrontFaceEXT"));
    functions_.vkCmdSetPrimitiveTopologyEXT = reinterpret_cast<PFN_vkCmdSetPrimitiveTopologyEXT>(loadProc("vkCmdSetPrimitiveTopologyEXT"));
    functions_.vkCmdSetViewportWithCountEXT = reinterpret_cast<PFN_vkCmdSetViewportWithCountEXT>(loadProc("vkCmdSetViewportWithCountEXT"));
    functions_.vkCmdSetScissorWithCountEXT = reinterpret_cast<PFN_vkCmdSetScissorWithCountEXT>(loadProc("vkCmdSetScissorWithCountEXT"));
    functions_.vkCmdSetDepthTestEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthTestEnableEXT>(loadProc("vkCmdSetDepthTestEnableEXT"));
    functions_.vkCmdSetDepthWriteEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthWriteEnableEXT>(loadProc("vkCmdSetDepthWriteEnableEXT"));
    functions_.vkCmdSetDepthCompareOpEXT = reinterpret_cast<PFN_vkCmdSetDepthCompareOpEXT>(loadProc("vkCmdSetDepthCompareOpEXT"));
    functions_.vkCmdSetDepthBoundsTestEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthBoundsTestEnableEXT>(loadProc("vkCmdSetDepthBoundsTestEnableEXT"));
    functions_.vkCmdSetStencilTestEnableEXT = reinterpret_cast<PFN_vkCmdSetStencilTestEnableEXT>(loadProc("vkCmdSetStencilTestEnableEXT"));
    functions_.vkCmdSetStencilOpEXT = reinterpret_cast<PFN_vkCmdSetStencilOpEXT>(loadProc("vkCmdSetStencilOpEXT"));
    functions_.vkCmdSetPrimitiveRestartEnableEXT = reinterpret_cast<PFN_vkCmdSetPrimitiveRestartEnableEXT>(loadProc("vkCmdSetPrimitiveRestartEnableEXT"));
    functions_.vkCmdSetRasterizerDiscardEnableEXT = reinterpret_cast<PFN_vkCmdSetRasterizerDiscardEnableEXT>(loadProc("vkCmdSetRasterizerDiscardEnableEXT"));
    functions_.vkCmdSetDepthBiasEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthBiasEnableEXT>(loadProc("vkCmdSetDepthBiasEnableEXT"));
    functions_.vkCmdSetLogicOpEXT = reinterpret_cast<PFN_vkCmdSetLogicOpEXT>(loadProc("vkCmdSetLogicOpEXT"));
    // Query
    functions_.vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(loadProc("vkCmdBeginQuery"));
    functions_.vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(loadProc("vkCmdEndQuery"));
    functions_.vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(loadProc("vkCmdResetQueryPool"));
    functions_.vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(loadProc("vkCmdWriteTimestamp"));
    // Conditional rendering
    functions_.vkCmdBeginConditionalRenderingEXT = reinterpret_cast<PFN_vkCmdBeginConditionalRenderingEXT>(loadProc("vkCmdBeginConditionalRenderingEXT"));
    functions_.vkCmdEndConditionalRenderingEXT = reinterpret_cast<PFN_vkCmdEndConditionalRenderingEXT>(loadProc("vkCmdEndConditionalRenderingEXT"));
    // Execute commands
    functions_.vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(loadProc("vkCmdExecuteCommands"));
    // Dynamic Rendering (VK_KHR_dynamic_rendering)
    functions_.vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(loadProc("vkCmdBeginRenderingKHR"));
    functions_.vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(loadProc("vkCmdEndRenderingKHR"));
    // Synchronization 2 (VK_KHR_synchronization2)
    functions_.vkCmdPipelineBarrier2KHR = reinterpret_cast<PFN_vkCmdPipelineBarrier2KHR>(loadProc("vkCmdPipelineBarrier2KHR"));
    functions_.vkCmdWriteTimestamp2KHR = reinterpret_cast<PFN_vkCmdWriteTimestamp2KHR>(loadProc("vkCmdWriteTimestamp2KHR"));
    functions_.vkCmdSetEvent2KHR = reinterpret_cast<PFN_vkCmdSetEvent2KHR>(loadProc("vkCmdSetEvent2KHR"));
    functions_.vkCmdResetEvent2KHR = reinterpret_cast<PFN_vkCmdResetEvent2KHR>(loadProc("vkCmdResetEvent2KHR"));
    functions_.vkCmdWaitEvents2KHR = reinterpret_cast<PFN_vkCmdWaitEvents2KHR>(loadProc("vkCmdWaitEvents2KHR"));
    // Copy Commands v2 (VK_KHR_copy_commands2)
    functions_.vkCmdCopyBuffer2KHR = reinterpret_cast<PFN_vkCmdCopyBuffer2KHR>(loadProc("vkCmdCopyBuffer2KHR"));
    functions_.vkCmdCopyImage2KHR = reinterpret_cast<PFN_vkCmdCopyImage2KHR>(loadProc("vkCmdCopyImage2KHR"));
    functions_.vkCmdBlitImage2KHR = reinterpret_cast<PFN_vkCmdBlitImage2KHR>(loadProc("vkCmdBlitImage2KHR"));
    functions_.vkCmdCopyBufferToImage2KHR = reinterpret_cast<PFN_vkCmdCopyBufferToImage2KHR>(loadProc("vkCmdCopyBufferToImage2KHR"));
    functions_.vkCmdCopyImageToBuffer2KHR = reinterpret_cast<PFN_vkCmdCopyImageToBuffer2KHR>(loadProc("vkCmdCopyImageToBuffer2KHR"));
    functions_.vkCmdResolveImage2KHR = reinterpret_cast<PFN_vkCmdResolveImage2KHR>(loadProc("vkCmdResolveImage2KHR"));
    // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
    functions_.vkCmdSetDepthClampEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthClampEnableEXT>(loadProc("vkCmdSetDepthClampEnableEXT"));
    functions_.vkCmdSetPolygonModeEXT = reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(loadProc("vkCmdSetPolygonModeEXT"));
    functions_.vkCmdSetRasterizationSamplesEXT = reinterpret_cast<PFN_vkCmdSetRasterizationSamplesEXT>(loadProc("vkCmdSetRasterizationSamplesEXT"));
    functions_.vkCmdSetSampleMaskEXT = reinterpret_cast<PFN_vkCmdSetSampleMaskEXT>(loadProc("vkCmdSetSampleMaskEXT"));
    functions_.vkCmdSetAlphaToCoverageEnableEXT = reinterpret_cast<PFN_vkCmdSetAlphaToCoverageEnableEXT>(loadProc("vkCmdSetAlphaToCoverageEnableEXT"));
    functions_.vkCmdSetAlphaToOneEnableEXT = reinterpret_cast<PFN_vkCmdSetAlphaToOneEnableEXT>(loadProc("vkCmdSetAlphaToOneEnableEXT"));
    functions_.vkCmdSetLogicOpEnableEXT = reinterpret_cast<PFN_vkCmdSetLogicOpEnableEXT>(loadProc("vkCmdSetLogicOpEnableEXT"));
    functions_.vkCmdSetColorBlendEnableEXT = reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(loadProc("vkCmdSetColorBlendEnableEXT"));
    functions_.vkCmdSetColorBlendEquationEXT = reinterpret_cast<PFN_vkCmdSetColorBlendEquationEXT>(loadProc("vkCmdSetColorBlendEquationEXT"));
    functions_.vkCmdSetColorWriteMaskEXT = reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>(loadProc("vkCmdSetColorWriteMaskEXT"));
    functions_.vkCmdSetTessellationDomainOriginEXT = reinterpret_cast<PFN_vkCmdSetTessellationDomainOriginEXT>(loadProc("vkCmdSetTessellationDomainOriginEXT"));
    functions_.vkCmdSetRasterizationStreamEXT = reinterpret_cast<PFN_vkCmdSetRasterizationStreamEXT>(loadProc("vkCmdSetRasterizationStreamEXT"));
    functions_.vkCmdSetConservativeRasterizationModeEXT = reinterpret_cast<PFN_vkCmdSetConservativeRasterizationModeEXT>(loadProc("vkCmdSetConservativeRasterizationModeEXT"));
    functions_.vkCmdSetExtraPrimitiveOverestimationSizeEXT = reinterpret_cast<PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT>(loadProc("vkCmdSetExtraPrimitiveOverestimationSizeEXT"));
    functions_.vkCmdSetDepthClipEnableEXT = reinterpret_cast<PFN_vkCmdSetDepthClipEnableEXT>(loadProc("vkCmdSetDepthClipEnableEXT"));
    functions_.vkCmdSetDepthClipNegativeOneToOneEXT = reinterpret_cast<PFN_vkCmdSetDepthClipNegativeOneToOneEXT>(loadProc("vkCmdSetDepthClipNegativeOneToOneEXT"));
    // Viewport swizzle (VK_NV_viewport_swizzle)
    functions_.vkCmdSetViewportWScalingEnableNV = reinterpret_cast<PFN_vkCmdSetViewportWScalingEnableNV>(loadProc("vkCmdSetViewportWScalingEnableNV"));
    functions_.vkCmdSetViewportSwizzleNV = reinterpret_cast<PFN_vkCmdSetViewportSwizzleNV>(loadProc("vkCmdSetViewportSwizzleNV"));

    functions_.vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(loadProc("vkCreateDescriptorSetLayout"));
    functions_.vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(loadProc("vkDestroyDescriptorSetLayout"));
    functions_.vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(loadProc("vkCreateDescriptorPool"));
    functions_.vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(loadProc("vkDestroyDescriptorPool"));
    functions_.vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(loadProc("vkResetDescriptorPool"));
    functions_.vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(loadProc("vkAllocateDescriptorSets"));
    functions_.vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(loadProc("vkFreeDescriptorSets"));
    functions_.vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(loadProc("vkUpdateDescriptorSets"));
    functions_.vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(loadProc("vkCreatePipelineLayout"));
    functions_.vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(loadProc("vkDestroyPipelineLayout"));
    functions_.vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(loadProc("vkCreateGraphicsPipelines"));
    functions_.vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(loadProc("vkCreateComputePipelines"));
    functions_.vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(loadProc("vkDestroyPipeline"));
    functions_.vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(loadProc("vkCreateSampler"));
    functions_.vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(loadProc("vkDestroySampler"));
    functions_.vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(loadProc("vkGetDeviceQueue"));
    functions_.vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(loadProc("vkDestroyDevice"));

    return true;
}

void VkLoader::unload() {
    if (libraryHandle_) {
        KX_DL_CLOSE(libraryHandle_);
        libraryHandle_ = nullptr;
    }
    functions_ = VkFunctionTable{};
}

// ============================================================================
// Global loader instance
// ============================================================================

VkLoader& getVkLoader() {
    static VkLoader loader;
    return loader;
}

} // namespace kx