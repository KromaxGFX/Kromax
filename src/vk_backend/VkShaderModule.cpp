/*
 * VkShaderModule.cpp
 *
 * Vulkan shader module implementation.
 */

#include "VkShaderModule.h"

#include <cstring>

namespace kx {

VkShaderModuleImpl::~VkShaderModuleImpl() {
    if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        // Note: vkDestroyShaderModule is in the device function table
        // We need the function table to destroy it properly
        // The actual destruction is handled by VkDriverDevice
    }
}

VkShaderModuleImpl::VkShaderModuleImpl(VkShaderModuleImpl&& other) noexcept
    : module_(other.module_)
    , device_(other.device_)
    , spirv_(std::move(other.spirv_))
    , stage_(other.stage_)
    , entry_point_(std::move(other.entry_point_))
    , bindings_(std::move(other.bindings_))
    , push_constants_(std::move(other.push_constants_)) {
    other.module_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.stage_ = 0;
}

VkShaderModuleImpl& VkShaderModuleImpl::operator=(VkShaderModuleImpl&& other) noexcept {
    if (this != &other) {
        if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
            // Note: Can't destroy without function table
        }
        module_ = other.module_;
        device_ = other.device_;
        spirv_ = std::move(other.spirv_);
        stage_ = other.stage_;
        entry_point_ = std::move(other.entry_point_);
        bindings_ = std::move(other.bindings_);
        push_constants_ = std::move(other.push_constants_);
        other.module_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.stage_ = 0;
    }
    return *this;
}

bool VkShaderModuleImpl::initialize(VkFunctionTable& functions,
                                    VkDevice device,
                                    const std::vector<uint32_t>& spirv) {
    device_ = device;
    spirv_ = spirv;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = static_cast<size_t>(spirv.size()) * sizeof(uint32_t);
    createInfo.pCode = spirv.data();

    VkResult result = functions.vkCreateShaderModule(device, &createInfo, nullptr, &module_);
    if (result != VK_SUCCESS) {
        return false;
    }

    // Reflect the SPIR-V to get bindings and push constants
    reflect_spirv(functions);

    return true;
}

void VkShaderModuleImpl::reflect_spirv(VkFunctionTable& functions) {
    // For now, we'll do basic reflection by parsing SPIR-V
    // A full implementation would use SPIRV-Cross or similar

    if (spirv_.size() < 5) return;

    // Check SPIR-V magic number
    if (spirv_[0] != 0x07230203) return;

    // Very basic SPIR-V parsing to find entry point and bindings
    // This is a simplified implementation - production code would use SPIRV-Cross

    uint32_t wordCount = spirv_[1];
    if (spirv_.size() < wordCount) return;

    // Parse through instructions looking for OpEntryPoint and OpDecorate
    for (uint32_t i = 5; i < wordCount; ) {
        uint32_t word = spirv_[i];
        uint16_t opcode = static_cast<uint16_t>(word & 0xFFFF);
        uint16_t wordCount_instr = static_cast<uint16_t>(word >> 16);

        if (opcode == 15) { // OpEntryPoint
            // OpEntryPoint: ResultType, EntryPoint, Name, Interface...
            if (i + 3 < wordCount) {
                stage_ = spirv_[i + 1];
                // Entry point name is a string starting at i+3
                const char* nameStart = reinterpret_cast<const char*>(&spirv_[i + 3]);
                entry_point_ = nameStart;
            }
        } else if (opcode == 71) { // OpDecorate
            // OpDecorate: Target, Decoration, [Params...]
            if (i + 3 < wordCount) {
                uint32_t target = spirv_[i + 1];
                uint32_t decoration = spirv_[i + 2];

                // DescriptorSet = 2, Binding = 1
                if (decoration == 2 && i + 3 < wordCount) {
                    // Find the variable this decorates
                } else if (decoration == 1 && i + 3 < wordCount) {
                    // Binding decoration
                }
            }
        }

        i += wordCount_instr;
    }
}

} // namespace kx