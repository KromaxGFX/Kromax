/*
 * VkPipeline.cpp
 *
 * Vulkan pipeline and pipeline layout implementation.
 */

#include "VkPipeline.h"

#include <vector>
#include <cstring>

namespace kx {

// ============================================================================
// Helper functions for Kx -> Vulkan enum conversion
// ============================================================================

static inline VkShaderStageFlagBits kx_shader_stage_to_vk(KxShaderStageFlags stage) {
    return static_cast<VkShaderStageFlagBits>(stage);
}

static inline VkPrimitiveTopology kx_primitive_topology_to_vk(KxPrimitiveTopology topo) {
    return static_cast<VkPrimitiveTopology>(topo);
}

static inline VkPolygonMode kx_polygon_mode_to_vk(KxPolygonMode mode) {
    return static_cast<VkPolygonMode>(mode);
}

static inline VkCullModeFlags kx_cull_mode_to_vk(KxCullModeFlags mode) {
    return static_cast<VkCullModeFlags>(mode);
}

static inline VkFrontFace kx_front_face_to_vk(KxFrontFace face) {
    return static_cast<VkFrontFace>(face);
}

static inline VkLogicOp kx_logic_op_to_vk(KxLogicOp op) {
    return static_cast<VkLogicOp>(op);
}

static inline VkCompareOp kx_compare_op_to_vk(KxCompareOp op) {
    return static_cast<VkCompareOp>(op);
}

static inline VkStencilOp kx_stencil_op_to_vk(KxStencilOp op) {
    return static_cast<VkStencilOp>(op);
}

static inline VkBlendFactor kx_blend_factor_to_vk(KxBlendFactor factor) {
    return static_cast<VkBlendFactor>(factor);
}

static inline VkBlendOp kx_blend_op_to_vk(KxBlendOp op) {
    return static_cast<VkBlendOp>(op);
}

static inline VkColorComponentFlags kx_color_component_flags_to_vk(KxColorComponentFlags flags) {
    return static_cast<VkColorComponentFlags>(flags);
}

static inline VkDynamicState kx_dynamic_state_to_vk(KxDynamicState state) {
    return static_cast<VkDynamicState>(state);
}

static inline VkVertexInputRate kx_vertex_input_rate_to_vk(uint32_t rate) {
    return static_cast<VkVertexInputRate>(rate);
}

static inline VkFormat kx_format_to_vk(KxFormat fmt) {
    return static_cast<VkFormat>(fmt);
}

static inline VkSampleCountFlagBits kx_sample_count_to_vk(KxSampleCountFlags flags) {
    return static_cast<VkSampleCountFlagBits>(flags);
}

// ============================================================================
// VkPipelineLayoutImpl
// ============================================================================

VkPipelineLayoutImpl::~VkPipelineLayoutImpl() {
    if (layout_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkPipelineLayoutImpl::VkPipelineLayoutImpl(VkPipelineLayoutImpl&& other) noexcept
    : layout_(other.layout_)
    , device_(other.device_)
    , set_layouts_(std::move(other.set_layouts_))
    , push_constants_(std::move(other.push_constants_)) {
    other.layout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
}

VkPipelineLayoutImpl& VkPipelineLayoutImpl::operator=(VkPipelineLayoutImpl&& other) noexcept {
    if (this != &other) {
        layout_ = other.layout_;
        device_ = other.device_;
        set_layouts_ = std::move(other.set_layouts_);
        push_constants_ = std::move(other.push_constants_);
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }
    return *this;
}

bool VkPipelineLayoutImpl::initialize(VkFunctionTable& functions,
                                      VkDevice device,
                                      const KxPipelineLayoutCreateInfo* create_info) {
    device_ = device;

    // Convert KxDescriptorSetLayout to VkDescriptorSetLayout
    set_layouts_.reserve(create_info->setLayoutCount);
    const VkDescriptorSetLayoutImpl* const* set_layouts = reinterpret_cast<const VkDescriptorSetLayoutImpl* const*>(create_info->pSetLayouts);
    for (uint32_t i = 0; i < create_info->setLayoutCount; ++i) {
        auto* layout_impl = set_layouts[i];
        if (layout_impl) {
            set_layouts_.push_back(layout_impl->handle());
        }
    }

    // Convert push constant ranges
    push_constants_.reserve(create_info->pushConstantRangeCount);
    const KxPushConstantRange* push_ranges = reinterpret_cast<const KxPushConstantRange*>(create_info->pPushConstantRanges);
    for (uint32_t i = 0; i < create_info->pushConstantRangeCount; ++i) {
        const KxPushConstantRange& kx_range = push_ranges[i];
        VkPushConstantRange vk_range{};
        vk_range.stageFlags = static_cast<VkShaderStageFlags>(kx_range.stageFlags);
        vk_range.offset = kx_range.offset;
        vk_range.size = kx_range.size;
        push_constants_.push_back(vk_range);
    }

    VkPipelineLayoutCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineLayoutCreateFlags>(create_info->flags);
    vk_info.setLayoutCount = static_cast<uint32_t>(set_layouts_.size());
    vk_info.pSetLayouts = set_layouts_.data();
    vk_info.pushConstantRangeCount = static_cast<uint32_t>(push_constants_.size());
    vk_info.pPushConstantRanges = push_constants_.data();

    VkResult result = functions.vkCreatePipelineLayout(device, &vk_info, nullptr, &layout_);
    return result == VK_SUCCESS;
}

// ============================================================================
// VkPipelineImpl - Shader Stage Helper
// ============================================================================

static VkPipelineShaderStageCreateInfo create_shader_stage(VkFunctionTable& functions,
                                                           VkShaderStageFlagBits stage,
                                                           KxShaderModule shader_module,
                                                           const char* entry_point) {
    VkPipelineShaderStageCreateInfo vk_stage{};
    vk_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vk_stage.stage = stage;
    
    if (shader_module) {
        auto* module_impl = reinterpret_cast<VkShaderModuleImpl*>(shader_module);
        if (module_impl) {
            vk_stage.module = module_impl->handle();
        }
    }
    
    vk_stage.pName = entry_point ? entry_point : "main";
    vk_stage.pSpecializationInfo = nullptr;
    
    return vk_stage;
}

// ============================================================================
// VkPipelineImpl
// ============================================================================

VkPipelineImpl::~VkPipelineImpl() {
    if (pipeline_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Destruction handled by VkDriverDevice
    }
}

VkPipelineImpl::VkPipelineImpl(VkPipelineImpl&& other) noexcept
    : pipeline_(other.pipeline_)
    , layout_(other.layout_)
    , device_(other.device_)
    , is_compute_(other.is_compute_)
    , shader_stages_(std::move(other.shader_stages_))
    , vertex_input_state_(other.vertex_input_state_)
    , input_assembly_state_(other.input_assembly_state_)
    , tessellation_state_(other.tessellation_state_)
    , viewport_state_(other.viewport_state_)
    , rasterization_state_(other.rasterization_state_)
    , multisample_state_(other.multisample_state_)
    , depth_stencil_state_(other.depth_stencil_state_)
    , color_blend_state_(other.color_blend_state_)
    , dynamic_state_(other.dynamic_state_)
    , dynamic_states_(std::move(other.dynamic_states_))
    , viewports_(std::move(other.viewports_))
    , scissors_(std::move(other.scissors_))
    , color_blend_attachments_(std::move(other.color_blend_attachments_))
    , vertex_bindings_(std::move(other.vertex_bindings_))
    , vertex_attributes_(std::move(other.vertex_attributes_)) {
    other.pipeline_ = VK_NULL_HANDLE;
    other.layout_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.is_compute_ = false;
}

VkPipelineImpl& VkPipelineImpl::operator=(VkPipelineImpl&& other) noexcept {
    if (this != &other) {
        pipeline_ = other.pipeline_;
        layout_ = other.layout_;
        device_ = other.device_;
        is_compute_ = other.is_compute_;
        shader_stages_ = std::move(other.shader_stages_);
        vertex_input_state_ = other.vertex_input_state_;
        input_assembly_state_ = other.input_assembly_state_;
        tessellation_state_ = other.tessellation_state_;
        viewport_state_ = other.viewport_state_;
        rasterization_state_ = other.rasterization_state_;
        multisample_state_ = other.multisample_state_;
        depth_stencil_state_ = other.depth_stencil_state_;
        color_blend_state_ = other.color_blend_state_;
        dynamic_state_ = other.dynamic_state_;
        dynamic_states_ = std::move(other.dynamic_states_);
        viewports_ = std::move(other.viewports_);
        scissors_ = std::move(other.scissors_);
        color_blend_attachments_ = std::move(other.color_blend_attachments_);
        vertex_bindings_ = std::move(other.vertex_bindings_);
        vertex_attributes_ = std::move(other.vertex_attributes_);
        other.pipeline_ = VK_NULL_HANDLE;
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.is_compute_ = false;
    }
    return *this;
}

// ============================================================================
// Graphics Pipeline Initialization
// ============================================================================

bool VkPipelineImpl::initialize_graphics(VkFunctionTable& functions,
                                         VkDevice device,
                                         VkPipelineCache cache,
                                         const KxPipelineCreateInfo* create_info) {
    device_ = device;
    is_compute_ = false;

    // Get pipeline layout
    auto* layout_impl = reinterpret_cast<VkPipelineLayoutImpl*>(create_info->layout);
    if (!layout_impl) {
        return false;
    }
    layout_ = layout_impl->handle();

    // Clear previous state
    shader_stages_.clear();
    dynamic_states_.clear();
    viewports_.clear();
    scissors_.clear();
    color_blend_attachments_.clear();
    vertex_bindings_.clear();
    vertex_attributes_.clear();

    // ========================================================================
    // 1. SHADER STAGES - Kx API has individual fields per stage
    // ========================================================================
    if (create_info->vertexShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_VERTEX_BIT, 
                                                     create_info->vertexShaderModule, "main"));
    }
    if (create_info->tessellationControlShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                                     create_info->tessellationControlShaderModule, "main"));
    }
    if (create_info->tessellationEvaluationShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                                     create_info->tessellationEvaluationShaderModule, "main"));
    }
    if (create_info->geometryShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_GEOMETRY_BIT,
                                                     create_info->geometryShaderModule, "main"));
    }
    if (create_info->fragmentShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                     create_info->fragmentShaderModule, "main"));
    }
    if (create_info->taskShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_TASK_BIT_EXT,
                                                     create_info->taskShaderModule, "main"));
    }
    if (create_info->meshShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_MESH_BIT_EXT,
                                                     create_info->meshShaderModule, "main"));
    }

    // Ray tracing stages (would need VK_KHR_ray_tracing_pipeline)
    // if (create_info->rayGenShaderModule) { ... }
    // if (create_info->anyHitShaderModule) { ... }
    // if (create_info->closestHitShaderModule) { ... }
    // if (create_info->missShaderModule) { ... }
    // if (create_info->intersectionShaderModule) { ... }
    // if (create_info->callableShaderModule) { ... }

    // ========================================================================
    // 2. VERTEX INPUT STATE
    // ========================================================================
    if (create_info->pVertexInputState) {
        const KxPipelineVertexInputStateCreateInfo* kx_vi = create_info->pVertexInputState;
        
        const KxVertexInputBindingDescription* kx_bindings = kx_vi->pVertexBindingDescriptions;
        vertex_bindings_.reserve(kx_vi->vertexBindingDescriptionCount);
        for (uint32_t i = 0; i < kx_vi->vertexBindingDescriptionCount; ++i) {
            VkVertexInputBindingDescription vk_binding{};
            vk_binding.binding = kx_bindings[i].binding;
            vk_binding.stride = kx_bindings[i].stride;
            vk_binding.inputRate = kx_vertex_input_rate_to_vk(kx_bindings[i].inputRate);
            vertex_bindings_.push_back(vk_binding);
        }

        const KxVertexInputAttributeDescription* kx_attrs = kx_vi->pVertexAttributeDescriptions;
        vertex_attributes_.reserve(kx_vi->vertexAttributeDescriptionCount);
        for (uint32_t i = 0; i < kx_vi->vertexAttributeDescriptionCount; ++i) {
            VkVertexInputAttributeDescription vk_attr{};
            vk_attr.location = kx_attrs[i].location;
            vk_attr.binding = kx_attrs[i].binding;
            vk_attr.format = kx_format_to_vk(kx_attrs[i].format);
            vk_attr.offset = kx_attrs[i].offset;
            vertex_attributes_.push_back(vk_attr);
        }

        vertex_input_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_.flags = static_cast<VkPipelineVertexInputStateCreateFlags>(kx_vi->flags);
        vertex_input_state_.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_bindings_.size());
        vertex_input_state_.pVertexBindingDescriptions = vertex_bindings_.data();
        vertex_input_state_.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attributes_.size());
        vertex_input_state_.pVertexAttributeDescriptions = vertex_attributes_.data();
    } else {
        vertex_input_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_.vertexBindingDescriptionCount = 0;
        vertex_input_state_.vertexAttributeDescriptionCount = 0;
    }

    // ========================================================================
    // 3. INPUT ASSEMBLY STATE
    // ========================================================================
    input_assembly_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_.topology = kx_primitive_topology_to_vk(create_info->primitiveTopology);
    input_assembly_state_.primitiveRestartEnable = VK_FALSE;

    // ========================================================================
    // 4. TESSELLATION STATE
    // ========================================================================
    if (create_info->pTessellationState) {
        const KxPipelineTessellationStateCreateInfo* kx_ts = create_info->pTessellationState;
        tessellation_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellation_state_.flags = static_cast<VkPipelineTessellationStateCreateFlags>(kx_ts->flags);
        tessellation_state_.patchControlPoints = kx_ts->patchControlPoints;
    }

    // ========================================================================
    // 5. VIEWPORT STATE
    // ========================================================================
    if (create_info->pViewportState) {
        const KxPipelineViewportStateCreateInfo* kx_vs = create_info->pViewportState;
        
        const KxViewport* kx_viewports = kx_vs->pViewports;
        viewports_.reserve(kx_vs->viewportCount);
        for (uint32_t i = 0; i < kx_vs->viewportCount; ++i) {
            VkViewport vp{};
            vp.x = kx_viewports[i].x;
            vp.y = kx_viewports[i].y;
            vp.width = kx_viewports[i].width;
            vp.height = kx_viewports[i].height;
            vp.minDepth = kx_viewports[i].minDepth;
            vp.maxDepth = kx_viewports[i].maxDepth;
            viewports_.push_back(vp);
        }

        const KxRect2D* kx_scissors = kx_vs->pScissors;
        scissors_.reserve(kx_vs->scissorCount);
        for (uint32_t i = 0; i < kx_vs->scissorCount; ++i) {
            VkRect2D sc{};
            sc.offset.x = kx_scissors[i].offset.x;
            sc.offset.y = kx_scissors[i].offset.y;
            sc.extent.width = kx_scissors[i].extent.width;
            sc.extent.height = kx_scissors[i].extent.height;
            scissors_.push_back(sc);
        }

        viewport_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_.flags = static_cast<VkPipelineViewportStateCreateFlags>(kx_vs->flags);
        viewport_state_.viewportCount = static_cast<uint32_t>(viewports_.size());
        viewport_state_.pViewports = viewports_.data();
        viewport_state_.scissorCount = static_cast<uint32_t>(scissors_.size());
        viewport_state_.pScissors = scissors_.data();
    } else {
        viewport_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_.viewportCount = 1;
        viewport_state_.scissorCount = 1;
    }

    // ========================================================================
    // 6. RASTERIZATION STATE
    // ========================================================================
    if (create_info->pRasterizationState) {
        const KxPipelineRasterizationStateCreateInfo* kx_rs = create_info->pRasterizationState;
        rasterization_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        // Note: KxPipelineRasterizationStateCreateInfo doesn't have flags field
        rasterization_state_.flags = 0;
        rasterization_state_.depthClampEnable = kx_rs->depthClampEnable;
        rasterization_state_.rasterizerDiscardEnable = kx_rs->rasterizerDiscardEnable;
        rasterization_state_.polygonMode = kx_polygon_mode_to_vk(kx_rs->polygonMode);
        rasterization_state_.cullMode = kx_cull_mode_to_vk(kx_rs->cullMode);
        rasterization_state_.frontFace = kx_front_face_to_vk(kx_rs->frontFace);
        rasterization_state_.depthBiasEnable = kx_rs->depthBiasEnable;
        rasterization_state_.depthBiasConstantFactor = kx_rs->depthBiasConstantFactor;
        rasterization_state_.depthBiasClamp = kx_rs->depthBiasClamp;
        rasterization_state_.depthBiasSlopeFactor = kx_rs->depthBiasSlopeFactor;
        rasterization_state_.lineWidth = kx_rs->lineWidth;
    } else {
        rasterization_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state_.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_.lineWidth = 1.0f;
    }

    // ========================================================================
    // 7. MULTISAMPLE STATE
    // ========================================================================
    if (create_info->pMultisampleState) {
        const KxPipelineMultisampleStateCreateInfo* kx_ms = create_info->pMultisampleState;
        multisample_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        // KxPipelineMultisampleStateCreateInfo doesn't have flags or rasterizationSamples
        // Default to single sample
        multisample_state_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_state_.sampleShadingEnable = kx_ms->sampleShadingEnable;
        multisample_state_.minSampleShading = kx_ms->minSampleShading;
        multisample_state_.pSampleMask = reinterpret_cast<const VkSampleMask*>(kx_ms->pSampleMask);
        multisample_state_.alphaToCoverageEnable = kx_ms->alphaToCoverageEnable;
        multisample_state_.alphaToOneEnable = kx_ms->alphaToOneEnable;
    } else {
        multisample_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    // ========================================================================
    // 8. DEPTH STENCIL STATE
    // ========================================================================
    if (create_info->pDepthStencilState) {
        const KxPipelineDepthStencilStateCreateInfo* kx_ds = create_info->pDepthStencilState;
        depth_stencil_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        // KxPipelineDepthStencilStateCreateInfo doesn't have flags field
        depth_stencil_state_.flags = 0;
        depth_stencil_state_.depthTestEnable = kx_ds->depthTestEnable;
        depth_stencil_state_.depthWriteEnable = kx_ds->depthWriteEnable;
        depth_stencil_state_.depthCompareOp = kx_compare_op_to_vk(kx_ds->depthCompareOp);
        depth_stencil_state_.depthBoundsTestEnable = kx_ds->depthBoundsTestEnable;
        depth_stencil_state_.stencilTestEnable = kx_ds->stencilTestEnable;
        
        // Front stencil
        depth_stencil_state_.front.failOp = kx_stencil_op_to_vk(kx_ds->front.failOp);
        depth_stencil_state_.front.passOp = kx_stencil_op_to_vk(kx_ds->front.passOp);
        depth_stencil_state_.front.depthFailOp = kx_stencil_op_to_vk(kx_ds->front.depthFailOp);
        depth_stencil_state_.front.compareOp = kx_compare_op_to_vk(kx_ds->front.compareOp);
        depth_stencil_state_.front.compareMask = kx_ds->front.compareMask;
        depth_stencil_state_.front.writeMask = kx_ds->front.writeMask;
        depth_stencil_state_.front.reference = kx_ds->front.reference;
        
        // Back stencil
        depth_stencil_state_.back.failOp = kx_stencil_op_to_vk(kx_ds->back.failOp);
        depth_stencil_state_.back.passOp = kx_stencil_op_to_vk(kx_ds->back.passOp);
        depth_stencil_state_.back.depthFailOp = kx_stencil_op_to_vk(kx_ds->back.depthFailOp);
        depth_stencil_state_.back.compareOp = kx_compare_op_to_vk(kx_ds->back.compareOp);
        depth_stencil_state_.back.compareMask = kx_ds->back.compareMask;
        depth_stencil_state_.back.writeMask = kx_ds->back.writeMask;
        depth_stencil_state_.back.reference = kx_ds->back.reference;
        
        depth_stencil_state_.minDepthBounds = kx_ds->minDepthBounds;
        depth_stencil_state_.maxDepthBounds = kx_ds->maxDepthBounds;
    } else {
        depth_stencil_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state_.depthTestEnable = VK_TRUE;
        depth_stencil_state_.depthWriteEnable = VK_TRUE;
        depth_stencil_state_.depthCompareOp = VK_COMPARE_OP_LESS;
    }

    // ========================================================================
    // 9. COLOR BLEND STATE
    // ========================================================================
    if (create_info->pColorBlendState) {
        const KxPipelineColorBlendStateCreateInfo* kx_cb = create_info->pColorBlendState;
        
        const KxPipelineColorBlendAttachmentState* kx_attachments = kx_cb->pAttachments;
        color_blend_attachments_.reserve(kx_cb->attachmentCount);
        for (uint32_t i = 0; i < kx_cb->attachmentCount; ++i) {
            VkPipelineColorBlendAttachmentState vk_att{};
            vk_att.blendEnable = kx_attachments[i].blendEnable;
            vk_att.srcColorBlendFactor = kx_blend_factor_to_vk(kx_attachments[i].srcColorBlendFactor);
            vk_att.dstColorBlendFactor = kx_blend_factor_to_vk(kx_attachments[i].dstColorBlendFactor);
            vk_att.colorBlendOp = kx_blend_op_to_vk(kx_attachments[i].colorBlendOp);
            vk_att.srcAlphaBlendFactor = kx_blend_factor_to_vk(kx_attachments[i].srcAlphaBlendFactor);
            vk_att.dstAlphaBlendFactor = kx_blend_factor_to_vk(kx_attachments[i].dstAlphaBlendFactor);
            vk_att.alphaBlendOp = kx_blend_op_to_vk(kx_attachments[i].alphaBlendOp);
            vk_att.colorWriteMask = kx_color_component_flags_to_vk(kx_attachments[i].colorWriteMask);
            color_blend_attachments_.push_back(vk_att);
        }

        color_blend_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        // KxPipelineColorBlendStateCreateInfo doesn't have flags field
        color_blend_state_.flags = 0;
        color_blend_state_.logicOpEnable = kx_cb->logicOpEnable;
        color_blend_state_.logicOp = kx_logic_op_to_vk(kx_cb->logicOp);
        color_blend_state_.attachmentCount = static_cast<uint32_t>(color_blend_attachments_.size());
        color_blend_state_.pAttachments = color_blend_attachments_.data();
        std::memcpy(color_blend_state_.blendConstants, kx_cb->blendConstants, sizeof(float) * 4);
    } else {
        color_blend_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_.attachmentCount = 0;
    }

    // ========================================================================
    // 10. DYNAMIC STATE
    // ========================================================================
    if (create_info->pDynamicState) {
        const KxPipelineDynamicStateCreateInfo* kx_dyn = create_info->pDynamicState;
        const KxDynamicState* kx_dyn_states = kx_dyn->pDynamicStates;
        dynamic_states_.reserve(kx_dyn->dynamicStateCount);
        for (uint32_t i = 0; i < kx_dyn->dynamicStateCount; ++i) {
            dynamic_states_.push_back(kx_dynamic_state_to_vk(kx_dyn_states[i]));
        }

        dynamic_state_.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_.flags = static_cast<VkPipelineDynamicStateCreateFlags>(kx_dyn->flags);
        dynamic_state_.dynamicStateCount = static_cast<uint32_t>(dynamic_states_.size());
        dynamic_state_.pDynamicStates = dynamic_states_.data();
    }

    // ========================================================================
    // CREATE GRAPHICS PIPELINE
    // ========================================================================
    VkGraphicsPipelineCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineCreateFlags>(create_info->flags);
    vk_info.stageCount = static_cast<uint32_t>(shader_stages_.size());
    vk_info.pStages = shader_stages_.data();
    vk_info.pVertexInputState = &vertex_input_state_;
    vk_info.pInputAssemblyState = &input_assembly_state_;
    vk_info.pTessellationState = create_info->pTessellationState ? &tessellation_state_ : nullptr;
    vk_info.pViewportState = &viewport_state_;
    vk_info.pRasterizationState = &rasterization_state_;
    vk_info.pMultisampleState = &multisample_state_;
    vk_info.pDepthStencilState = &depth_stencil_state_;
    vk_info.pColorBlendState = &color_blend_state_;
    vk_info.pDynamicState = create_info->pDynamicState ? &dynamic_state_ : nullptr;
    vk_info.layout = layout_;
    vk_info.renderPass = reinterpret_cast<VkRenderPass>(create_info->renderPass);
    vk_info.subpass = create_info->subpass;
    vk_info.basePipelineHandle = reinterpret_cast<VkPipeline>(create_info->basePipelineHandle);
    vk_info.basePipelineIndex = create_info->basePipelineIndex;

    VkResult result = functions.vkCreateGraphicsPipelines(device, cache, 1, &vk_info, nullptr, &pipeline_);
    return result == VK_SUCCESS;
}

