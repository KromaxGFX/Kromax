#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H

/*
 * SoftwarePipeline.h
 *
 * Software pipeline object: holds graphics/compute state used when
 * replaying commands.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"
#include "SoftwarePipelineLayout.h"
#include "SoftwareShaderModule.h"

namespace kx {

class SoftwarePipeline {
public:
    SoftwarePipeline() = default;
    ~SoftwarePipeline() = default;

    SoftwarePipeline(const SoftwarePipeline&) = delete;
    SoftwarePipeline& operator=(const SoftwarePipeline&) = delete;

    void set_create_info(const KxPipelineCreateInfo* create_info) {
        if (!create_info) return;
        create_info_ = *create_info;
    }

    const KxPipelineCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    KxPipelineBindPoint bind_point() const noexcept {
        return create_info_.bindPoint;
    }

    // Vertex input state
    const KxPipelineVertexInputStateCreateInfo* vertex_input_state() const noexcept {
        return create_info_.pVertexInputState;
    }

    // Viewport state
    const KxPipelineViewportStateCreateInfo* viewport_state() const noexcept {
        return create_info_.pViewportState;
    }

    // Rasterization state
    const KxPipelineRasterizationStateCreateInfo* rasterization_state() const noexcept {
        return create_info_.pRasterizationState;
    }

    // Multisample state
    const KxPipelineMultisampleStateCreateInfo* multisample_state() const noexcept {
        return create_info_.pMultisampleState;
    }

    // Depth/stencil state
    const KxPipelineDepthStencilStateCreateInfo* depth_stencil_state() const noexcept {
        return create_info_.pDepthStencilState;
    }

    // Color blend state
    const KxPipelineColorBlendStateCreateInfo* color_blend_state() const noexcept {
        return create_info_.pColorBlendState;
    }

    // Dynamic state
    const KxPipelineDynamicStateCreateInfo* dynamic_state() const noexcept {
        return create_info_.pDynamicState;
    }

    // Legacy accessors (for backward compatibility)
    KxPrimitiveTopology primitive_topology() const noexcept {
        return create_info_.primitiveTopology;
    }

    KxPolygonMode polygon_mode() const noexcept {
        return create_info_.pRasterizationState ? create_info_.pRasterizationState->polygonMode : KX_POLYGON_MODE_FILL;
    }

    KxFrontFace front_face() const noexcept {
        return create_info_.pRasterizationState ? create_info_.pRasterizationState->frontFace : KX_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    bool depth_test_enable() const noexcept {
        if (create_info_.pDepthStencilState) {
            return create_info_.pDepthStencilState->depthTestEnable;
        }
        return false;
    }

    bool depth_write_enable() const noexcept {
        if (create_info_.pDepthStencilState) {
            return create_info_.pDepthStencilState->depthWriteEnable;
        }
        return false;
    }

    KxCompareOp depth_compare_op() const noexcept {
        if (create_info_.pDepthStencilState) {
            return create_info_.pDepthStencilState->depthCompareOp;
        }
        return KX_COMPARE_OP_LESS_OR_EQUAL;
    }

    SoftwarePipelineLayout* pipeline_layout() const noexcept {
        return pipeline_layout_;
    }

    void set_pipeline_layout(SoftwarePipelineLayout* layout) noexcept {
        pipeline_layout_ = layout;
    }

    SoftwareShaderModule* vertex_shader() const noexcept {
        return vertex_shader_;
    }

    void set_vertex_shader(SoftwareShaderModule* shader) noexcept {
        vertex_shader_ = shader;
    }

    SoftwareShaderModule* fragment_shader() const noexcept {
        return fragment_shader_;
    }

    void set_fragment_shader(SoftwareShaderModule* shader) noexcept {
        fragment_shader_ = shader;
    }

    SoftwareShaderModule* compute_shader() const noexcept {
        return compute_shader_;
    }

    void set_compute_shader(SoftwareShaderModule* shader) noexcept {
        compute_shader_ = shader;
    }

private:
    KxPipelineCreateInfo create_info_{};
    SoftwarePipelineLayout* pipeline_layout_ = nullptr;
    SoftwareShaderModule* vertex_shader_ = nullptr;
    SoftwareShaderModule* fragment_shader_ = nullptr;
    SoftwareShaderModule* compute_shader_ = nullptr;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H */