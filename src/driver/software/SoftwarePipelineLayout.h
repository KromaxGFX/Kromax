#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINELAYOUT_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINELAYOUT_H

/*
 * SoftwarePipelineLayout.h
 *
 * Software pipeline layout: holds descriptor set layouts and push constant ranges.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwarePipelineLayout {
public:
    SoftwarePipelineLayout() = default;
    explicit SoftwarePipelineLayout(const KxPipelineLayoutCreateInfo* create_info) {
        set_create_info(create_info);
    }
    ~SoftwarePipelineLayout() = default;

    SoftwarePipelineLayout(const SoftwarePipelineLayout&) = delete;
    SoftwarePipelineLayout& operator=(const SoftwarePipelineLayout&) = delete;

    void set_create_info(const KxPipelineLayoutCreateInfo* create_info) {
        if (!create_info) return;
        create_info_ = *create_info;

        // Copy descriptor set layouts (just store handles for now)
        if (create_info->setLayoutCount > 0 && create_info->pSetLayouts) {
            set_layouts_.resize(create_info->setLayoutCount);
            for (uint32_t i = 0; i < create_info->setLayoutCount; ++i) {
                // In a real implementation, we'd resolve the opaque handles
                // For software driver, we just track the count
            }
        }

        // Copy push constant ranges
        if (create_info->pushConstantRangeCount > 0 && create_info->pPushConstantRanges) {
            push_constant_ranges_.resize(create_info->pushConstantRangeCount);
            for (uint32_t i = 0; i < create_info->pushConstantRangeCount; ++i) {
                // The pPushConstantRanges is void*, but KxPushConstantRange is:
                // struct { KxShaderStageFlags stageFlags; uint32_t offset; uint32_t size; }
                // We'll store as bytes for simplicity
            }
        }
    }

    const KxPipelineLayoutCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    uint32_t set_layout_count() const noexcept {
        return static_cast<uint32_t>(set_layouts_.size());
    }

    uint32_t push_constant_range_count() const noexcept {
        return static_cast<uint32_t>(push_constant_ranges_.size());
    }

private:
    KxPipelineLayoutCreateInfo create_info_{};
    std::vector<void*> set_layouts_;
    std::vector<uint8_t> push_constant_ranges_; // Raw storage
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINELAYOUT_H */