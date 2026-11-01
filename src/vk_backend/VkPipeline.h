#ifndef KROMAX_SRC_VK_BACKEND_VKPIPELINE_H
#define KROMAX_SRC_VK_BACKEND_VKPIPELINE_H

/*
 * VkPipeline.h
 *
 * Vulkan pipeline and pipeline layout wrappers.
 */

#include "VkLoader.h"
#include "VkShaderModule.h"
#include "VkDescriptor.h"
#include <kx_core.h>
#include <kx_pipeline.h>

#include <vector>
#include <string>
#include <array>

namespace kx {

// Pipeline layout
class VkPipelineLayoutImpl final {
public:
    VkPipelineLayoutImpl() = default;
    ~VkPipelineLayoutImpl();

    VkPipelineLayoutImpl(const VkPipelineLayoutImpl&) = delete;
    VkPipelineLayoutImpl& operator=(const VkPipelineLayoutImpl&) = delete;

    VkPipelineLayoutImpl(VkPipelineLayoutImpl&& other) noexcept;
    VkPipelineLayoutImpl& operator=(VkPipelineLayoutImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxPipelineLayoutCreateInfo* create_info);

    VkPipelineLayout handle() const noexcept { return layout_; }
    const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts() const noexcept { return set_layouts_; }
    const std::vector<VkPushConstantRange>& push_constant_ranges() const noexcept { return push_constants_; }

private:
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> set_layouts_;
    std::vector<VkPushConstantRange> push_constants_;
};

// Graphics pipeline
class VkPipelineImpl final {
public:
    VkPipelineImpl() = default;
    ~VkPipelineImpl();

    VkPipelineImpl(const VkPipelineImpl&) = delete;
    VkPipelineImpl& operator=(const VkPipelineImpl&) = delete;

    VkPipelineImpl(VkPipelineImpl&& other) noexcept;
    VkPipelineImpl& operator=(VkPipelineImpl&& other) noexcept;

    // Create graphics pipeline
    bool initialize_graphics(VkFunctionTable& functions,
                             VkDevice device,
                             VkPipelineCache cache,
                             const KxPipelineCreateInfo* create_info);

    // Create compute pipeline
    bool initialize_compute(VkFunctionTable& functions,
                            VkDevice device,
                            VkPipelineCache cache,
                            const KxPipelineCreateInfo* create_info);

    VkPipeline handle() const noexcept { return pipeline_; }
    VkPipelineLayout layout() const noexcept { return layout_; }
    bool is_compute() const noexcept { return is_compute_; }

private:
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    bool is_compute_ = false;
    
    // Pipeline create info components for graphics
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;
    VkPipelineVertexInputStateCreateInfo vertex_input_state_{};
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_{};
    VkPipelineTessellationStateCreateInfo tessellation_state_{};
    VkPipelineViewportStateCreateInfo viewport_state_{};
    VkPipelineRasterizationStateCreateInfo rasterization_state_{};
    VkPipelineMultisampleStateCreateInfo multisample_state_{};
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_{};
    VkPipelineColorBlendStateCreateInfo color_blend_state_{};
    VkPipelineDynamicStateCreateInfo dynamic_state_{};
    
    // Dynamic state arrays
    std::vector<VkDynamicState> dynamic_states_;
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments_;
    std::vector<VkVertexInputBindingDescription> vertex_bindings_;
    std::vector<VkVertexInputAttributeDescription> vertex_attributes_;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKPIPELINE_H