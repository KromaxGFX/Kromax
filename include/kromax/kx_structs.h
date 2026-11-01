#ifndef KX_STRUCTS_H
#define KX_STRUCTS_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Physical device properties / features / memory
 * ======================================================================= */

typedef struct KxPhysicalDeviceProperties {
    uint32_t apiVersion;
    uint32_t driverVersion;
    uint32_t vendorID;
    uint32_t deviceID;
    KxPhysicalDeviceType deviceType;
    char deviceName[256];
    char driverName[256];
} KxPhysicalDeviceProperties;

typedef struct KxPhysicalDeviceFeatures {
    KxBool32 shaderFloat64;
    KxBool32 shaderInt64;
    KxBool32 geometryShader;
    KxBool32 tessellationShader;
    KxBool32 textureCompressionBC;
    KxBool32 textureCompressionETC2;
    KxBool32 textureCompressionASTC_LDR;
    KxBool32 pipelineStatisticsQuery;
    KxBool32 vertexPipelineStoresAndAtomics;
} KxPhysicalDeviceFeatures;

typedef uint32_t KxQueueFlags;
#define KX_QUEUE_GRAPHICS_BIT      0x00000001u
#define KX_QUEUE_COMPUTE_BIT       0x00000002u
#define KX_QUEUE_TRANSFER_BIT      0x00000004u
#define KX_QUEUE_SPARSE_BINDING_BIT 0x00000008u
#define KX_QUEUE_PROTECTED_BIT     0x00000010u
#define KX_QUEUE_MAX_ENUM          0x7FFFFFFFu

typedef struct KxQueueFamilyProperties {
    KxQueueFlags queueFlags;
    uint32_t queueCount;
    uint32_t timestampValidBits;
} KxQueueFamilyProperties;

typedef struct KxMemoryHeap {
    uint64_t size;
    uint32_t memoryTypeIndex;
} KxMemoryHeap;

typedef uint32_t KxMemoryPropertyFlags;
#define KX_MEMORY_PROPERTY_DEVICE_LOCAL_BIT       0x00000001u
#define KX_MEMORY_PROPERTY_HOST_VISIBLE_BIT       0x00000002u
#define KX_MEMORY_PROPERTY_HOST_COHERENT_BIT      0x00000004u
#define KX_MEMORY_PROPERTY_HOST_CACHED_BIT        0x00000008u
#define KX_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT   0x00000010u
#define KX_MEMORY_PROPERTY_PROTECTED_BIT          0x00000020u
#define KX_MEMORY_PROPERTY_MAX_ENUM               0x7FFFFFFFu

typedef struct KxMemoryType {
    uint32_t heapIndex;
    KxMemoryPropertyFlags propertyFlags;
} KxMemoryType;

typedef struct KxPhysicalDeviceMemoryProperties {
    uint32_t memoryHeapCount;
    KxMemoryHeap memoryHeaps[16];
    uint32_t memoryTypeCount;
    KxMemoryType memoryTypes[32];
} KxPhysicalDeviceMemoryProperties;

/* =======================================================================
 * Memory / Buffer / Image create info
 * ======================================================================= */

typedef uint32_t KxDeviceCreateFlags;
#define KX_DEVICE_CREATE_NONE 0u
#define KX_DEVICE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxDeviceQueueCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t queueFamilyIndex;
    uint32_t queueCount;
    const float* pQueuePriorities;
} KxDeviceQueueCreateInfo;

typedef struct KxDeviceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxDeviceCreateFlags flags;
    uint32_t queueCreateInfoCount;
    const void* pQueueCreateInfos;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
    const KxPhysicalDeviceFeatures* pEnabledFeatures;
} KxDeviceCreateInfo;

typedef uint32_t KxMemoryAllocateFlags;
#define KX_MEMORY_ALLOCATE_NONE 0u
#define KX_MEMORY_ALLOCATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxMemoryAllocateInfo {
    KxStructureType sType;
    const void* pNext;
    uint64_t allocationSize;
    uint32_t memoryTypeIndex;
} KxMemoryAllocateInfo;

