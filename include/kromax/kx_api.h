#ifndef KX_API_H
#define KX_API_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"
#include "kx_pipeline.h"
#include "kx_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Instance functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateInstance(
    const KxInstanceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxInstance* pInstance);

KX_API void KX_CALL kxDestroyInstance(
    KxInstance instance,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxEnumeratePhysicalDevices(
    KxInstance instance,
    uint32_t* pPhysicalDeviceCount,
    KxPhysicalDevice* pPhysicalDevices);

KX_API KxResult KX_CALL kxGetPhysicalDeviceProperties(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceProperties* pProperties);

KX_API KxResult KX_CALL kxGetPhysicalDeviceFeatures(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceFeatures* pFeatures);

KX_API KxResult KX_CALL kxGetPhysicalDeviceMemoryProperties(
    KxPhysicalDevice physicalDevice,
    KxPhysicalDeviceMemoryProperties* pMemoryProperties);

KX_API KxResult KX_CALL kxGetPhysicalDeviceQueueFamilyProperties(
    KxPhysicalDevice physicalDevice,
    uint32_t* pQueueFamilyPropertyCount,
    KxQueueFamilyProperties* pQueueFamilyProperties);

KX_API KxResult KX_CALL kxGetPhysicalDeviceFormatProperties(
    KxPhysicalDevice physicalDevice,
    KxFormat format,
    void* pFormatProperties);

KX_API KxResult KX_CALL kxGetPhysicalDeviceImageFormatProperties(
    KxPhysicalDevice physicalDevice,
    KxFormat format,
    KxImageType type,
    KxImageTiling tiling,
    KxImageUsageFlags usage,
    KxImageCreateFlags flags,
    void* pImageFormatProperties);

KX_API KxPfnVoidFunction KX_CALL kxGetInstanceProcAddr(
    KxInstance instance,
    const char* pName);

/* =======================================================================
 * Device functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateDevice(
    KxPhysicalDevice physicalDevice,
    const KxDeviceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDevice* pDevice);

KX_API void KX_CALL kxDestroyDevice(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxGetDeviceQueue(
    KxDevice device,
    uint32_t queueFamilyIndex,
    uint32_t queueIndex,
    KxQueue* pQueue);

KX_API KxResult KX_CALL kxQueueSubmit(
    KxQueue queue,
    uint32_t submitCount,
    const KxSubmitInfo* pSubmits,
    KxFence fence);

KX_API KxResult KX_CALL kxQueueWaitIdle(
    KxQueue queue);

KX_API KxResult KX_CALL kxDeviceWaitIdle(
    KxDevice device);

KX_API KxPfnVoidFunction KX_CALL kxGetDeviceProcAddr(
    KxDevice device,
    const char* pName);

/* =======================================================================
 * Memory functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxAllocateMemory(
    KxDevice device,
    const KxMemoryAllocateInfo* pAllocateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDeviceMemory* pMemory);

KX_API void KX_CALL kxFreeMemory(
    KxDevice device,
    KxDeviceMemory memory,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxMapMemory(
    KxDevice device,
    KxDeviceMemory memory,
    uint64_t offset,
    uint64_t size,
    KxMemoryMapFlags flags,
    void** ppData);

KX_API void KX_CALL kxUnmapMemory(
    KxDevice device,
    KxDeviceMemory memory);

/* =======================================================================
 * Buffer functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateBuffer(
    KxDevice device,
    const KxBufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxBuffer* pBuffer);

KX_API void KX_CALL kxDestroyBuffer(
    KxDevice device,
    KxBuffer buffer,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxBindBufferMemory(
    KxDevice device,
    KxBuffer buffer,
    KxDeviceMemory memory,
    uint64_t memoryOffset);

/* =======================================================================
 * Image functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateImage(
    KxDevice device,
    const KxImageCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage);

KX_API void KX_CALL kxDestroyImage(
    KxDevice device,
    KxImage image,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxGetImageMemoryRequirements(
    KxDevice device,
    KxImage image,
    void* pMemoryRequirements);

KX_API KxResult KX_CALL kxBindImageMemory(
    KxDevice device,
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memoryOffset);

/* =======================================================================
 * Image View functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateImageView(
    KxDevice device,
    const KxImageViewCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImageView* pImageView);

KX_API void KX_CALL kxDestroyImageView(
    KxDevice device,
    KxImageView imageView,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Shader Module functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateShaderModule(
    KxDevice device,
    const KxShaderModuleCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule);

KX_API void KX_CALL kxDestroyShaderModule(
    KxDevice device,
    KxShaderModule shaderModule,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Pipeline Layout functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreatePipelineLayout(
    KxDevice device,
    const KxPipelineLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipelineLayout* pPipelineLayout);

KX_API void KX_CALL kxDestroyPipelineLayout(
    KxDevice device,
    KxPipelineLayout pipelineLayout,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Pipeline functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreatePipeline(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const KxPipelineCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipeline* pPipeline);

KX_API void KX_CALL kxDestroyPipeline(
    KxDevice device,
    KxPipeline pipeline,
    const KxAllocationCallbacks* pAllocator);

/* Pipeline Cache functions are declared in kx_pipeline_cache.h */

/* =======================================================================
 * Descriptor Set Layout functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateDescriptorSetLayout(
    KxDevice device,
    const KxDescriptorSetLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDescriptorSetLayout* pSetLayout);

KX_API void KX_CALL kxDestroyDescriptorSetLayout(
    KxDevice device,
    KxDescriptorSetLayout descriptorSetLayout,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Descriptor Pool functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateDescriptorPool(
    KxDevice device,
    const KxDescriptorPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDescriptorPool* pDescriptorPool);

KX_API void KX_CALL kxDestroyDescriptorPool(
    KxDevice device,
    KxDescriptorPool descriptorPool,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxResetDescriptorPool(
    KxDevice device,
    KxDescriptorPool descriptorPool,
    KxDescriptorPoolResetFlags flags);

/* =======================================================================
 * Descriptor Set functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxAllocateDescriptorSets(
    KxDevice device,
    const KxDescriptorSetAllocateInfo* pAllocateInfo,
    KxDescriptorSet* pDescriptorSets);

KX_API KxResult KX_CALL kxFreeDescriptorSets(
    KxDevice device,
    KxDescriptorPool descriptorPool,
    uint32_t descriptorSetCount,
    const KxDescriptorSet* pDescriptorSets);

KX_API void KX_CALL kxUpdateDescriptorSets(
    KxDevice device,
    uint32_t descriptorWriteCount,
    const KxWriteDescriptorSet* pDescriptorWrites,
    uint32_t descriptorCopyCount,
    const KxCopyDescriptorSet* pDescriptorCopies);

/* =======================================================================
 * Sampler functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateSampler(
    KxDevice device,
    const KxSamplerCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSampler* pSampler);

KX_API void KX_CALL kxDestroySampler(
    KxDevice device,
    KxSampler sampler,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Texture sampling
 * ======================================================================= */

KX_API KxResult KX_CALL kxSampleTexture(
    const KxBoundTexture* pTexture,
    float u,
    float v,
    float outColor[4]);

/* Sample texture with explicit derivatives for mip level selection */
KX_API KxResult KX_CALL kxSampleTextureGrad(
    const KxBoundTexture* pTexture,
    float u, float v,
    float dudx, float dvdx,
    float dudy, float dvdy,
    float outColor[4]);

/* Sample texture with explicit LOD */
KX_API KxResult KX_CALL kxSampleTextureLod(
    const KxBoundTexture* pTexture,
    float u, float v,
    float lod,
    float outColor[4]);

/* =======================================================================
 * Render Pass functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateRenderPass(
    KxDevice device,
    const KxRenderPassCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxRenderPass* pRenderPass);

KX_API void KX_CALL kxDestroyRenderPass(
    KxDevice device,
    KxRenderPass renderPass,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Framebuffer functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateFramebuffer(
    KxDevice device,
    const KxFramebufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxFramebuffer* pFramebuffer);

KX_API void KX_CALL kxDestroyFramebuffer(
    KxDevice device,
    KxFramebuffer framebuffer,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Command Pool functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateCommandPool(
    KxDevice device,
    const KxCommandPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxCommandPool* pCommandPool);

KX_API void KX_CALL kxDestroyCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxResetCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    KxCommandPoolResetFlags flags);

/* =======================================================================
 * Command Buffer functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxAllocateCommandBuffers(
    KxDevice device,
    const KxCommandBufferAllocateInfo* pAllocateInfo,
    KxCommandBuffer* pCommandBuffers);

KX_API void KX_CALL kxFreeCommandBuffers(
    KxDevice device,
    KxCommandPool commandPool,
    uint32_t commandBufferCount,
    const KxCommandBuffer* pCommandBuffers);

KX_API KxResult KX_CALL kxBeginCommandBuffer(
    KxCommandBuffer commandBuffer,
    const KxCommandBufferBeginInfo* pBeginInfo);

KX_API KxResult KX_CALL kxEndCommandBuffer(
    KxCommandBuffer commandBuffer);

KX_API KxResult KX_CALL kxResetCommandBuffer(
    KxCommandBuffer commandBuffer,
    KxCommandBufferResetFlags flags);

/* =======================================================================
 * Fence / Semaphore functions
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateFence(
    KxDevice device,
    const KxFenceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxFence* pFence);

KX_API void KX_CALL kxDestroyFence(
    KxDevice device,
    KxFence fence,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxWaitForFences(
    KxDevice device,
    uint32_t fenceCount,
    const KxFence* pFences,
    KxBool32 waitAll,
    uint64_t timeout);

KX_API KxResult KX_CALL kxResetFences(
    KxDevice device,
    uint32_t fenceCount,
    const KxFence* pFences);

KX_API KxResult KX_CALL kxGetFenceStatus(
    KxDevice device,
    KxFence fence);

KX_API KxResult KX_CALL kxCreateSemaphore(
    KxDevice device,
    const KxSemaphoreCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxSemaphore* pSemaphore);

KX_API void KX_CALL kxDestroySemaphore(
    KxDevice device,
    KxSemaphore semaphore,
    const KxAllocationCallbacks* pAllocator);

/* =======================================================================
 * Command recording functions
 * ======================================================================= */

KX_API void KX_CALL kxCmdBindPipeline(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipeline pipeline);

KX_API void KX_CALL kxCmdBindVertexBuffers(
    KxCommandBuffer commandBuffer,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const KxBuffer* pBuffers,
    const uint64_t* pOffsets);

KX_API void KX_CALL kxCmdBindIndexBuffer(
    KxCommandBuffer commandBuffer,
    KxBuffer buffer,
    uint64_t offset,
    KxIndexType indexType);

KX_API void KX_CALL kxCmdBindDescriptors(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const KxDescriptorSet* pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets);

KX_API void KX_CALL kxCmdPushConstants(
    KxCommandBuffer commandBuffer,
    KxPipelineLayout layout,
    KxShaderStageFlags stageFlags,
    uint32_t offset,
    uint32_t size,
    const void* pValues);

KX_API void KX_CALL kxCmdDraw(
    KxCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance);

KX_API void KX_CALL kxCmdDrawIndexed(
    KxCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance);

KX_API void KX_CALL kxCmdDrawIndirect(
    KxCommandBuffer commandBuffer,
    KxBuffer buffer,
    uint64_t offset,
    uint32_t drawCount,
    uint32_t stride);

KX_API void KX_CALL kxCmdDispatch(
    KxCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ);

KX_API void KX_CALL kxCmdDispatchIndirect(
    KxCommandBuffer commandBuffer,
    KxBuffer buffer,
    uint64_t offset);

/* Copy / Clear / Barrier */
KX_API void KX_CALL kxCmdCopyBuffer(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxBuffer dstBuffer,
    uint32_t regionCount,
    const void* pRegions);

KX_API void KX_CALL kxCmdCopyBufferToImage(
    KxCommandBuffer commandBuffer,
    KxBuffer srcBuffer,
    KxImage dstImage,
    KxImageLayout dstImageLayout,
    uint32_t regionCount,
    const void* pRegions);

KX_API void KX_CALL kxCmdCopyImage(
    KxCommandBuffer commandBuffer,
    KxImage srcImage,
    KxImageLayout srcImageLayout,
    KxImage dstImage,
    KxImageLayout dstImageLayout,
    uint32_t regionCount,
    const void* pRegions);

KX_API void KX_CALL kxCmdClearImage(
    KxCommandBuffer commandBuffer,
    KxImage image,
    KxImageLayout imageLayout,
    const void* pColor,
    const void* pDepthStencil,
    uint32_t rangeCount,
    const void* pRanges);

KX_API void KX_CALL kxCmdPipelineBarrier(
    KxCommandBuffer commandBuffer,
    KxPipelineStageFlags srcStageMask,
    KxPipelineStageFlags dstStageMask,
    KxDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const void* pMemoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const void* pBufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const void* pImageMemoryBarriers);

/* Render pass commands */
KX_API void KX_CALL kxCmdBeginRenderPass(
    KxCommandBuffer commandBuffer,
    const KxRenderPassBeginInfo* pRenderPassBegin,
    KxSubpassContents contents);

KX_API void KX_CALL kxCmdEndRenderPass(
    KxCommandBuffer commandBuffer);

KX_API void KX_CALL kxCmdNextSubpass(
    KxCommandBuffer commandBuffer,
    KxSubpassContents contents);

/* Dynamic Rendering (VK_KHR_dynamic_rendering) */
KX_API void KX_CALL kxCmdBeginRendering(
    KxCommandBuffer commandBuffer,
    const KxRenderingInfoKHR* pRenderingInfo);

KX_API void KX_CALL kxCmdEndRendering(
    KxCommandBuffer commandBuffer);

/* Synchronization 2 (VK_KHR_synchronization2) */
KX_API void KX_CALL kxCmdPipelineBarrier2(
    KxCommandBuffer commandBuffer,
    const KxDependencyInfoKHR* pDependencyInfo);

KX_API void KX_CALL kxCmdWriteTimestamp2(
    KxCommandBuffer commandBuffer,
    KxPipelineStageFlags2KHR stage,
    KxQueryPool queryPool,
    uint32_t query);

KX_API void KX_CALL kxCmdSetEvent2(
    KxCommandBuffer commandBuffer,
    KxEvent event,
    const KxDependencyInfoKHR* pDependencyInfo);

KX_API void KX_CALL kxCmdResetEvent2(
    KxCommandBuffer commandBuffer,
    KxEvent event,
    KxPipelineStageFlags2KHR stageMask);

KX_API void KX_CALL kxCmdWaitEvents2(
    KxCommandBuffer commandBuffer,
    uint32_t eventCount,
    const KxEvent* pEvents,
    const KxDependencyInfoKHR* pDependencyInfos);

/* Copy Commands v2 (VK_KHR_copy_commands2) */
KX_API void KX_CALL kxCmdCopyBuffer2(
    KxCommandBuffer commandBuffer,
    const void* pCopyBufferInfo);

KX_API void KX_CALL kxCmdCopyImage2(
    KxCommandBuffer commandBuffer,
    const void* pCopyImageInfo);

KX_API void KX_CALL kxCmdBlitImage2(
    KxCommandBuffer commandBuffer,
    const void* pBlitImageInfo);

KX_API void KX_CALL kxCmdCopyBufferToImage2(
    KxCommandBuffer commandBuffer,
    const void* pCopyBufferToImageInfo);

KX_API void KX_CALL kxCmdCopyImageToBuffer2(
    KxCommandBuffer commandBuffer,
    const void* pCopyImageToBufferInfo);

KX_API void KX_CALL kxCmdResolveImage2(
    KxCommandBuffer commandBuffer,
    const void* pResolveImageInfo);

/* Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3) */
KX_API void KX_CALL kxCmdSetDepthClampEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 depthClampEnable);

