#ifndef KX_CORE_H
 /* callbacks, opaque handles, object creation/info structs, and every */
#define KX_CORE_H
 /* core kx* function. Functions that are declared but not yet implemented */
 /* return KX_ERROR_FEATURE_NOT_PRESENT. */

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
    KX_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxStructureType;

/* Base structure that must begin every input struct for pNext chaining */
typedef struct KxBaseInStructure {
    KxStructureType sType;
    const void* pNext;
} KxBaseInStructure;

/* Base structure for output structs */
typedef struct KxBaseOutStructure {
    KxStructureType sType;
    void* pNext;
} KxBaseOutStructure;
/* =======================================================================
 * Allocation callbacks
 * ======================================================================= */

typedef struct KxAllocationCallbacks KxAllocationCallbacks;

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
    KX_INTERNAL_ALLOCATION_TYPE_DESCRIPTOR = 1,
    KX_INTERNAL_ALLOCATION_TYPE_DESCRIPTOR_SET = 2,
    KX_INTERNAL_ALLOCATION_TYPE_FRAMEBUFFER = 3,
    KX_INTERNAL_ALLOCATION_TYPE_COMMAND_BUFFER = 4,
    KX_INTERNAL_ALLOCATION_TYPE_IMAGE = 5,
    KX_INTERNAL_ALLOCATION_TYPE_IMAGEVIEW = 6,
    KX_INTERNAL_ALLOCATION_TYPE_BUFFER = 7,
    KX_INTERNAL_ALLOCATION_TYPE_DEVICE_MEMORY = 8,
    KX_INTERNAL_ALLOCATION_TYPE_SAMPLER = 9,
    KX_INTERNAL_ALLOCATION_TYPE_SHADER_MODULE = 10,
    KX_INTERNAL_ALLOCATION_TYPE_PIPELINE = 11,
    KX_INTERNAL_ALLOCATION_TYPE_PIPELINE_LAYOUT = 12,
    KX_INTERNAL_ALLOCATION_TYPE_RENDER_PASS = 13,
    KX_INTERNAL_ALLOCATION_TYPE_DESCRIPTOR_POOL = 14,
    KX_INTERNAL_ALLOCATION_TYPE_DESCRIPTOR_SET_TEMP = 15,
    KX_INTERNAL_ALLOCATION_TYPE_COMMAND_POOL = 16,
    KX_INTERNAL_ALLOCATION_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxInternalAllocationType;

typedef void* (KX_CALL *KxPfnAllocationFunction)(
    void* pUserData,
    size_t size,
    size_t alignment,
    KxSystemAllocationScope allocationScope);

typedef void* (KX_CALL *KxPfnReallocationFunction)(
    void* pUserData,
    void* pOriginal,
    size_t size,
    size_t alignment,
    KxSystemAllocationScope allocationScope);

typedef void (KX_CALL *KxPfnFreeFunction)(
    void* pUserData,
    void* pMemory);

typedef void (KX_CALL *KxPfnInternalAllocationNotification)(
    void* pUserData,
    size_t size,
    KxInternalAllocationType allocationType,
    KxSystemAllocationScope allocationScope);

typedef void (KX_CALL *KxPfnInternalFreeNotification)(
    void* pUserData,
    size_t size,
    KxInternalAllocationType allocationType,
    KxSystemAllocationScope allocationScope);

struct KxAllocationCallbacks {
    void* pUserData;
    KxPfnAllocationFunction pfnAllocation;
    KxPfnReallocationFunction pfnReallocation;
    KxPfnFreeFunction pfnFree;
    KxPfnInternalAllocationNotification pfnInternalAllocation;
    KxPfnInternalFreeNotification pfnInternalFree;
};

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
typedef uint32_t KxInstanceCreateFlags;
#define KX_INSTANCE_CREATE_NONE 0u

typedef uint32_t KxDeviceCreateFlags;
#define KX_DEVICE_CREATE_NONE 0u