typedef uint32_t KxBufferCreateFlags;
#define KX_BUFFER_CREATE_NONE 0u
#define KX_BUFFER_CREATE_SPARSE_BINDING_BIT 0x00000001u
#define KX_BUFFER_CREATE_SPARSE_RESIDENCY_BIT 0x00000002u
#define KX_BUFFER_CREATE_SPARSE_ALIASED_BIT 0x00000004u
#define KX_BUFFER_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxBufferUsageFlags;
#define KX_BUFFER_USAGE_TRANSFER_SRC_BIT           0x00000001u
#define KX_BUFFER_USAGE_TRANSFER_DST_BIT           0x00000002u
#define KX_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT   0x00000004u
#define KX_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT   0x00000008u
#define KX_BUFFER_USAGE_UNIFORM_BUFFER_BIT         0x00000010u
#define KX_BUFFER_USAGE_STORAGE_BUFFER_BIT         0x00000020u
#define KX_BUFFER_USAGE_INDEX_BUFFER_BIT           0x00000040u
#define KX_BUFFER_USAGE_VERTEX_BUFFER_BIT          0x00000080u
#define KX_BUFFER_USAGE_INDIRECT_BUFFER_BIT        0x00000100u
#define KX_BUFFER_USAGE_MAX_ENUM                   0x7FFFFFFFu

typedef struct KxBufferCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBufferCreateFlags flags;
    uint64_t size;
    KxBufferUsageFlags usage;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxBool32 sharingModeExclusive;
} KxBufferCreateInfo;

typedef struct KxBufferCopy {
    uint64_t srcOffset;
    uint64_t dstOffset;
    uint64_t size;
} KxBufferCopy;

typedef uint32_t KxImageCreateFlags;
#define KX_IMAGE_CREATE_NONE 0u
#define KX_IMAGE_CREATE_SPARSE_BINDING_BIT 0x00000001u
#define KX_IMAGE_CREATE_SPARSE_RESIDENCY_BIT 0x00000002u
#define KX_IMAGE_CREATE_MUTABLE_FORMAT_BIT 0x00000004u
#define KX_IMAGE_CREATE_CUBE_COMPATIBLE_BIT 0x00000008u
#define KX_IMAGE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxImageUsageFlags;
#define KX_IMAGE_USAGE_TRANSFER_SRC_BIT           0x00000001u
#define KX_IMAGE_USAGE_TRANSFER_DST_BIT           0x00000002u
#define KX_IMAGE_USAGE_SAMPLED_BIT                0x00000004u
#define KX_IMAGE_USAGE_STORAGE_BIT                0x00000008u
#define KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT       0x00000010u
#define KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 0x00000020u
#define KX_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT   0x00000040u
#define KX_IMAGE_USAGE_INPUT_ATTACHMENT_BIT       0x00000080u
#define KX_IMAGE_USAGE_MAX_ENUM                   0x7FFFFFFFu

typedef enum KxSampleCountFlagBits {
    KX_SAMPLE_COUNT_1_BIT = 0x00000001u,
    KX_SAMPLE_COUNT_2_BIT = 0x00000002u,
    KX_SAMPLE_COUNT_4_BIT = 0x00000004u,
    KX_SAMPLE_COUNT_8_BIT = 0x00000008u,
    KX_SAMPLE_COUNT_16_BIT = 0x00000010u,
    KX_SAMPLE_COUNT_32_BIT = 0x00000020u,
    KX_SAMPLE_COUNT_64_BIT = 0x00000040u,
    KX_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} KxSampleCountFlagBits;
typedef uint32_t KxSampleCountFlags;

