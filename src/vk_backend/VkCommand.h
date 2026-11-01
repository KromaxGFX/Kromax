#ifndef KROMAX_SRC_VK_BACKEND_VKCOMMAND_H
#define KROMAX_SRC_VK_BACKEND_VKCOMMAND_H

/*
 * VkCommand.h
 *
 * Vulkan command pool and command buffer wrappers.
 */

#include "VkLoader.h"
#include "VkPipeline.h"
#include "VkDescriptor.h"
#include <kx_core.h>

#include <vector>
#include <string>
#include <array>
#include <cstring>

namespace kx {

// Command pool
class VkCommandPoolImpl final {
public:
    VkCommandPoolImpl() = default;
    ~VkCommandPoolImpl();

    VkCommandPoolImpl(const VkCommandPoolImpl&) = delete;
    VkCommandPoolImpl& operator=(const VkCommandPoolImpl&) = delete;

    VkCommandPoolImpl(VkCommandPoolImpl&& other) noexcept;
    VkCommandPoolImpl& operator=(VkCommandPoolImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const KxCommandPoolCreateInfo* create_info);

    VkCommandPool handle() const noexcept { return pool_; }
    uint32_t queue_family_index() const noexcept { return queue_family_index_; }

private:
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    uint32_t queue_family_index_ = 0;
};

// Command buffer
class VkCommandBufferImpl final {
public:
    VkCommandBufferImpl() = default;
    ~VkCommandBufferImpl();

    VkCommandBufferImpl(const VkCommandBufferImpl&) = delete;
    VkCommandBufferImpl& operator=(const VkCommandBufferImpl&) = delete;

    VkCommandBufferImpl(VkCommandBufferImpl&& other) noexcept;
    VkCommandBufferImpl& operator=(VkCommandBufferImpl&& other) noexcept;

    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    VkCommandPool pool,
                    KxCommandBufferLevel level);

    VkCommandBuffer handle() const noexcept { return buffer_; }
    VkCommandPool pool() const noexcept { return pool_; }
    KxCommandBufferLevel level() const noexcept { return level_; }

    // Recording state
    bool is_recording() const noexcept { return recording_; }
    void set_recording(bool recording) noexcept { recording_ = recording; }
    bool is_inside_render_pass() const noexcept { return inside_render_pass_; }
    void set_inside_render_pass(bool inside) noexcept { inside_render_pass_ = inside; }

    // Bound pipeline state
    VkPipelineLayout current_pipeline_layout() const noexcept { return current_pipeline_layout_; }
    void set_current_pipeline_layout(VkPipelineLayout layout) noexcept { current_pipeline_layout_ = layout; }
    VkPipelineBindPoint current_pipeline_bind_point() const noexcept { return current_pipeline_bind_point_; }
    void set_current_pipeline_bind_point(VkPipelineBindPoint point) noexcept { current_pipeline_bind_point_ = point; }

    // Descriptor sets
    const std::vector<VkDescriptorSet>& current_descriptor_sets() const noexcept { return current_descriptor_sets_; }
    void set_current_descriptor_sets(const std::vector<VkDescriptorSet>& sets, uint32_t first_set) noexcept {
        current_descriptor_sets_ = sets;
        first_descriptor_set_ = first_set;
    }
    uint32_t first_descriptor_set() const noexcept { return first_descriptor_set_; }

    // Bound vertex buffers
    struct VertexBufferBinding {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize offset = 0;
    };
    const std::vector<VertexBufferBinding>& vertex_buffers() const noexcept { return vertex_buffers_; }
    void bind_vertex_buffers(uint32_t first_binding, const std::vector<VertexBufferBinding>& buffers) noexcept {
        if (first_binding + buffers.size() > vertex_buffers_.size()) {
            vertex_buffers_.resize(first_binding + buffers.size());
        }
        for (size_t i = 0; i < buffers.size(); ++i) {
            vertex_buffers_[first_binding + i] = buffers[i];
        }
    }

    // Bound index buffer
    VkBuffer index_buffer() const noexcept { return index_buffer_; }
    VkDeviceSize index_buffer_offset() const noexcept { return index_buffer_offset_; }
    VkIndexType index_type() const noexcept { return index_type_; }
    void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type) noexcept {
        index_buffer_ = buffer;
        index_buffer_offset_ = offset;
        index_type_ = type;
    }

    // Push constants
    const std::vector<uint8_t>& push_constants() const noexcept { return push_constants_; }
    void set_push_constants(VkPipelineLayout layout, VkShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) noexcept {
        if (offset + size > push_constants_.size()) {
            push_constants_.resize(offset + size);
        }
        std::memcpy(push_constants_.data() + offset, data, size);
    }

    // Dynamic state
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
    float line_width_ = 1.0f;
    float depth_bias_constant_factor_ = 0.0f;
    float depth_bias_clamp_ = 0.0f;
    float depth_bias_slope_factor_ = 0.0f;
    std::array<float, 4> blend_constants_ = {0, 0, 0, 0};
    uint32_t stencil_compare_mask_ = 0xFFFFFFFF;
    uint32_t stencil_write_mask_ = 0xFFFFFFFF;
    uint32_t stencil_reference_ = 0;

private:
    VkCommandBuffer buffer_ = VK_NULL_HANDLE;
    VkCommandPool pool_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    KxCommandBufferLevel level_ = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    bool recording_ = false;
    bool inside_render_pass_ = false;

    // State tracking
    VkPipelineLayout current_pipeline_layout_ = VK_NULL_HANDLE;
    VkPipelineBindPoint current_pipeline_bind_point_ = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkDescriptorSet> current_descriptor_sets_;
    uint32_t first_descriptor_set_ = 0;
    std::vector<VertexBufferBinding> vertex_buffers_;
    VkBuffer index_buffer_ = VK_NULL_HANDLE;
    VkDeviceSize index_buffer_offset_ = 0;
    VkIndexType index_type_ = VK_INDEX_TYPE_UINT16;
    std::vector<uint8_t> push_constants_;
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKCOMMAND_H