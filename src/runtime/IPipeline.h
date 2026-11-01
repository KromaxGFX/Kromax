#ifndef KROMAX_SRC_RUNTIME_IPIPELINE_H
#define KROMAX_SRC_RUNTIME_IPIPELINE_H

/*
 * IPipeline.h
 *
 * Backend-agnostic pipeline interface.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class PipelineLayout;
class RenderPass;

enum class PipelineBindPoint : uint32_t {
    GRAPHICS = 0,
    COMPUTE  = 1,
    RAY_TRACING_KHR = 2,
};

struct PipelineShaderStageCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineShaderStageCreateFlags flags = 0;
    KxShaderStageFlagBits stage;
    class IShaderModule* module = nullptr;
    const char* pName = "main";
    const KxSpecializationInfo* pSpecializationInfo = nullptr;
};

struct PipelineVertexInputStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineVertexInputStateCreateFlags flags = 0;
    uint32_t vertexBindingDescriptionCount = 0;
    const KxVertexInputBindingDescription* pVertexBindingDescriptions = nullptr;
    uint32_t vertexAttributeDescriptionCount = 0;
    const KxVertexInputAttributeDescription* pVertexAttributeDescriptions = nullptr;
};

struct PipelineInputAssemblyStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineInputAssemblyStateCreateFlags flags = 0;
    KxPrimitiveTopology topology = KX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    KxBool32 primitiveRestartEnable = false;
};

struct PipelineTessellationStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineTessellationStateCreateFlags flags = 0;
    uint32_t patchControlPoints = 0;
};

struct PipelineViewportStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineViewportStateCreateFlags flags = 0;
    uint32_t viewportCount = 1;
    const KxViewport* pViewports = nullptr;
    uint32_t scissorCount = 1;
    const KxRect2D* pScissors = nullptr;
};

struct PipelineRasterizationStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineRasterizationStateCreateFlags flags = 0;
    KxBool32 depthClampEnable = false;
    KxBool32 rasterizerDiscardEnable = false;
    KxPolygonMode polygonMode = KX_POLYGON_MODE_FILL;
    KxCullModeFlags cullMode = KX_CULL_MODE_BACK_BIT;
    KxFrontFace frontFace = KX_FRONT_FACE_COUNTER_CLOCKWISE;
    KxBool32 depthBiasEnable = false;
    float depthBiasConstantFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
    float lineWidth = 1.0f;
};

struct PipelineMultisampleStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineMultisampleStateCreateFlags flags = 0;
    KxSampleCountFlagBits rasterizationSamples = KX_SAMPLE_COUNT_1_BIT;
    KxBool32 sampleShadingEnable = false;
    float minSampleShading = 1.0f;
    const KxSampleMask* pSampleMask = nullptr;
    KxBool32 alphaToCoverageEnable = false;
    KxBool32 alphaToOneEnable = false;
};

struct PipelineDepthStencilStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineDepthStencilStateCreateFlags flags = 0;
    KxBool32 depthTestEnable = true;
    KxBool32 depthWriteEnable = true;
    KxCompareOp depthCompareOp = KX_COMPARE_OP_LESS;
    KxBool32 depthBoundsTestEnable = false;
    KxBool32 stencilTestEnable = false;
    KxStencilOpState front{};
    KxStencilOpState back{};
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
};

struct PipelineColorBlendAttachmentState {
    KxBool32 blendEnable = false;
    KxBlendFactor srcColorBlendFactor = KX_BLEND_FACTOR_ONE;
    KxBlendFactor dstColorBlendFactor = KX_BLEND_FACTOR_ZERO;
    KxBlendOp colorBlendOp = KX_BLEND_OP_ADD;
    KxBlendFactor srcAlphaBlendFactor = KX_BLEND_FACTOR_ONE;
    KxBlendFactor dstAlphaBlendFactor = KX_BLEND_FACTOR_ZERO;
    KxBlendOp alphaBlendOp = KX_BLEND_OP_ADD;
    KxColorComponentFlags colorWriteMask = KX_COLOR_COMPONENT_R_BIT | KX_COLOR_COMPONENT_G_BIT | KX_COLOR_COMPONENT_B_BIT | KX_COLOR_COMPONENT_A_BIT;
};

struct PipelineColorBlendStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineColorBlendStateCreateFlags flags = 0;
    KxBool32 logicOpEnable = false;
    KxLogicOp logicOp = KX_LOGIC_OP_COPY;
    uint32_t attachmentCount = 0;
    const PipelineColorBlendAttachmentState* pAttachments = nullptr;
    float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct PipelineDynamicStateCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineDynamicStateCreateFlags flags = 0;
    uint32_t dynamicStateCount = 0;
    const KxDynamicState* pDynamicStates = nullptr;
};

class IPipeline {
public:
    virtual ~IPipeline() = default;

    // Pipeline type
    virtual PipelineBindPoint getBindPoint() const noexcept = 0;

    // Layout
    virtual PipelineLayout getLayout() const noexcept = 0;

    // For graphics pipelines
    virtual RenderPass getRenderPass() const noexcept = 0;
    virtual uint32_t getSubpass() const noexcept = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IPIPELINE_H