typedef uint32_t KxMemoryPropertyFlags;
#define KX_MEMORY_PROPERTY_DEVICE_LOCAL_BIT 0x00000001u
#define KX_MEMORY_PROPERTY_HOST_VISIBLE_BIT 0x00000002u
#define KX_MEMORY_PROPERTY_HOST_COHERENT_BIT 0x00000004u
#define KX_MEMORY_PROPERTY_HOST_CACHED_BIT 0x00000008u
#define KX_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT 0x00000010u
#define KX_MEMORY_PROPERTY_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxBufferCreateFlags;
#define KX_BUFFER_CREATE_NONE 0u

typedef uint32_t KxBufferUsageFlags;
#define KX_BUFFER_USAGE_TRANSFER_SRC_BIT 0x00000001u
#define KX_BUFFER_USAGE_TRANSFER_DST_BIT 0x00000002u
#define KX_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT 0x00000004u
#define KX_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT 0x00000008u
#define KX_BUFFER_USAGE_UNIFORM_BUFFER_BIT 0x00000010u
#define KX_BUFFER_USAGE_STORAGE_BUFFER_BIT 0x00000020u
#define KX_BUFFER_USAGE_INDEX_BUFFER_BIT 0x00000040u
#define KX_BUFFER_USAGE_VERTEX_BUFFER_BIT 0x00000080u
#define KX_BUFFER_USAGE_INDIRECT_BUFFER_BIT 0x00000100u
#define KX_BUFFER_USAGE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxImageCreateFlags;
#define KX_IMAGE_CREATE_NONE 0u

typedef uint32_t KxImageUsageFlags;
#define KX_IMAGE_USAGE_TRANSFER_SRC_BIT 0x00000001u
#define KX_IMAGE_USAGE_TRANSFER_DST_BIT 0x00000002u
#define KX_IMAGE_USAGE_SAMPLED_BIT 0x00000004u
#define KX_IMAGE_USAGE_STORAGE_BIT 0x00000008u
#define KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT 0x00000010u
#define KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT 0x00000020u
#define KX_IMAGE_USAGE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxImageAspectFlags;
#define KX_IMAGE_ASPECT_COLOR_BIT 0x00000001u
#define KX_IMAGE_ASPECT_DEPTH_BIT 0x00000002u
#define KX_IMAGE_ASPECT_STENCIL_BIT 0x00000004u
#define KX_IMAGE_ASPECT_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxImageViewCreateFlags;
#define KX_IMAGE_VIEW_CREATE_NONE 0u

typedef uint32_t KxShaderModuleCreateFlags;
#define KX_SHADER_MODULE_CREATE_NONE 0u

typedef uint32_t KxPipelineCreateFlags;
#define KX_PIPELINE_CREATE_NONE 0u

typedef uint32_t KxPipelineLayoutCreateFlags;
#define KX_PIPELINE_LAYOUT_CREATE_NONE 0u

typedef uint32_t KxCommandPoolCreateFlags;
#define KX_COMMAND_POOL_CREATE_NONE 0u

typedef uint32_t KxCommandBufferUsageFlags;
#define KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 0x00000001u
#define KX_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT 0x00000002u
#define KX_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT 0x00000004u
#define KX_COMMAND_BUFFER_USAGE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxFenceCreateFlags;
#define KX_FENCE_CREATE_SIGNALED_BIT 0x00000001u
#define KX_FENCE_CREATE_NONE 0u

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

