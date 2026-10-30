#ifndef KX_TYPES_H
#define KX_TYPES_H

#include "kx_platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Result codes
 * ======================================================================= */

typedef enum KxResult {
    /* Success codes (>= 0) */
    KX_SUCCESS = 0,
    KX_NOT_READY = 1,
    KX_TIMEOUT = 2,
    KX_EVENT_SET = 3,
    KX_EVENT_RESET = 4,
    KX_INCOMPLETE = 5,

    /* Error codes (< 0) */
    KX_ERROR_OUT_OF_HOST_MEMORY = -1,
    KX_ERROR_OUT_OF_DEVICE_MEMORY = -2,
    KX_ERROR_INITIALIZATION_FAILED = -3,
    KX_ERROR_DEVICE_LOST = -4,
    KX_ERROR_MEMORY_MAP_FAILED = -5,
    KX_ERROR_LAYER_NOT_PRESENT = -6,
    KX_ERROR_EXTENSION_NOT_PRESENT = -7,
    KX_ERROR_FEATURE_NOT_PRESENT = -8,
    KX_ERROR_INCOMPATIBLE_DRIVER = -9,
    KX_ERROR_TOO_MANY_OBJECTS = -10,
    KX_ERROR_FORMAT_NOT_SUPPORTED = -11,
    KX_ERROR_FRAGMENTED_POOL = -12,
    KX_ERROR_OUT_OF_POOL_MEMORY = -13,
    KX_ERROR_INVALID_EXTERNAL_HANDLE = -14,
    KX_ERROR_INVALID_ARGUMENT = -15,

    /* Extension / future error codes */
    KX_ERROR_INCOMPATIBLE_DISPLAY_KHR = -1000000004,
    KX_ERROR_VALIDATION_FAILED_EXT = -1000000015,
} KxResult;

/* Bool32 type for C compatibility */
typedef uint32_t KxBool32;
#define KX_TRUE 1u
#define KX_FALSE 0u

/* Pack a semver into a uint32_t: major[21:22] minor[12:21] patch[0:11] */
#define KX_MAKE_VERSION(major, minor, patch) \
    ((uint32_t)(((uint32_t)(major) << 22) | ((uint32_t)(minor) << 12) | (uint32_t)(patch)))

/* Current public API version: 0.1.0 */
#define KX_API_VERSION_0_1 KX_MAKE_VERSION(0, 1, 0)
#define KX_API_VERSION_MAJOR 0u
#define KX_API_VERSION_MINOR 1u
#define KX_API_VERSION_PATCH 0u
#define KX_API_VERSION KX_API_VERSION_0_1

/* =======================================================================
 * Structure types & extensibility (sType / pNext)
 * ======================================================================= */

