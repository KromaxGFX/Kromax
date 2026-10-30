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
#define KX_PIPELINE_STAGE_TOP_OF_PIPE_BIT 0x00000001u
#define KX_PIPELINE_STAGE_DRAW_INDIRECT_BIT 0x00000002u
#define KX_PIPELINE_STAGE_VERTEX_INPUT_BIT 0x00000004u
#define KX_PIPELINE_STAGE_VERTEX_SHADER_BIT 0x00000008u
#define KX_PIPELINE_STAGE_FRAGMENT_SHADER_BIT 0x00000010u
#define KX_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT 0x00000020u
#define KX_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT 0x00000040u
#define KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 0x00000080u
#define KX_PIPELINE_STAGE_COMPUTE_SHADER_BIT 0x00000100u
#define KX_PIPELINE_STAGE_TRANSFER_BIT 0x00000200u
#define KX_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT 0x00000400u
#define KX_PIPELINE_STAGE_MAX_ENUM 0x7FFFFFFFu

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_STRUCTS_H */