typedef uint32_t KxAccessFlags;
#define KX_ACCESS_INDIRECT_COMMAND_READ_BIT 0x00000001u
#define KX_ACCESS_INDEX_READ_BIT 0x00000002u
#define KX_ACCESS_VERTEX_ATTRIBUTE_READ_BIT 0x00000004u
#define KX_ACCESS_UNIFORM_READ_BIT 0x00000008u
#define KX_ACCESS_INPUT_ATTACHMENT_READ_BIT 0x00000010u
#define KX_ACCESS_SHADER_READ_BIT 0x00000020u
#define KX_ACCESS_SHADER_WRITE_BIT 0x00000040u
#define KX_ACCESS_COLOR_ATTACHMENT_READ_BIT 0x00000080u
#define KX_ACCESS_COLOR_ATTACHMENT_WRITE_BIT 0x00000100u
#define KX_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT 0x00000200u
#define KX_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT 0x00000400u
#define KX_ACCESS_TRANSFER_READ_BIT 0x00000800u
#define KX_ACCESS_TRANSFER_WRITE_BIT 0x00001000u
#define KX_ACCESS_HOST_READ_BIT 0x00002000u
#define KX_ACCESS_HOST_WRITE_BIT 0x00004000u
#define KX_ACCESS_MEMORY_READ_BIT 0x00008000u
#define KX_ACCESS_MEMORY_WRITE_BIT 0x00010000u
#define KX_ACCESS_MAX_ENUM 0x7FFFFFFFu


typedef uint32_t KxColorComponentFlags;
#define KX_COLOR_COMPONENT_R_BIT 0x00000001u
#define KX_COLOR_COMPONENT_G_BIT 0x00000002u
#define KX_COLOR_COMPONENT_B_BIT 0x00000004u
#define KX_COLOR_COMPONENT_A_BIT 0x00000008u
#define KX_COLOR_COMPONENT_MAX_ENUM 0x7FFFFFFFu



