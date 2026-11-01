#ifndef KROMAX_SRC_RUNTIME_IDEVICE_H
#define KROMAX_SRC_RUNTIME_IDEVICE_H

/*
 * IDevice.h
 *
 * Backend-agnostic device interface. Resource factory for buffers, images,
 * pipelines, descriptors, command buffers, swapchains, synchronization.
 *
 * SOLID: Interface Segregation - fine-grained interface for device operations.
 */

#include <cstdint>
#include <memory>
#include <vector>

#include "kx_core.h"
#include "Expected.h"

namespace kx {

class Buffer;
class Image;
class ImageView;
class ShaderModule;
class Pipeline;
class PipelineLayout;
class DescriptorSetLayout;
class DescriptorSet;
class DescriptorPool;
class CommandPool;
class CommandBuffer;
class RenderPass;
class Framebuffer;
class Swapchain;
class Semaphore;
class Fence;
class Queue;
class Sampler;
class ThreadPool;

struct BufferCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    KxBufferCreateFlags flags = 0;
    KxDeviceSize size = 0;
    KxBufferUsageFlags usage = 0;
    KxSharingMode sharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
};

struct ImageCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    const void* pNext = nullptr;
    KxImageCreateFlags flags = 0;
    KxImageType imageType = KX_IMAGE_TYPE_2D;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxExtent3D extent = {1, 1, 1};
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    KxSampleCountFlagBits samples = KX_SAMPLE_COUNT_1_BIT;
    KxImageTiling tiling = KX_IMAGE_TILING_OPTIMAL;
    KxImageUsageFlags usage = 0;
    KxSharingMode sharingMode = KX_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    KxImageLayout initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
};

struct ImageViewCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    const void* pNext = nullptr;
    KxImageViewCreateFlags flags = 0;
    Image image = nullptr;
    KxImageViewType viewType = KX_IMAGE_VIEW_TYPE_2D;
    KxFormat format = KX_FORMAT_UNDEFINED;
    KxComponentMapping components{};
    KxImageSubresourceRange subresourceRange{};
};

struct ShaderModuleCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    const void* pNext = nullptr;
    KxShaderModuleCreateFlags flags = 0;
    size_t codeSize = 0;
    const uint32_t* pCode = nullptr;
};

struct PipelineLayoutCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineLayoutCreateFlags flags = 0;
    uint32_t setLayoutCount = 0;
    const DescriptorSetLayout* pSetLayouts = nullptr;
    uint32_t pushConstantRangeCount = 0;
    const KxPushConstantRange* pPushConstantRanges = nullptr;
};

struct DescriptorSetLayoutCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    const void* pNext = nullptr;
    KxDescriptorSetLayoutCreateFlags flags = 0;
    uint32_t bindingCount = 0;
    const KxDescriptorSetLayoutBinding* pBindings = nullptr;
};

struct DescriptorPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    KxDescriptorPoolCreateFlags flags = 0;
    uint32_t maxSets = 0;
    uint32_t poolSizeCount = 0;
    const KxDescriptorPoolSize* pPoolSizes = nullptr;
};

struct DescriptorSetAllocateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    const void* pNext = nullptr;
    DescriptorPool descriptorPool = nullptr;
    uint32_t descriptorSetCount = 0;
    const DescriptorSetLayout* pSetLayouts = nullptr;
};

struct CommandPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    KxCommandPoolCreateFlags flags = 0;
    uint32_t queueFamilyIndex = 0;
};

struct CommandBufferAllocateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    const void* pNext = nullptr;
    CommandPool commandPool = nullptr;
    KxCommandBufferLevel level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    uint32_t commandBufferCount = 1;
};

struct RenderPassCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    const void* pNext = nullptr;
    KxRenderPassCreateFlags flags = 0;
    uint32_t attachmentCount = 0;
    const KxAttachmentDescription* pAttachments = nullptr;
    uint32_t subpassCount = 0;
    const KxSubpassDescription* pSubpasses = nullptr;
    uint32_t dependencyCount = 0;
    const KxSubpassDependency* pDependencies = nullptr;
};