typedef enum KxStructureType {
    KX_STRUCTURE_TYPE_APPLICATION_INFO = 1,
    KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 2,
    KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO = 3,
    KX_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO = 4,
    KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5,
    KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO = 6,
    KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 7,
    KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 8,
    KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO = 9,
    KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 10,
    KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO = 11,
    KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO = 12,
    KX_STRUCTURE_TYPE_COMMAND_BUFFER_CREATE_INFO = 13,
    KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO = 14,
    KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 15,
    KX_STRUCTURE_TYPE_FENCE_CREATE_INFO = 16,
    KX_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 17,
    KX_STRUCTURE_TYPE_SUBMIT_INFO = 18,
    KX_STRUCTURE_TYPE_PRESENT_INFO_EXT = 1000,
    KX_STRUCTURE_TYPE_DISPLAY_CREATE_INFO_EXT = 1001,
    KX_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_EXT = 1002,
    KX_STRUCTURE_TYPE_DEBUG_MESSENGER_CREATE_INFO_EXT = 2000,
    KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION = 19,
    KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION = 20,
    KX_STRUCTURE_TYPE_SUBPASS_DEPENDENCY = 21,
    KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 22,
    KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO = 23,
    KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 24,

    KX_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = 25,
    KX_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO = 26,
    KX_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = 27,
    KX_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = 28,
    KX_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO = 29,
    KX_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = 30,
    KX_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO = 31,
    KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = 32,
    KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO = 33,
    KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO = 34,
    KX_STRUCTURE_TYPE_SAMPLER_CREATE_INFO = 35,
    KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET = 36,
    KX_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET = 37,

    KX_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxStructureType;

/* =======================================================================
 * Opaque handle definitions
 * ======================================================================= */

KX_DEFINE_HANDLE(KxInstance)
KX_DEFINE_HANDLE(KxPhysicalDevice)
KX_DEFINE_HANDLE(KxDevice)
KX_DEFINE_HANDLE(KxQueue)
KX_DEFINE_HANDLE(KxDeviceMemory)
KX_DEFINE_HANDLE(KxBuffer)
KX_DEFINE_HANDLE(KxImage)
KX_DEFINE_HANDLE(KxImageView)
KX_DEFINE_HANDLE(KxShaderModule)
KX_DEFINE_HANDLE(KxPipelineLayout)
KX_DEFINE_HANDLE(KxPipeline)
KX_DEFINE_HANDLE(KxCommandPool)
KX_DEFINE_HANDLE(KxCommandBuffer)
KX_DEFINE_HANDLE(KxFence)
KX_DEFINE_HANDLE(KxSemaphore)
KX_DEFINE_HANDLE(KxRenderPass)
KX_DEFINE_HANDLE(KxFramebuffer)
KX_DEFINE_HANDLE(KxDescriptorSetLayout)
KX_DEFINE_HANDLE(KxDescriptorPool)
KX_DEFINE_HANDLE(KxDescriptorSet)
KX_DEFINE_HANDLE(KxSampler)

/* =======================================================================
 * Allocation callbacks
 * ======================================================================= */

typedef void (KX_CALL* KxPfnVoidFunction)(void);

typedef enum KxSystemAllocationScope {
    KX_SYSTEM_ALLOCATION_SCOPE_COMMAND = 0,
    KX_SYSTEM_ALLOCATION_SCOPE_OBJECT = 1,
    KX_SYSTEM_ALLOCATION_SCOPE_CACHE = 2,
    KX_SYSTEM_ALLOCATION_SCOPE_DEVICE = 3,
    KX_SYSTEM_ALLOCATION_SCOPE_INSTANCE = 4,
    KX_SYSTEM_ALLOCATION_SCOPE_MAX_ENUM = 0x7FFFFFFF
} KxSystemAllocationScope;

typedef enum KxInternalAllocationType {
    KX_INTERNAL_ALLOCATION_TYPE_EXECUTABLE = 0,
    KX_INTERNAL_ALLOCATION_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxInternalAllocationType;

typedef struct KxAllocationCallbacks {
    void* pUserData;
    void* (KX_CALL* pfnAllocation)(void* pUserData, size_t size, size_t alignment, KxSystemAllocationScope allocationScope);
    void* (KX_CALL* pfnReallocation)(void* pUserData, void* pOriginal, size_t size, size_t alignment, KxSystemAllocationScope allocationScope);
    void (KX_CALL* pfnFree)(void* pUserData, void* pMemory);
    void (KX_CALL* pfnInternalAllocation)(void* pUserData, size_t size, KxInternalAllocationType allocationType);
    void (KX_CALL* pfnInternalFree)(void* pUserData, size_t size, KxInternalAllocationType allocationType);
} KxAllocationCallbacks;

/* =======================================================================
 * Application info
 * ======================================================================= */

typedef struct KxApplicationInfo {
    KxStructureType sType;
    const void* pNext;
    const char* pApplicationName;
    uint32_t applicationVersion;
    const char* pEngineName;
    uint32_t engineVersion;
    uint32_t apiVersion;
} KxApplicationInfo;

/* =======================================================================
 * Instance create info
 * ======================================================================= */

typedef uint32_t KxInstanceCreateFlags;
#define KX_INSTANCE_CREATE_NONE 0u
#define KX_INSTANCE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxInstanceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxInstanceCreateFlags flags;
    const KxApplicationInfo* pApplicationInfo;
    uint32_t enabledLayerCount;
    const char* const* ppEnabledLayerNames;
    uint32_t enabledExtensionCount;
    const char* const* ppEnabledExtensionNames;
} KxInstanceCreateInfo;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_TYPES_H */