typedef enum KxPhysicalDeviceType {
    KX_PHYSICAL_DEVICE_TYPE_OTHER = 0,
    KX_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
    KX_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
    KX_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
    KX_PHYSICAL_DEVICE_TYPE_CPU = 4,
    KX_PHYSICAL_DEVICE_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxPhysicalDeviceType;

typedef enum KxFormat {
    KX_FORMAT_UNDEFINED = 0,
    KX_FORMAT_R8_UNORM = 1,
    KX_FORMAT_R8_SNORM = 2,
    KX_FORMAT_R8_USCALED = 3,
    KX_FORMAT_R8_SSCALED = 4,
    KX_FORMAT_R8_UINT = 5,
    KX_FORMAT_R8_SINT = 6,
    KX_FORMAT_R16_UNORM = 7,
    KX_FORMAT_R16_SNORM = 8,
    KX_FORMAT_R16_USCALED = 9,
    KX_FORMAT_R16_SSCALED = 10,
    KX_FORMAT_R16_UINT = 11,
    KX_FORMAT_R16_SINT = 12,
    KX_FORMAT_R16_SFLOAT = 13,
    KX_FORMAT_R8G8_UNORM = 14,
    KX_FORMAT_R8G8_SNORM = 15,
    KX_FORMAT_R8G8_UINT = 16,
    KX_FORMAT_R8G8_SINT = 17,
    KX_FORMAT_R32_SFLOAT = 18,
    KX_FORMAT_R32G32_SFLOAT = 19,
    KX_FORMAT_R32G32B32_SFLOAT = 20,
    KX_FORMAT_R32G32B32A32_SFLOAT = 21,
    KX_FORMAT_B8G8R8A8_UNORM = 22,
    KX_FORMAT_B8G8R8A8_SRGB = 23,
    KX_FORMAT_R8G8B8A8_UNORM = 24,
    KX_FORMAT_R8G8B8A8_SRGB = 25,
    KX_FORMAT_B8G8R8_UNORM = 26,
    KX_FORMAT_D32_SFLOAT = 100,
    KX_FORMAT_D16_UNORM = 101,
    KX_FORMAT_D24_UNORM_S8_UINT = 102,
    KX_FORMAT_MAX_ENUM = 0x7FFFFFFF
} KxFormat;

typedef enum KxImageType {
    KX_IMAGE_TYPE_1D = 0,
    KX_IMAGE_TYPE_2D = 1,
    KX_IMAGE_TYPE_3D = 2,
    KX_IMAGE_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxImageType;

typedef enum KxImageTiling {
    KX_IMAGE_TILING_OPTIMAL = 0,
    KX_IMAGE_TILING_LINEAR = 1,
    KX_IMAGE_TILING_MAX_ENUM = 0x7FFFFFFF
} KxImageTiling;

typedef enum KxImageLayout {
    KX_IMAGE_LAYOUT_UNDEFINED = 0,
    KX_IMAGE_LAYOUT_GENERAL = 1,
    KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
    KX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    KX_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
    KX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
    KX_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    KX_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    KX_IMAGE_LAYOUT_PREINITIALIZED = 8,
    KX_IMAGE_LAYOUT_MAX_ENUM = 0x7FFFFFFF
} KxImageLayout;

typedef enum KxPresentModeKHR {
    KX_PRESENT_MODE_IMMEDIATE_KHR = 0,
    KX_PRESENT_MODE_MAILBOX_KHR = 1,
    KX_PRESENT_MODE_FIFO_KHR = 2,
    KX_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
    KX_PRESENT_MODE_MAX_ENUM_KHR = 0x7FFFFFFF
} KxPresentModeKHR;

typedef enum KxShaderFormat {
    KX_SHADER_FORMAT_UNDEFINED = 0,
    KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT = 1000,
    KX_SHADER_FORMAT_KIR_EXT = 2000,
    KX_SHADER_FORMAT_MAX_ENUM = 0x7FFFFFFF
} KxShaderFormat;

/* =======================================================================
 * Shader callback ABI (for KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT)
 * ======================================================================= */

/* Vertex shader input/output */
typedef struct KxVertexInput {
    const float* position;      /* vec3 or vec4 */
    const float* normal;        /* vec3 */
    const float* texcoord;      /* vec2 */
    const float* color;         /* vec4 */
    uint32_t instance_index;
    uint32_t vertex_index;
} KxVertexInput;

typedef struct KxVertexOutput {
    float position[4];          /* clip space position (vec4) */
    float normal[3];            /* world space normal */
    float texcoord[2];          /* texture coordinates */
    float color[4];             /* vertex color */
    float gl_Position[4];       /* built-in: clip space position */
} KxVertexOutput;

/* Fragment shader input/output */
typedef struct KxFragmentInput {
    float frag_coord[4];        /* fragment coordinate */
    float front_facing;         /* 1.0 if front facing, 0.0 otherwise */
    float point_coord[2];       /* point sprite coordinates */
    float normal[3];            /* interpolated normal */
    float texcoord[2];          /* interpolated texcoord */
    float color[4];             /* interpolated color */
} KxFragmentInput;

typedef struct KxFragmentOutput {
    float color[4];             /* output color (SV_Target0) */
    float depth;                /* output depth (SV_Depth) */
    uint32_t stencil;           /* output stencil */
} KxFragmentOutput;

/* Compute shader input/output */
typedef struct KxComputeInput {
    uint32_t group_id[3];       /* workgroup ID */
    uint32_t local_id[3];       /* local invocation ID */
    uint32_t global_id[3];      /* global invocation ID */
    uint32_t local_index;       /* linear local index */
} KxComputeInput;

typedef struct KxComputeOutput {
    /* No built-in outputs for compute; use buffer/image stores */
} KxComputeOutput;

/* Shader callback function signatures */
typedef void (KX_CALL* KxVertexShaderCallback)(const KxVertexInput* input, KxVertexOutput* output, const void* push_constants);
typedef void (KX_CALL* KxFragmentShaderCallback)(const KxFragmentInput* input, KxFragmentOutput* output, const void* push_constants);
typedef void (KX_CALL* KxComputeShaderCallback)(const KxComputeInput* input, KxComputeOutput* output, const void* push_constants);

/* Shader module create info for native callbacks */
typedef struct KxShaderModuleNativeCallbackInfo {
    KxVertexShaderCallback vertex_fn;
    KxFragmentShaderCallback fragment_fn;
    KxComputeShaderCallback compute_fn;
    const void* push_constants;
    uint32_t push_constants_size;
} KxShaderModuleNativeCallbackInfo;

typedef enum KxPrimitiveTopology {
    KX_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
    KX_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
    KX_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
    KX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
    KX_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
    KX_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
    KX_PRIMITIVE_TOPOLOGY_MAX_ENUM = 0x7FFFFFFF
} KxPrimitiveTopology;

typedef enum KxPolygonMode {
    KX_POLYGON_MODE_FILL = 0,
    KX_POLYGON_MODE_LINE = 1,
    KX_POLYGON_MODE_POINT = 2,
    KX_POLYGON_MODE_MAX_ENUM = 0x7FFFFFFF
} KxPolygonMode;

typedef enum KxFrontFace {
    KX_FRONT_FACE_COUNTER_CLOCKWISE = 0,
    KX_FRONT_FACE_CLOCKWISE = 1,
    KX_FRONT_FACE_MAX_ENUM = 0x7FFFFFFF
} KxFrontFace;

typedef enum KxCompareOp {
    KX_COMPARE_OP_NEVER = 0,
    KX_COMPARE_OP_LESS = 1,
    KX_COMPARE_OP_EQUAL = 2,
    KX_COMPARE_OP_LESS_OR_EQUAL = 3,
    KX_COMPARE_OP_GREATER = 4,
    KX_COMPARE_OP_NOT_EQUAL = 5,
    KX_COMPARE_OP_GREATER_OR_EQUAL = 6,
    KX_COMPARE_OP_ALWAYS = 7,
    KX_COMPARE_OP_MAX_ENUM = 0x7FFFFFFF
} KxCompareOp;

typedef enum KxStencilOp {
    KX_STENCIL_OP_KEEP = 0,
    KX_STENCIL_OP_ZERO = 1,
    KX_STENCIL_OP_REPLACE = 2,
    KX_STENCIL_OP_INCREMENT_AND_CLAMP = 3,
    KX_STENCIL_OP_DECREMENT_AND_CLAMP = 4,
    KX_STENCIL_OP_INVERT = 5,
    KX_STENCIL_OP_INCREMENT_AND_WRAP = 6,
    KX_STENCIL_OP_DECREMENT_AND_WRAP = 7,
    KX_STENCIL_OP_MAX_ENUM = 0x7FFFFFFF
} KxStencilOp;

typedef enum KxLogicOp {
    KX_LOGIC_OP_CLEAR = 0,
    KX_LOGIC_OP_AND = 1,
    KX_LOGIC_OP_AND_REVERSE = 2,
    KX_LOGIC_OP_COPY = 3,
    KX_LOGIC_OP_AND_INVERTED = 4,
    KX_LOGIC_OP_NO_OP = 5,
    KX_LOGIC_OP_XOR = 6,
    KX_LOGIC_OP_OR = 7,
    KX_LOGIC_OP_NOR = 8,
    KX_LOGIC_OP_EQUIVALENT = 9,
    KX_LOGIC_OP_INVERT = 10,
    KX_LOGIC_OP_OR_REVERSE = 11,
    KX_LOGIC_OP_COPY_INVERTED = 12,
    KX_LOGIC_OP_OR_INVERTED = 13,
    KX_LOGIC_OP_NAND = 14,
    KX_LOGIC_OP_SET = 15,
    KX_LOGIC_OP_MAX_ENUM = 0x7FFFFFFF
} KxLogicOp;

typedef enum KxBlendFactor {
    KX_BLEND_FACTOR_ZERO = 0,
    KX_BLEND_FACTOR_ONE = 1,
    KX_BLEND_FACTOR_SRC_COLOR = 2,
    KX_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
    KX_BLEND_FACTOR_DST_COLOR = 4,
    KX_BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
    KX_BLEND_FACTOR_SRC_ALPHA = 6,
    KX_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
    KX_BLEND_FACTOR_DST_ALPHA = 8,
    KX_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
    KX_BLEND_FACTOR_CONSTANT_COLOR = 10,
    KX_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
    KX_BLEND_FACTOR_CONSTANT_ALPHA = 12,
    KX_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
    KX_BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
    KX_BLEND_FACTOR_SRC1_COLOR = 15,
    KX_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16,
    KX_BLEND_FACTOR_SRC1_ALPHA = 17,
    KX_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18,
    KX_BLEND_FACTOR_MAX_ENUM = 0x7FFFFFFF
} KxBlendFactor;

typedef enum KxBlendOp {
    KX_BLEND_OP_ADD = 0,
    KX_BLEND_OP_SUBTRACT = 1,
    KX_BLEND_OP_REVERSE_SUBTRACT = 2,
    KX_BLEND_OP_MIN = 3,
    KX_BLEND_OP_MAX = 4,
    KX_BLEND_OP_MAX_ENUM = 0x7FFFFFFF
} KxBlendOp;

typedef enum KxPipelineBindPoint {
    KX_PIPELINE_BIND_POINT_GRAPHICS = 0,
    KX_PIPELINE_BIND_POINT_COMPUTE = 1,
    KX_PIPELINE_BIND_POINT_MAX_ENUM = 0x7FFFFFFF
} KxPipelineBindPoint;

typedef enum KxCommandBufferLevel {
    KX_COMMAND_BUFFER_LEVEL_PRIMARY = 0,
    KX_COMMAND_BUFFER_LEVEL_SECONDARY = 1,
    KX_COMMAND_BUFFER_LEVEL_MAX_ENUM = 0x7FFFFFFF
} KxCommandBufferLevel;

/* =======================================================================
 * Opaque handles
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

/* =======================================================================
 * Properties / features / memory
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

typedef struct KxQueueFamilyProperties {
    uint32_t queueFlags;
    uint32_t queueCount;
    uint32_t timestampValidBits;
} KxQueueFamilyProperties;

typedef struct KxMemoryHeap {
    uint64_t size;
    uint32_t memoryTypeIndex;
} KxMemoryHeap;

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
 * Create info structures (each starts with sType / pNext)
 * ======================================================================= */

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

typedef struct KxMemoryAllocateInfo {
    KxStructureType sType;
    const void* pNext;
    uint64_t allocationSize;
    uint32_t memoryTypeIndex;
} KxMemoryAllocateInfo;

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

typedef enum KxImageViewType {
    KX_IMAGE_VIEW_TYPE_1D = 0,
    KX_IMAGE_VIEW_TYPE_2D = 1,
    KX_IMAGE_VIEW_TYPE_3D = 2,
    KX_IMAGE_VIEW_TYPE_1D_ARRAY = 3,
    KX_IMAGE_VIEW_TYPE_2D_ARRAY = 4,
    KX_IMAGE_VIEW_TYPE_CUBE = 5,
    KX_IMAGE_VIEW_TYPE_CUBE_ARRAY = 6,
    KX_IMAGE_VIEW_TYPE_MAX_ENUM = 0x7FFFFFFF
} KxImageViewType;

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
    KxBool32 sharingModeExclusive;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxImageLayout initialLayout;
} KxImageCreateInfo;

typedef struct KxImageViewCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxImageViewCreateFlags flags;
    KxImage image;
    KxImageViewType viewType;
    KxFormat format;
    KxImageAspectFlags aspectMask;
    uint32_t baseMipLevel;
    uint32_t levelCount;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
} KxImageViewCreateInfo;