KX_API void KX_CALL kxCmdSetPolygonMode(
    KxCommandBuffer commandBuffer,
    KxPolygonMode polygonMode);

KX_API void KX_CALL kxCmdSetRasterizationSamples(
    KxCommandBuffer commandBuffer,
    KxSampleCountFlagBits rasterizationSamples);

KX_API void KX_CALL kxCmdSetSampleMask(
    KxCommandBuffer commandBuffer,
    KxSampleMask sampleMask);

KX_API void KX_CALL kxCmdSetAlphaToCoverageEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 alphaToCoverageEnable);

KX_API void KX_CALL kxCmdSetAlphaToOneEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 alphaToOneEnable);

KX_API void KX_CALL kxCmdSetLogicOpEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 logicOpEnable);

KX_API void KX_CALL kxCmdSetColorBlendEnable(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const KxBool32* pColorBlendEnables);

KX_API void KX_CALL kxCmdSetColorBlendEquation(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const void* pColorBlendEquations);

KX_API void KX_CALL kxCmdSetColorWriteMask(
    KxCommandBuffer commandBuffer,
    uint32_t firstAttachment,
    uint32_t attachmentCount,
    const KxColorComponentFlags* pColorWriteMasks);

KX_API void KX_CALL kxCmdSetTessellationDomainOrigin(
    KxCommandBuffer commandBuffer,
    KxTessellationDomainOriginKHR domainOrigin);