struct FramebufferCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    KxFramebufferCreateFlags flags = 0;
    RenderPass renderPass = nullptr;
    uint32_t attachmentCount = 0;
    const ImageView* pAttachments = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
};

struct SamplerCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    const void* pNext = nullptr;
    KxSamplerCreateFlags flags = 0;
    KxFilter magFilter = KX_FILTER_NEAREST;
    KxFilter minFilter = KX_FILTER_NEAREST;
    KxSamplerMipmapMode mipmapMode = KX_SAMPLER_MIPMAP_MODE_NEAREST;
    KxSamplerAddressMode addressModeU = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    KxSamplerAddressMode addressModeV = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    KxSamplerAddressMode addressModeW = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.0f;
    KxBool32 anisotropyEnable = false;
    float maxAnisotropy = 1.0f;
    KxBool32 compareEnable = false;
    KxCompareOp compareOp = KX_COMPARE_OP_NEVER;
    float minLod = 0.0f;
    float maxLod = 0.0f;
    KxBorderColor borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    KxBool32 unnormalizedCoordinates = false;
};

struct ThreadPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_THREAD_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    KxThreadPoolCreateFlags flags = 0;
    uint32_t threadCount = 0;
};

class IDevice {
public:
    virtual ~IDevice() = default;

    // Buffer operations
    virtual Result createBuffer(const BufferCreateInfo& info, Buffer** outBuffer) = 0;
    virtual void destroyBuffer(Buffer* buffer) = 0;
    virtual Result getBufferMemoryRequirements(Buffer* buffer, KxMemoryRequirements* pRequirements) = 0;

    // Image operations
    virtual Result createImage(const ImageCreateInfo& info, Image** outImage) = 0;
    virtual void destroyImage(Image* image) = 0;
    virtual Result getImageMemoryRequirements(Image* image, KxMemoryRequirements* pRequirements) = 0;
    virtual Result createImageView(const ImageViewCreateInfo& info, ImageView** outView) = 0;
    virtual void destroyImageView(ImageView* view) = 0;

    // Memory operations
    virtual Result allocateMemory(const KxMemoryAllocateInfo& info, KxDeviceMemory* pMemory) = 0;
    virtual void freeMemory(KxDeviceMemory memory) = 0;
    virtual Result mapMemory(KxDeviceMemory memory, KxDeviceSize offset, KxDeviceSize size, KxMemoryMapFlags flags, void** ppData) = 0;
    virtual void unmapMemory(KxDeviceMemory memory) = 0;
    virtual Result bindBufferMemory(Buffer* buffer, KxDeviceMemory memory, KxDeviceSize offset) = 0;
    virtual Result bindImageMemory(Image* image, KxDeviceMemory memory, KxDeviceSize offset) = 0;

    // Shader modules
    virtual Result createShaderModule(const ShaderModuleCreateInfo& info, ShaderModule** outModule) = 0;
    virtual void destroyShaderModule(ShaderModule* module) = 0;

    // Pipeline operations
    virtual Result createPipelineLayout(const PipelineLayoutCreateInfo& info, PipelineLayout** outLayout) = 0;
    virtual void destroyPipelineLayout(PipelineLayout layout) = 0;
    virtual Result createGraphicsPipelines(uint32_t createInfoCount, const PipelineCreateInfo* pCreateInfos, Pipeline* pPipelines) = 0;
    virtual Result createComputePipelines(uint32_t createInfoCount, const PipelineCreateInfo* pCreateInfos, Pipeline* pPipelines) = 0;
    virtual void destroyPipeline(Pipeline pipeline) = 0;