typedef struct KxShaderModuleCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxShaderModuleCreateFlags flags;
    KxShaderFormat format;
    uint32_t codeSize;
    const void* pCode;
} KxShaderModuleCreateInfo;

typedef struct KxPipelineLayoutCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineLayoutCreateFlags flags;
    uint32_t setLayoutCount;
    const void* pSetLayouts;
    uint32_t pushConstantRangeCount;
    const void* pPushConstantRanges;
} KxPipelineLayoutCreateInfo;

/* Blend state per attachment */
typedef struct KxPipelineColorBlendAttachmentState {
    KxBool32 blendEnable;
    KxBlendFactor srcColorBlendFactor;
    KxBlendFactor dstColorBlendFactor;
    KxBlendOp colorBlendOp;
    KxBlendFactor srcAlphaBlendFactor;
    KxBlendFactor dstAlphaBlendFactor;
    KxBlendOp alphaBlendOp;
    KxColorComponentFlags colorWriteMask;
} KxPipelineColorBlendAttachmentState;

typedef struct KxPipelineColorBlendStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBool32 logicOpEnable;
    KxLogicOp logicOp;
    uint32_t attachmentCount;
    const KxPipelineColorBlendAttachmentState* pAttachments;
    float blendConstants[4];
} KxPipelineColorBlendStateCreateInfo;