typedef struct KxImageCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxImageCreateFlags flags;
    KxImageType imageType;
    KxFormat format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    KxSampleCountFlags samples;
    KxImageTiling tiling;
    KxImageUsageFlags usage;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxBool32 sharingModeExclusive;
    KxImageLayout initialLayout;
} KxImageCreateInfo;

typedef uint32_t KxImageViewCreateFlags;
#define KX_IMAGE_VIEW_CREATE_NONE 0u
#define KX_IMAGE_VIEW_CREATE_FRAGMENT_DENSITY_MAP_DYNAMIC_BIT_EXT 0x00000001u
#define KX_IMAGE_VIEW_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxComponentMapping {
    KxComponentSwizzle r;
    KxComponentSwizzle g;
    KxComponentSwizzle b;
    KxComponentSwizzle a;
} KxComponentMapping;

typedef struct KxImageSubresourceRange {
    KxImageAspectFlags aspectMask;
    uint32_t baseMipLevel;
    uint32_t levelCount;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
} KxImageSubresourceRange;

typedef uint32_t KxImageAspectFlags;
#define KX_IMAGE_ASPECT_NONE 0u
#define KX_IMAGE_ASPECT_COLOR_BIT 0x00000001u
#define KX_IMAGE_ASPECT_DEPTH_BIT 0x00000002u
#define KX_IMAGE_ASPECT_STENCIL_BIT 0x00000004u
#define KX_IMAGE_ASPECT_METADATA_BIT 0x00000008u
#define KX_IMAGE_ASPECT_MAX_ENUM 0x7FFFFFFFu

typedef struct KxImageViewCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxImageViewCreateFlags flags;
    KxImage image;
    KxImageViewType viewType;
    KxFormat format;
    KxComponentMapping components;
    KxImageSubresourceRange subresourceRange;
} KxImageViewCreateInfo;

typedef struct KxOffset2D {
    int32_t x, y;
} KxOffset2D;

typedef struct KxExtent2D {
    uint32_t width, height;
} KxExtent2D;

typedef struct KxRect2D {
    KxOffset2D offset;
    KxExtent2D extent;
} KxRect2D;

typedef struct KxOffset3D {
    int32_t x, y, z;
} KxOffset3D;

typedef struct KxExtent3D {
    uint32_t width, height, depth;
} KxExtent3D;

/* =======================================================================
 * Render pass / framebuffer
 * ======================================================================= */

typedef struct KxAttachmentDescription {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    KxFormat format;
    KxSampleCountFlags samples;
    KxAttachmentLoadOp loadOp;
    KxAttachmentStoreOp storeOp;
    KxAttachmentLoadOp stencilLoadOp;
    KxAttachmentStoreOp stencilStoreOp;
    KxImageLayout initialLayout;
    KxImageLayout finalLayout;
} KxAttachmentDescription;

typedef struct KxAttachmentReference {
    uint32_t attachment;
    KxImageLayout layout;
} KxAttachmentReference;

typedef struct KxSubpassDescription {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    KxPipelineBindPoint pipelineBindPoint;
    uint32_t inputAttachmentCount;
    const KxAttachmentReference* pInputAttachments;
    uint32_t colorAttachmentCount;
    const KxAttachmentReference* pColorAttachments;
    const KxAttachmentReference* pResolveAttachments;
    const KxAttachmentReference* pDepthStencilAttachment;
    uint32_t preserveAttachmentCount;
    const uint32_t* pPreserveAttachments;
} KxSubpassDescription;

typedef struct KxSubpassDependency {
    KxStructureType sType;
    const void* pNext;
    uint32_t srcSubpass;
    uint32_t dstSubpass;
    KxPipelineStageFlags srcStageMask;
    KxPipelineStageFlags dstStageMask;
    KxAccessFlags srcAccessMask;
    KxAccessFlags dstAccessMask;
    uint32_t dependencyFlags;
} KxSubpassDependency;

