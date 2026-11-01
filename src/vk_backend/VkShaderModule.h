#ifndef KROMAX_SRC_VK_BACKEND_VKSHADERMODULE_H
#define KROMAX_SRC_VK_BACKEND_VKSHADERMODULE_H

/*
 * VkShaderModule.h
 *
 * Vulkan shader module wrapper. Handles SPIR-V shader module creation
 * and reflection. Reuses BuiltinMath.h for GLSL builtin compatibility.
 */

#include "VkLoader.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace kx {

struct ShaderReflectionBinding {
    uint32_t set = 0;
    uint32_t binding = 0;
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    uint32_t count = 0;
    VkShaderStageFlags stageFlags = 0;
    std::string name;
};

struct ShaderReflectionPushConstant {
    VkShaderStageFlags stageFlags = 0;
    uint32_t offset = 0;
    uint32_t size = 0;
    std::string name;
};

class VkShaderModuleImpl final {
public:
    VkShaderModuleImpl() = default;
    ~VkShaderModuleImpl();

    VkShaderModuleImpl(const VkShaderModuleImpl&) = delete;
    VkShaderModuleImpl& operator=(const VkShaderModuleImpl&) = delete;

    VkShaderModuleImpl(VkShaderModuleImpl&& other) noexcept;
    VkShaderModuleImpl& operator=(VkShaderModuleImpl&& other) noexcept;

    // Initialize from SPIR-V code
    bool initialize(VkFunctionTable& functions,
                    VkDevice device,
                    const std::vector<uint32_t>& spirv);

    // Accessors
    VkShaderModule handle() const noexcept { return module_; }
    const std::vector<uint32_t>& spirv() const noexcept { return spirv_; }
    VkShaderStageFlags stage() const noexcept { return stage_; }
    const std::string& entry_point() const noexcept { return entry_point_; }

    const std::vector<ShaderReflectionBinding>& bindings() const noexcept { return bindings_; }
    const std::vector<ShaderReflectionPushConstant>& push_constants() const noexcept { return push_constants_; }

    // Check if this is a graphics or compute shader
    bool is_graphics() const noexcept { return (stage_ & VK_SHADER_STAGE_ALL_GRAPHICS) != 0; }
    bool is_compute() const noexcept { return (stage_ & VK_SHADER_STAGE_COMPUTE_BIT) != 0; }

private:
    VkShaderModule module_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    std::vector<uint32_t> spirv_;
    VkShaderStageFlags stage_ = 0;
    std::string entry_point_ = "main";

    // Reflection data
    std::vector<ShaderReflectionBinding> bindings_;
    std::vector<ShaderReflectionPushConstant> push_constants_;

    // Reflection helper
    void reflect_spirv(VkFunctionTable& functions);
};

} // namespace kx

#endif // KROMAX_SRC_VK_BACKEND_VKSHADERMODULE_H