typedef struct KxPipelineCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineCreateFlags flags;
    KxPipelineBindPoint bindPoint;
    KxShaderModule vertexShaderModule;
    KxShaderModule fragmentShaderModule;
    KxPrimitiveTopology primitiveTopology;
    KxPolygonMode polygonMode;
    KxFrontFace frontFace;
    KxBool32 depthTestEnable;
    KxBool32 depthWriteEnable;
    KxCompareOp depthCompareOp;
    const KxPipelineColorBlendStateCreateInfo* pColorBlendState;
    KxShaderModule computeShaderModule;
    KxPipelineLayout layout;
    KxPipeline basePipelineHandle;
    int32_t basePipelineIndex;
} KxPipelineCreateInfo;

typedef struct KxCommandPoolCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxCommandPoolCreateFlags flags;
    uint32_t queueFamilyIndex;
} KxCommandPoolCreateInfo;

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

typedef struct KxFenceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxFenceCreateFlags flags;
} KxFenceCreateInfo;

typedef struct KxSemaphoreCreateInfo {
    KxStructureType sType;
    const void* pNext;
} KxSemaphoreCreateInfo;

/* =======================================================================
 * Command buffer submission
 * ======================================================================= */

typedef struct KxSubmitInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t waitSemaphoreCount;
    const KxSemaphore* pWaitSemaphores;
    const uint32_t* pWaitDstStageMask;
    uint32_t commandBufferCount;
    const KxCommandBuffer* pCommandBuffers;
    uint32_t signalSemaphoreCount;
    const KxSemaphore* pSignalSemaphores;
} KxSubmitInfo;