typedef struct KxRenderPassCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t attachmentCount;
    const KxAttachmentDescription* pAttachments;
    uint32_t subpassCount;
    const KxSubpassDescription* pSubpasses;
    uint32_t dependencyCount;
    const KxSubpassDependency* pDependencies;
} KxRenderPassCreateInfo;

typedef struct KxClearValue {
    union {
        float color[4];
        struct {
            float depth;
            uint32_t stencil;
        } depthStencil;
    };
} KxClearValue;

typedef struct KxRenderPassBeginInfo {
    KxStructureType sType;
    const void* pNext;
    KxRenderPass renderPass;
    KxFramebuffer framebuffer;
    KxRect2D renderArea;
    uint32_t clearValueCount;
    const KxClearValue* pClearValues;
} KxRenderPassBeginInfo;

typedef struct KxFramebufferCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    KxRenderPass renderPass;
    uint32_t attachmentCount;
    const KxImageView* pAttachments;
    uint32_t width;
    uint32_t height;
    uint32_t layers;
} KxFramebufferCreateInfo;

/* =======================================================================
 * Command pool / buffer
 * ======================================================================= */

typedef uint32_t KxCommandPoolCreateFlags;
#define KX_COMMAND_POOL_CREATE_NONE 0u
#define KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT 0x00000002u
#define KX_COMMAND_POOL_CREATE_TRANSIENT_BIT 0x00000004u
#define KX_COMMAND_POOL_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxCommandPoolCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxCommandPoolCreateFlags flags;
    uint32_t queueFamilyIndex;
} KxCommandPoolCreateInfo;

typedef uint32_t KxCommandBufferUsageFlags;
#define KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 0x00000001u
#define KX_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT 0x00000002u
#define KX_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT 0x00000004u
#define KX_COMMAND_BUFFER_USAGE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxCommandBufferAllocateInfo {
    KxStructureType sType;
    const void* pNext;
    KxCommandPool commandPool;
    KxCommandBufferLevel level;
    uint32_t commandBufferCount;
} KxCommandBufferAllocateInfo;

typedef struct KxCommandBufferBeginInfo {
    KxStructureType sType;
    const void* pNext;
    KxCommandBufferUsageFlags flags;
    const void* pInheritanceInfo;
} KxCommandBufferBeginInfo;

typedef uint32_t KxCommandBufferResetFlags;
#define KX_COMMAND_BUFFER_RESET_NONE 0u
#define KX_COMMAND_BUFFER_RESET_MAX_ENUM 0x7FFFFFFFu

/* =======================================================================
 * Fences / Semaphores
 * ======================================================================= */

typedef uint32_t KxFenceCreateFlags;
#define KX_FENCE_CREATE_SIGNALED_BIT 0x00000001u
#define KX_FENCE_CREATE_NONE 0u
#define KX_FENCE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxFenceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxFenceCreateFlags flags;
} KxFenceCreateInfo;

typedef uint32_t KxSemaphoreCreateFlags;
#define KX_SEMAPHORE_CREATE_NONE 0u
#define KX_SEMAPHORE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxSemaphoreCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxSemaphoreCreateFlags flags;
    uint64_t initialValue;  // Para timeline semaphores (0 = binary semaphore)
} KxSemaphoreCreateInfo;

typedef struct KxSubmitInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    const KxSemaphore* pWaitSemaphores;
    const KxPipelineStageFlags* pWaitDstStageMask;
    uint32_t commandBufferCount;
    const KxCommandBuffer* pCommandBuffers;
    uint32_t signalSemaphoreCount;
    const KxSemaphore* pSignalSemaphores;
} KxSubmitInfo;

typedef uint32_t KxPipelineStageFlags;
#define KX_PIPELINE_STAGE_MAX_ENUM 0x7FFFFFFFu

/* =======================================================================
 * Dynamic Rendering (VK_KHR_dynamic_rendering)
 * ======================================================================= */