    // Descriptor operations
    virtual Result createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout** outLayout) = 0;
    virtual void destroyDescriptorSetLayout(DescriptorSetLayout layout) = 0;
    virtual Result createDescriptorPool(const DescriptorPoolCreateInfo& info, DescriptorPool** outPool) = 0;
    virtual void destroyDescriptorPool(DescriptorPool pool) = 0;
    virtual Result allocateDescriptorSets(const DescriptorSetAllocateInfo& info, DescriptorSet* pDescriptorSets) = 0;
    virtual Result freeDescriptorSets(DescriptorPool pool, uint32_t descriptorSetCount, const DescriptorSet* pDescriptorSets) = 0;
    virtual Result updateDescriptorSets(uint32_t descriptorWriteCount, const KxWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const KxCopyDescriptorSet* pDescriptorCopies) = 0;

    // Command operations
    virtual Result createCommandPool(const CommandPoolCreateInfo& info, CommandPool** outPool) = 0;
    virtual void destroyCommandPool(CommandPool pool) = 0;
    virtual Result allocateCommandBuffers(const CommandBufferAllocateInfo& info, CommandBuffer* pCommandBuffers) = 0;
    virtual void freeCommandBuffers(CommandPool pool, uint32_t commandBufferCount, const CommandBuffer* pCommandBuffers) = 0;

    // Render pass / framebuffer
    virtual Result createRenderPass(const RenderPassCreateInfo& info, RenderPass** outRenderPass) = 0;
    virtual void destroyRenderPass(RenderPass renderPass) = 0;
    virtual Result createFramebuffer(const FramebufferCreateInfo& info, Framebuffer** outFramebuffer) = 0;
    virtual void destroyFramebuffer(Framebuffer framebuffer) = 0;

    // Swapchain
    virtual Result createSwapchain(const SwapchainCreateInfo& info, Swapchain** outSwapchain) = 0;
    virtual void destroySwapchain(Swapchain swapchain) = 0;
    virtual Result getSwapchainImages(Swapchain swapchain, uint32_t* pImageCount, Image* pImages) = 0;
    virtual Result acquireNextImage(Swapchain swapchain, uint64_t timeout, Semaphore semaphore, Fence fence, uint32_t* pImageIndex) = 0;

    // Synchronization
    virtual Result createSemaphore(const SemaphoreCreateInfo& info, Semaphore** outSemaphore) = 0;
    virtual void destroySemaphore(Semaphore semaphore) = 0;
    virtual Result createFence(const KxFenceCreateInfo& info, Fence** outFence) = 0;
    virtual void destroyFence(Fence fence) = 0;
    virtual Result waitForFences(uint32_t fenceCount, const Fence* pFences, KxBool32 waitAll, uint64_t timeout) = 0;
    virtual Result resetFences(uint32_t fenceCount, const Fence* pFences) = 0;
    virtual Result signalSemaphore(Semaphore semaphore, uint64_t value) = 0;
    virtual Result waitSemaphores(uint32_t semaphoreCount, const Semaphore* pSemaphores, const uint64_t* pValues, KxBool32 waitAll, uint64_t timeout) = 0;

    // Queue operations
    virtual Result getDeviceQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, Queue** outQueue) = 0;
    virtual Result queueSubmit(Queue* queue, uint32_t submitCount, const KxSubmitInfo* pSubmits, Fence fence) = 0;
    virtual Result queuePresent(Queue* queue, const KxPresentInfoKHR* pPresentInfo) = 0;
    virtual Result queueWaitIdle(Queue* queue) = 0;

    // Sampler
    virtual Result createSampler(const SamplerCreateInfo& info, Sampler** outSampler) = 0;
    virtual void destroySampler(Sampler* sampler) = 0;

    // Thread pool
    virtual Result createThreadPool(const ThreadPoolCreateInfo& info, ThreadPool** outThreadPool) = 0;
    virtual void destroyThreadPool(ThreadPool* threadPool) = 0;

    // Device properties
    virtual Result waitIdle() = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IDEVICE_H