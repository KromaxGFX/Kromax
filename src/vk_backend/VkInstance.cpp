/*
 * VkInstance.cpp
 *
 * Vulkan instance wrapper implementation.
 */

#include "VkInstance.h"

#include <cstring>
#include <algorithm>

namespace kx {

// ============================================================================
// VkInstanceImpl implementation
// ============================================================================

VkInstanceImpl::VkInstanceImpl(const InstanceCreateInfo& info) : createInfo_(info), handle_(nullptr) {
    // Collect extensions
    for (uint32_t i = 0; i < info.enabledExtensionCount; ++i) {
        enabledExtensions_.push_back(info.ppEnabledExtensionNames[i]);
    }

    // Always add required surface extensions
    std::vector<const char*> tempExtensions;
    for (const auto& ext : enabledExtensions_) {
        tempExtensions.push_back(ext.c_str());
    }
    collectSurfaceExtensions(tempExtensions);
    for (const auto& ext : tempExtensions) {
        // Check if already present
        bool found = false;
        for (const auto& existing : enabledExtensions_) {
            if (existing == ext) { found = true; break; }
        }
        if (!found) enabledExtensions_.push_back(ext);
    }

    // Collect layers
    for (uint32_t i = 0; i < info.enabledLayerCount; ++i) {
        enabledLayers_.push_back(info.ppEnabledLayerNames[i]);
    }
}

VkInstanceImpl::~VkInstanceImpl() {
    // Instance destruction is handled by VkRuntime via VkInstanceUtil::destroyInstance
}

VkInstanceImpl::VkInstanceImpl(VkInstanceImpl&& other) noexcept
    : createInfo_(std::move(other.createInfo_))
    , handle_(other.handle_)
    , enabledExtensions_(std::move(other.enabledExtensions_))
    , enabledLayers_(std::move(other.enabledLayers_)) {
    other.handle_ = nullptr;
}

VkInstanceImpl& VkInstanceImpl::operator=(VkInstanceImpl&& other) noexcept {
    if (this != &other) {
        handle_ = other.handle_;
        createInfo_ = std::move(other.createInfo_);
        enabledExtensions_ = std::move(other.enabledExtensions_);
        enabledLayers_ = std::move(other.enabledLayers_);
        other.handle_ = nullptr;
    }
    return *this;
}

void VkInstanceImpl::collectSurfaceExtensions(std::vector<const char*>& extensions) {
    extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
}

VkInstanceCreateInfo VkInstanceImpl::buildCreateInfo(const InstanceCreateInfo& info,
                                                      const std::vector<const char*>& extensions,
                                                      const std::vector<const char*>& layers,
                                                      VkApplicationInfo* outAppInfo) {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    if (info.pApplicationInfo) {
        appInfo.pApplicationName = info.pApplicationInfo->pApplicationName;
        appInfo.applicationVersion = info.pApplicationInfo->applicationVersion;
        appInfo.pEngineName = info.pApplicationInfo->pEngineName;
        appInfo.engineVersion = info.pApplicationInfo->engineVersion;
        appInfo.apiVersion = info.pApplicationInfo->apiVersion;
    } else {
        appInfo.pApplicationName = "Kromax Application";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Kromax Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;
    }

    if (outAppInfo) {
        *outAppInfo = appInfo;
    }

    VkInstanceCreateInfo vkCreateInfo{};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInfo.flags = info.flags;
    vkCreateInfo.pApplicationInfo = &appInfo;
    vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    vkCreateInfo.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();
    vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    vkCreateInfo.ppEnabledExtensionNames = extensions.data();

    return vkCreateInfo;
}

// ============================================================================
// VkInstanceUtil implementation
// ============================================================================

namespace VkInstanceUtil {

KxResult createInstance(const InstanceCreateInfo& info,
                         VkFunctionTable& functions,
                         VkInstanceImpl** outInstance) {
    if (!outInstance) return KX_ERROR_INVALID_ARGUMENT;
    if (!functions.vkCreateInstance) return KX_ERROR_INITIALIZATION_FAILED;

    auto* instance = new VkInstanceImpl(info);

    // Build extension list
    std::vector<const char*> extensions;
    if (info.enabledExtensionCount > 0) {
        extensions.insert(extensions.end(), info.ppEnabledExtensionNames,
                         info.ppEnabledExtensionNames + info.enabledExtensionCount);
    }
    instance->collectSurfaceExtensions(extensions);

    // Build layer list
    std::vector<const char*> layers;
    if (info.enabledLayerCount > 0) {
        layers.insert(layers.end(), info.ppEnabledLayerNames,
                     info.ppEnabledLayerNames + info.enabledLayerCount);
    }

    // Build VkInstanceCreateInfo
    VkApplicationInfo appInfo;
    VkInstanceCreateInfo vkCreateInfo = VkInstanceImpl::buildCreateInfo(info, extensions, layers, &appInfo);
    vkCreateInfo.pApplicationInfo = &appInfo;

    VkInstance vkInstance = VK_NULL_HANDLE;
    VkResult result = functions.vkCreateInstance(&vkCreateInfo, nullptr, &vkInstance);

    if (result != VK_SUCCESS) {
        delete instance;
        return static_cast<KxResult>(result);
    }

    instance->setHandle(vkInstance);

    // Load instance functions
    if (!functions.vkGetInstanceProcAddr) {
        functions.vkDestroyInstance(vkInstance, nullptr);
        delete instance;
        return KX_ERROR_INITIALIZATION_FAILED;
    }

    // Load instance functions into the function table
    auto loadProc = [vkInstance, &functions](const char* name) -> void* {
        return reinterpret_cast<void*>(functions.vkGetInstanceProcAddr(vkInstance, name));
    };

    functions.vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(loadProc("vkDestroyInstance"));
    functions.vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(loadProc("vkEnumeratePhysicalDevices"));
    functions.vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(loadProc("vkGetPhysicalDeviceProperties"));
    functions.vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(loadProc("vkGetPhysicalDeviceFeatures"));
    functions.vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(loadProc("vkGetPhysicalDeviceQueueFamilyProperties"));
    functions.vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(loadProc("vkGetPhysicalDeviceMemoryProperties"));
    functions.vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(loadProc("vkCreateDevice"));
    functions.vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(loadProc("vkCreateShaderModule"));
    functions.vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(loadProc("vkDestroyShaderModule"));
    functions.vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(loadProc("vkCreatePipelineCache"));
    functions.vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(loadProc("vkDestroyPipelineCache"));
    functions.vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(loadProc("vkGetPipelineCacheData"));
    functions.vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(loadProc("vkMergePipelineCaches"));
    functions.vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(loadProc("vkCreateSwapchainKHR"));
    functions.vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(loadProc("vkDestroySwapchainKHR"));
    functions.vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(loadProc("vkGetSwapchainImagesKHR"));
    functions.vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(loadProc("vkAcquireNextImageKHR"));
    functions.vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(loadProc("vkQueuePresentKHR"));
    functions.vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(loadProc("vkCreateSemaphore"));
    functions.vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(loadProc("vkDestroySemaphore"));
    functions.vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(loadProc("vkCreateFence"));
    functions.vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(loadProc("vkDestroyFence"));
    functions.vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(loadProc("vkWaitForFences"));
    functions.vkResetFences = reinterpret_cast<PFN_vkResetFences>(loadProc("vkResetFences"));
    functions.vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(loadProc("vkQueueSubmit"));
    functions.vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(loadProc("vkQueueWaitIdle"));
    functions.vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(loadProc("vkDeviceWaitIdle"));
    functions.vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(loadProc("vkAllocateMemory"));
    functions.vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(loadProc("vkFreeMemory"));
    functions.vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(loadProc("vkMapMemory"));
    functions.vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(loadProc("vkUnmapMemory"));
    functions.vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(loadProc("vkBindBufferMemory"));
    functions.vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(loadProc("vkBindImageMemory"));
    functions.vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(loadProc("vkCreateBuffer"));
    functions.vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(loadProc("vkDestroyBuffer"));
    functions.vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(loadProc("vkCreateImage"));
    functions.vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(loadProc("vkDestroyImage"));
    functions.vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(loadProc("vkGetBufferMemoryRequirements"));
    functions.vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(loadProc("vkGetImageMemoryRequirements"));
    functions.vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(loadProc("vkCreateImageView"));
    functions.vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(loadProc("vkDestroyImageView"));
    functions.vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(loadProc("vkCreateRenderPass"));
    functions.vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(loadProc("vkDestroyRenderPass"));
    functions.vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(loadProc("vkCreateFramebuffer"));
    functions.vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(loadProc("vkDestroyFramebuffer"));
    functions.vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(loadProc("vkCreateCommandPool"));
    functions.vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(loadProc("vkDestroyCommandPool"));
    functions.vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(loadProc("vkAllocateCommandBuffers"));
    functions.vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(loadProc("vkFreeCommandBuffers"));
    functions.vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(loadProc("vkBeginCommandBuffer"));
    functions.vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(loadProc("vkEndCommandBuffer"));
    functions.vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(loadProc("vkResetCommandBuffer"));
    functions.vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(loadProc("vkCmdBindPipeline"));
    functions.vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(loadProc("vkCmdBindDescriptorSets"));
    functions.vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(loadProc("vkCmdPushConstants"));
    functions.vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(loadProc("vkCmdBindVertexBuffers"));
    functions.vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(loadProc("vkCmdBindIndexBuffer"));
    functions.vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(loadProc("vkCmdDraw"));
    functions.vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(loadProc("vkCmdDrawIndexed"));
    functions.vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(loadProc("vkCmdDispatch"));
    functions.vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(loadProc("vkCmdBeginRenderPass"));
    functions.vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(loadProc("vkCmdNextSubpass"));
    functions.vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(loadProc("vkCmdEndRenderPass"));
    functions.vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(loadProc("vkCmdPipelineBarrier"));
    functions.vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(loadProc("vkCmdCopyBuffer"));
    functions.vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(loadProc("vkCmdCopyImage"));
    functions.vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(loadProc("vkCmdCopyBufferToImage"));
    functions.vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(loadProc("vkCmdCopyImageToBuffer"));
    functions.vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(loadProc("vkCmdBlitImage"));
    functions.vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(loadProc("vkCreateDescriptorSetLayout"));
    functions.vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(loadProc("vkDestroyDescriptorSetLayout"));
    functions.vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(loadProc("vkCreateDescriptorPool"));
    functions.vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(loadProc("vkDestroyDescriptorPool"));
    functions.vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(loadProc("vkAllocateDescriptorSets"));
    functions.vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(loadProc("vkFreeDescriptorSets"));
    functions.vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(loadProc("vkUpdateDescriptorSets"));
    functions.vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(loadProc("vkCreatePipelineLayout"));
    functions.vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(loadProc("vkDestroyPipelineLayout"));
    functions.vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(loadProc("vkCreateGraphicsPipelines"));
    functions.vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(loadProc("vkCreateComputePipelines"));
    functions.vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(loadProc("vkDestroyPipeline"));
    functions.vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(loadProc("vkCreateSampler"));
    functions.vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(loadProc("vkDestroySampler"));

    *outInstance = instance;
    return KX_SUCCESS;
}

void destroyInstance(VkInstanceImpl* instance, VkFunctionTable& functions) {
    if (!instance) return;

    if (instance->getHandle() && functions.vkDestroyInstance) {
        functions.vkDestroyInstance(reinterpret_cast<VkInstance>(instance->getHandle()), nullptr);
    }
    delete instance;
}

} // namespace VkInstanceUtil

} // namespace kx