/* =======================================================================
 * Core instance/device functions
 * ======================================================================= */

/* Instance */
KX_API KxResult KX_CALL kxCreateInstance(
    const KxInstanceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxInstance* pInstance);

KX_API void KX_CALL kxDestroyInstance(
    KxInstance instance,
    const KxAllocationCallbacks* pAllocator);

/* Physical device enumeration */
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

/* Device */
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

/* Memory */
KX_API KxResult KX_CALL kxAllocateMemory(
    KxDevice device,
    const KxMemoryAllocateInfo* pAllocateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDeviceMemory* pMemory);

KX_API void KX_CALL kxFreeMemory(
    KxDevice device,
    KxDeviceMemory memory,
    const KxAllocationCallbacks* pAllocator);

typedef uint32_t KxMemoryMapFlags;
#define KX_MEMORY_MAP_NONE 0u

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

/* Buffers */
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

/* Images */
KX_API KxResult KX_CALL kxCreateImage(
    KxDevice device,
    const KxImageCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImage* pImage);

KX_API void KX_CALL kxDestroyImage(
    KxDevice device,
    KxImage image,
    const KxAllocationCallbacks* pAllocator);

KX_API KxResult KX_CALL kxBindImageMemory(
    KxDevice device,
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memoryOffset);

/* Image views */
KX_API KxResult KX_CALL kxCreateImageView(
    KxDevice device,
    const KxImageViewCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxImageView* pImageView);