#define KX_STRUCTURE_TYPE_RENDERING_INFO_KHR        1000044000
#define KX_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR 1000044001

typedef uint32_t KxRenderingFlags;
#define KX_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT 0x00000001
#define KX_RENDERING_SUSPENDING_BIT                     0x00000002
#define KX_RENDERING_RESUMING_BIT                       0x00000004
#define KX_RENDERING_MAX_ENUM                           0x7FFFFFFF

typedef struct KxRenderingAttachmentInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxImageView imageView;
    KxImageLayout imageLayout;
    KxResolveModeFlagBits resolveMode;
    KxImageView resolveImageView;
    KxImageLayout resolveImageLayout;
    KxAttachmentLoadOp loadOp;
    KxAttachmentStoreOp storeOp;
    KxClearValue clearValue;
} KxRenderingAttachmentInfoKHR;

typedef struct KxRenderingInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxRenderingFlags flags;
    KxRect2D renderArea;
    uint32_t layerCount;
    uint32_t viewMask;
    uint32_t colorAttachmentCount;
    const KxRenderingAttachmentInfoKHR* pColorAttachments;
    const KxRenderingAttachmentInfoKHR* pDepthAttachment;
    const KxRenderingAttachmentInfoKHR* pStencilAttachment;
} KxRenderingInfoKHR;

/* =======================================================================
 * Synchronization 2 (VK_KHR_synchronization2)
 * ======================================================================= */

#define KX_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR              1000314000
#define KX_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR             1000314001
#define KX_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR      1000314002
#define KX_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR       1000314003
#define KX_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO_KHR        1000314004
#define KX_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO_KHR   1000314005
#define KX_STRUCTURE_TYPE_SUBMIT_INFO_2_KHR                1000314006

typedef uint64_t KxPipelineStageFlags2KHR;
#define KX_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT_KHR          0x00000001
#define KX_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT_KHR        0x00000002
#define KX_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR         0x00000004
#define KX_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR        0x00000008
#define KX_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT_KHR 0x00000010
#define KX_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT_KHR 0x00000020
#define KX_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT_KHR      0x00000040
#define KX_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR      0x00000080
#define KX_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR 0x00000100
#define KX_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT_KHR  0x00000200
#define KX_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR 0x00000400
#define KX_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR       0x00000800
#define KX_PIPELINE_STAGE_2_ALL_TRANSFER_BIT_KHR         0x00001000
#define KX_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR         0x00002000
#define KX_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR         0x00004000
#define KX_PIPELINE_STAGE_2_NONE_KHR                     0x00008000
#define KX_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT   0x01000000
#define KX_PIPELINE_STAGE_2_CONDITIONAL_RENDERING_BIT_EXT 0x02000000
#define KX_PIPELINE_STAGE_2_COMMAND_PREPROCESS_BIT_NV    0x04000000
#define KX_PIPELINE_STAGE_2_FRAGMENT_SHADING_RATE_BIT_KHR 0x08000000
#define KX_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR 0x10000000
#define KX_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR   0x20000000
#define KX_PIPELINE_STAGE_2_MAX_ENUM_KHR                 0x7FFFFFFF

typedef uint32_t KxAccessFlags2KHR;
#define KX_ACCESS_2_INDIRECT_COMMAND_READ_BIT_KHR        0x00000001
#define KX_ACCESS_2_INDEX_READ_BIT_KHR                   0x00000002
#define KX_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR        0x00000004
#define KX_ACCESS_2_UNIFORM_READ_BIT_KHR                 0x00000008
#define KX_ACCESS_2_SHADER_READ_BIT_KHR                  0x00000010
#define KX_ACCESS_2_SHADER_WRITE_BIT_KHR                 0x00000020
#define KX_ACCESS_2_COLOR_ATTACHMENT_READ_BIT_KHR        0x00000040
#define KX_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR       0x00000080
#define KX_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR 0x00000100
#define KX_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR 0x00000200
#define KX_ACCESS_2_TRANSFER_READ_BIT_KHR                0x00000400
#define KX_ACCESS_2_TRANSFER_WRITE_BIT_KHR               0x00000800
#define KX_ACCESS_2_HOST_READ_BIT_KHR                    0x00001000
#define KX_ACCESS_2_HOST_WRITE_BIT_KHR                   0x00002000
#define KX_ACCESS_2_MEMORY_READ_BIT_KHR                  0x00004000
#define KX_ACCESS_2_MEMORY_WRITE_BIT_KHR                 0x00008000
#define KX_ACCESS_2_NONE_KHR                             0x0000FFFF
#define KX_ACCESS_2_MAX_ENUM_KHR                         0x7FFFFFFF

