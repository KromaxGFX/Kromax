#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSET_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSET_H

/*
 * SoftwareDescriptorSet.h
 *
 * Software descriptor set: holds descriptor data per binding (images, buffers, samplers).
 */

#include <cstdint>
#include <vector>
#include <variant>
#include <optional>

#include "kx_core.h"
#include "SoftwareDescriptorSetLayout.h"
#include "SoftwareImageView.h"
#include "SoftwareBuffer.h"
#include "SoftwareSampler.h"

namespace kx {

class SoftwareDevice;

// Descriptor data for a single binding array element
struct DescriptorData {
    // For sampled image / storage image / input attachment
    struct ImageDescriptor {
        SoftwareImageView* image_view = nullptr;
        SoftwareSampler* sampler = nullptr;        // For sampled image
        KxImageLayout layout = KX_IMAGE_LAYOUT_UNDEFINED;
    };
    // For uniform buffer / storage buffer / uniform texel buffer / storage texel buffer
    struct BufferDescriptor {
        SoftwareBuffer* buffer = nullptr;
        uint64_t offset = 0;
        uint64_t range = 0;  // KX_WHOLE_SIZE means rest of buffer
    };

    std::variant<std::monostate, ImageDescriptor, BufferDescriptor> data;
    KxDescriptorType type = KX_DESCRIPTOR_TYPE_MAX_ENUM;
    bool valid = false;

    DescriptorData() = default;
    DescriptorData(const DescriptorData&) = default;
    DescriptorData& operator=(const DescriptorData&) = default;
};

class SoftwareDescriptorSet {
public:
    SoftwareDescriptorSet() = default;
    ~SoftwareDescriptorSet() = default;

    SoftwareDescriptorSet(const SoftwareDescriptorSet&) = delete;
    SoftwareDescriptorSet& operator=(const SoftwareDescriptorSet&) = delete;

    // Initialize with layout and device reference
    void init(const SoftwareDescriptorSetLayout* layout, SoftwareDevice* device) {
        layout_ = layout;
        device_ = device;
        if (layout_) {
            // One vector per binding, sized by descriptorCount
            binding_data_.resize(layout_->binding_count());
            for (uint32_t i = 0; i < layout_->binding_count(); ++i) {
                const auto& binding = layout_->bindings()[i];
                binding_data_[i].resize(binding.descriptorCount);
                for (auto& desc : binding_data_[i]) {
                    desc.type = binding.descriptorType;
                }
            }
        }
    }

    const SoftwareDescriptorSetLayout* layout() const noexcept { return layout_; }

    // Update descriptors from write descriptor set
    KxResult update(const KxWriteDescriptorSet& write);

    // Get all valid descriptors for a binding (for shader access)
    const std::vector<DescriptorData>& binding_data(uint32_t binding_idx) const noexcept {
        static const std::vector<DescriptorData> empty;
        if (binding_idx < binding_data_.size()) return binding_data_[binding_idx];
        return empty;
    }

    // Get single descriptor (for simple cases)
    std::optional<DescriptorData> get_descriptor(uint32_t binding_idx, uint32_t array_element = 0) const noexcept {
        if (binding_idx >= binding_data_.size()) return std::nullopt;
        const auto& vec = binding_data_[binding_idx];
        if (array_element >= vec.size()) return std::nullopt;
        return vec[array_element];
    }

    // Check if a binding has any valid descriptors
    bool has_valid_descriptor(uint32_t binding_idx) const noexcept {
        if (binding_idx >= binding_data_.size()) return false;
        for (const auto& d : binding_data_[binding_idx]) {
            if (d.valid) return true;
        }
        return false;
    }

private:
    const SoftwareDescriptorSetLayout* layout_ = nullptr;
    SoftwareDevice* device_ = nullptr;
    std::vector<std::vector<DescriptorData>> binding_data_; // binding_data_[binding_idx][array_element]
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSET_H */