KX_API void KX_CALL kxDestroyImageView(
    KxDevice device,
    KxImageView imageView,
    const KxAllocationCallbacks* pAllocator);

/* Shader modules */
KX_API KxResult KX_CALL kxCreateShaderModule(
    KxDevice device,
    const KxShaderModuleCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule);

KX_API void KX_CALL kxDestroyShaderModule(
    KxDevice device,
    KxShaderModule shaderModule,
    const KxAllocationCallbacks* pAllocator);

/* Pipelines */
typedef struct KxPipelineCache_T* KxPipelineCache;

KX_API KxResult KX_CALL kxCreatePipelineLayout(
    KxDevice device,
    const KxPipelineLayoutCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipelineLayout* pPipelineLayout);

KX_API void KX_CALL kxDestroyPipelineLayout(
    KxDevice device,
    KxPipelineLayout pipelineLayout,
    const KxAllocationCallbacks* pAllocator);

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

/* Command buffers */
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

typedef uint32_t KxCommandBufferResetFlags;
#define KX_COMMAND_BUFFER_RESET_NONE 0u

KX_API KxResult KX_CALL kxResetCommandBuffer(
    KxCommandBuffer commandBuffer,
    KxCommandBufferResetFlags flags);

/* Fences / semaphores */
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

/* Queue submit */
KX_API KxResult KX_CALL kxQueueSubmit(
    KxQueue queue,
    uint32_t submitCount,
    const KxSubmitInfo* pSubmits,
    KxFence fence);

KX_API KxResult KX_CALL kxQueueWaitIdle(KxQueue queue);

/* =======================================================================
 * Commands recordable in a command buffer (vkCmd* style)
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

KX_API void KX_CALL kxCmdBindDescriptors(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const void* pDescriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* pDynamicOffsets);

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

KX_API void KX_CALL kxCmdDispatch(
    KxCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ);

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

KX_API void KX_CALL kxCmdClearImage(
    KxCommandBuffer commandBuffer,
    KxImage image,
    KxImageLayout imageLayout,
    const void* pColor,
    const void* pDepthStencil,
    uint32_t rangeCount,
    const void* pRanges);

typedef uint32_t KxDependencyFlags;
#define KX_DEPENDENCY_NONE 0u

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

/* Command pools */
KX_API KxResult KX_CALL kxCreateCommandPool(
    KxDevice device,
    const KxCommandPoolCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxCommandPool* pCommandPool);

KX_API void KX_CALL kxDestroyCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    const KxAllocationCallbacks* pAllocator);

typedef uint32_t KxCommandPoolResetFlags;
#define KX_COMMAND_POOL_RESET_NONE 0u

KX_API KxResult KX_CALL kxResetCommandPool(
    KxDevice device,
    KxCommandPool commandPool,
    KxCommandPoolResetFlags flags);

/* Device wait idle / proc addresses */
KX_API KxResult KX_CALL kxDeviceWaitIdle(KxDevice device);

typedef KxResult (KX_CALL *KxPfnVoidFunction)(void);

KX_API KxPfnVoidFunction KX_CALL kxGetDeviceProcAddr(
    KxDevice device,
    const char* pName);

KX_API KxPfnVoidFunction KX_CALL kxGetInstanceProcAddr(
    KxInstance instance,
    const char* pName);

KX_API const char* KX_CALL kxResultToString(KxResult result);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_CORE_H */