typedef struct KxDependencyInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxDependencyFlags dependencyFlags;
    uint32_t memoryBarrierCount;
    const struct KxMemoryBarrier2KHR* pMemoryBarriers;
    uint32_t bufferMemoryBarrierCount;
    const struct KxBufferMemoryBarrier2KHR* pBufferMemoryBarriers;
    uint32_t imageMemoryBarrierCount;
    const struct KxImageMemoryBarrier2KHR* pImageMemoryBarriers;
} KxDependencyInfoKHR;

typedef struct KxMemoryBarrier2KHR {
    KxStructureType sType;
    const void* pNext;
    KxAccessFlags2KHR srcAccessMask;
    KxAccessFlags2KHR dstAccessMask;
} KxMemoryBarrier2KHR;

typedef struct KxBufferMemoryBarrier2KHR {
    KxStructureType sType;
    const void* pNext;
    KxAccessFlags2KHR srcAccessMask;
    KxAccessFlags2KHR dstAccessMask;
    KxBuffer srcBuffer;
    KxBuffer dstBuffer;
    uint64_t srcOffset;
    uint64_t dstOffset;
    uint64_t size;
} KxBufferMemoryBarrier2KHR;

typedef struct KxImageMemoryBarrier2KHR {
    KxStructureType sType;
    const void* pNext;
    KxAccessFlags2KHR srcAccessMask;
    KxAccessFlags2KHR dstAccessMask;
    KxImageLayout oldLayout;
    KxImageLayout newLayout;
    KxImage srcImage;
    KxImage dstImage;
    KxImageSubresourceRange subresourceRange;
} KxImageMemoryBarrier2KHR;

typedef struct KxSemaphoreSubmitInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxSemaphore semaphore;
    uint64_t value;
    KxPipelineStageFlags2KHR stageMask;
    uint32_t deviceIndex;
} KxSemaphoreSubmitInfoKHR;

typedef struct KxCommandBufferSubmitInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxCommandBuffer commandBuffer;
    uint32_t deviceMask;
} KxCommandBufferSubmitInfoKHR;

typedef struct KxSubmitInfo2KHR {
    KxStructureType sType;
    const void* pNext;
    KxSubmitFlags flags;
    uint32_t waitSemaphoreInfoCount;
    const KxSemaphoreSubmitInfoKHR* pWaitSemaphoreInfos;
    uint32_t commandBufferInfoCount;
    const KxCommandBufferSubmitInfoKHR* pCommandBufferInfos;
    uint32_t signalSemaphoreInfoCount;
    const KxSemaphoreSubmitInfoKHR* pSignalSemaphoreInfos;
} KxSubmitInfo2KHR;

/* =======================================================================
 * ARM64
 * ======================================================================= */

#define KX_STRUCTURE_TYPE_ARM_CPU_INFO 1000001000

typedef struct KxArmCpuInfo {
    KxStructureType sType;
    void* pNext;
    uint32_t features;
    uint32_t implementer;
    uint32_t variant;
    uint32_t part;
    uint32_t revision;
    uint32_t cpuid;
    char name[64];
} KxArmCpuInfo;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_STRUCTS_H */