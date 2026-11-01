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
    functions_.vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(loadProc("vkCreateFence"));
    functions_.vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(loadProc("vkDestroyFence"));
    functions_.vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(loadProc("vkWaitForFences"));
    functions_.vkResetFences = reinterpret_cast<PFN_vkResetFences>(loadProc("vkResetFences"));
    functions_.vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(loadProc("vkGetFenceStatus"));
    functions_.vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(loadProc("vkQueueSubmit"));
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
    functions_.vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(loadProc("vkCmdDispatch"));
    functions_.vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(loadProc("vkCmdBeginRenderPass"));
    functions_.vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(loadProc("vkCmdNextSubpass"));
    functions_.vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(loadProc("vkCmdEndRenderPass"));
    functions_.vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(loadProc("vkCmdPipelineBarrier"));
    functions_.vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(loadProc("vkCmdCopyBuffer"));
    functions_.vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(loadProc("vkCmdCopyImage"));
    functions_.vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(loadProc("vkCmdCopyBufferToImage"));
    functions_.vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(loadProc("vkCmdCopyImageToBuffer"));
    functions_.vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(loadProc("vkCmdBlitImage"));
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