bool VkPipelineImpl::initialize_compute(VkFunctionTable& functions,
                                        VkDevice device,
                                        VkPipelineCache cache,
                                        const KxPipelineCreateInfo* create_info) {
    device_ = device;
    is_compute_ = true;

    // Get pipeline layout
    auto* layout_impl = reinterpret_cast<VkPipelineLayoutImpl*>(create_info->layout);
    if (!layout_impl) {
        return false;
    }
    layout_ = layout_impl->handle();

    // Compute pipeline uses computeShaderModule field
    shader_stages_.clear();
    
    if (create_info->computeShaderModule) {
        shader_stages_.push_back(create_shader_stage(functions, VK_SHADER_STAGE_COMPUTE_BIT,
                                                     create_info->computeShaderModule, "main"));
    } else {
        return false; // Compute pipeline requires compute shader
    }

    VkComputePipelineCreateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    vk_info.flags = static_cast<VkPipelineCreateFlags>(create_info->flags);
    vk_info.stage = shader_stages_[0];
    vk_info.layout = layout_;
    vk_info.basePipelineHandle = reinterpret_cast<VkPipeline>(create_info->basePipelineHandle);
    vk_info.basePipelineIndex = create_info->basePipelineIndex;

    VkResult result = functions.vkCreateComputePipelines(device, cache, 1, &vk_info, nullptr, &pipeline_);
    return result == VK_SUCCESS;
}

} // namespace kx