KX_API void KX_CALL kxCmdSetRasterizationStream(
    KxCommandBuffer commandBuffer,
    uint32_t rasterizationStream);

KX_API void KX_CALL kxCmdSetConservativeRasterizationMode(
    KxCommandBuffer commandBuffer,
    KxConservativeRasterizationModeEXT conservativeRasterizationMode);

KX_API void KX_CALL kxCmdSetExtraPrimitiveOverestimationSize(
    KxCommandBuffer commandBuffer,
    float extraPrimitiveOverestimationSize);

KX_API void KX_CALL kxCmdSetDepthClipEnable(
    KxCommandBuffer commandBuffer,
    KxBool32 depthClipEnable);

KX_API void KX_CALL kxCmdSetDepthClipNegativeOneToOne(
    KxCommandBuffer commandBuffer,
    KxBool32 negativeOneToOne);

KX_API void KX_CALL kxCmdSetViewportWScalingEnableNV(
    KxCommandBuffer commandBuffer,
    KxBool32 viewportWScalingEnable);

KX_API void KX_CALL kxCmdSetViewportSwizzleNV(
    KxCommandBuffer commandBuffer,
    uint32_t firstViewport,
    uint32_t viewportCount,
    const void* pViewportSwizzles);

/* =======================================================================
 * Utility functions
 * ======================================================================= */

const char* KX_CALL kxResultToString(KxResult result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_API_H */