#ifndef KROMAX_SRC_VK_BACKEND_VKPIPELINE_H
#define KROMAX_SRC_VK_BACKEND_VKPIPELINE_H

/*
 * VkPipeline.h
 *
 * Vulkan pipeline and pipeline layout wrappers.
 */

#include "VkLoader.h"
#include "VkShaderModule.h"
#include <kx_core.h>

#include <vector>
#include <string>

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
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKPIPELINE_H