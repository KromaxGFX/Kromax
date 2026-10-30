#ifndef KX_PIPELINE_H
#define KX_PIPELINE_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"
#include "kx_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Pipeline create flags
 * ======================================================================= */

typedef uint32_t KxPipelineCreateFlags;
#define KX_PIPELINE_CREATE_NONE 0u
#define KX_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT 0x00000001u
#define KX_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT 0x00000002u
#define KX_PIPELINE_CREATE_DERIVATIVE_BIT 0x00000004u
#define KX_PIPELINE_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef uint32_t KxPipelineLayoutCreateFlags;
#define KX_PIPELINE_LAYOUT_CREATE_NONE 0u
#define KX_PIPELINE_LAYOUT_CREATE_MAX_ENUM 0x7FFFFFFFu

/* =======================================================================
 * Shader module
 * ======================================================================= */

typedef uint32_t KxShaderModuleCreateFlags;
#define KX_SHADER_MODULE_CREATE_NONE 0u
#define KX_SHADER_MODULE_CREATE_MAX_ENUM 0x7FFFFFFFu

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
    uint32_t boundTextureCount; /* number of bound sampled textures */
    const KxBoundTexture* pBoundTextures; /* resolved sampled textures */
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

typedef struct KxShaderModuleCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxShaderModuleCreateFlags flags;
    KxShaderFormat format;
    uint32_t codeSize;
    const void* pCode;
} KxShaderModuleCreateInfo;

/* =======================================================================
 * Pipeline layout
 * ======================================================================= */

typedef struct KxPushConstantRange {
    KxShaderStageFlags stageFlags;
    uint32_t offset;
    uint32_t size;
} KxPushConstantRange;

typedef struct KxPipelineLayoutCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineLayoutCreateFlags flags;
    uint32_t setLayoutCount;
    const void* pSetLayouts;
    uint32_t pushConstantRangeCount;
    const void* pPushConstantRanges;
} KxPipelineLayoutCreateInfo;

/* =======================================================================
 * Pipeline state structures
 * ======================================================================= */

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

/* Vertex input state */
typedef struct KxVertexInputBindingDescription {
    uint32_t binding;
    uint32_t stride;
    uint32_t inputRate; /* KxVertexInputRate */
} KxVertexInputBindingDescription;

typedef struct KxVertexInputAttributeDescription {
    uint32_t location;
    uint32_t binding;
    KxFormat format;
    uint32_t offset;
} KxVertexInputAttributeDescription;

typedef struct KxPipelineVertexInputStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    uint32_t vertexBindingDescriptionCount;
    const KxVertexInputBindingDescription* pVertexBindingDescriptions;
    uint32_t vertexAttributeDescriptionCount;
    const KxVertexInputAttributeDescription* pVertexAttributeDescriptions;
} KxPipelineVertexInputStateCreateInfo;

/* Viewport state */
typedef struct KxViewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
} KxViewport;

typedef struct KxPipelineViewportStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    uint32_t viewportCount;
    const KxViewport* pViewports;
    uint32_t scissorCount;
    const KxRect2D* pScissors;
} KxPipelineViewportStateCreateInfo;

/* Rasterization state */
typedef struct KxPipelineRasterizationStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBool32 depthClampEnable;
    KxBool32 rasterizerDiscardEnable;
    KxPolygonMode polygonMode;
    KxCullModeFlags cullMode;
    KxFrontFace frontFace;
    KxBool32 depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
} KxPipelineRasterizationStateCreateInfo;

/* Multisample state */
typedef struct KxPipelineMultisampleStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBool32 sampleShadingEnable;
    float minSampleShading;
    const KxSampleCountFlags* pSampleMask;
    KxBool32 alphaToCoverageEnable;
    KxBool32 alphaToOneEnable;
} KxPipelineMultisampleStateCreateInfo;

/* Depth/stencil state */
typedef struct KxStencilOpState {
    KxStencilOp failOp;
    KxStencilOp passOp;
    KxStencilOp depthFailOp;
    KxCompareOp compareOp;
    uint32_t compareMask;
    uint32_t writeMask;
    uint32_t reference;
} KxStencilOpState;

typedef struct KxPipelineDepthStencilStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxBool32 depthTestEnable;
    KxBool32 depthWriteEnable;
    KxCompareOp depthCompareOp;
    KxBool32 depthBoundsTestEnable;
    KxBool32 stencilTestEnable;
    KxStencilOpState front;
    KxStencilOpState back;
    float minDepthBounds;
    float maxDepthBounds;
} KxPipelineDepthStencilStateCreateInfo;

/* Dynamic state */
typedef enum KxDynamicState {
    KX_DYNAMIC_STATE_VIEWPORT = 0,
    KX_DYNAMIC_STATE_SCISSOR = 1,
    KX_DYNAMIC_STATE_LINE_WIDTH = 2,
    KX_DYNAMIC_STATE_DEPTH_BIAS = 3,
    KX_DYNAMIC_STATE_BLEND_CONSTANTS = 4,
    KX_DYNAMIC_STATE_DEPTH_BOUNDS = 5,
    KX_DYNAMIC_STATE_STENCIL_COMPARE_MASK = 6,
    KX_DYNAMIC_STATE_STENCIL_WRITE_MASK = 7,
    KX_DYNAMIC_STATE_STENCIL_REFERENCE = 8,
    KX_DYNAMIC_STATE_MAX_ENUM = 0x7FFFFFFF
} KxDynamicState;

typedef struct KxPipelineDynamicStateCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    uint32_t dynamicStateCount;
    const KxDynamicState* pDynamicStates;
} KxPipelineDynamicStateCreateInfo;

/* =======================================================================
 * Pipeline create info
 * ======================================================================= */

typedef struct KxPipelineCache_T* KxPipelineCache;

typedef struct KxPipelineCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineCreateFlags flags;
    KxPipelineBindPoint bindPoint;
    KxPipelineLayout layout;
    KxShaderModule vertexShaderModule;
    KxShaderModule fragmentShaderModule;
    KxShaderModule computeShaderModule;
    const KxPipelineVertexInputStateCreateInfo* pVertexInputState;
    const KxPipelineViewportStateCreateInfo* pViewportState;
    const KxPipelineRasterizationStateCreateInfo* pRasterizationState;
    const KxPipelineMultisampleStateCreateInfo* pMultisampleState;
    const KxPipelineDepthStencilStateCreateInfo* pDepthStencilState;
    const KxPipelineColorBlendStateCreateInfo* pColorBlendState;
    const KxPipelineDynamicStateCreateInfo* pDynamicState;
    KxPrimitiveTopology primitiveTopology;
    KxRenderPass renderPass;
    uint32_t subpass;
    KxPipeline basePipelineHandle;
    int32_t basePipelineIndex;
} KxPipelineCreateInfo;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_PIPELINE_H */