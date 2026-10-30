#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSETLAYOUT_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSETLAYOUT_H

/*
 * SoftwareDescriptorSetLayout.h
 *
 * Software descriptor set layout: stores binding table from create info.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareDescriptorSetLayout {
public:
    SoftwareDescriptorSetLayout() = default;
    explicit SoftwareDescriptorSetLayout(const KxDescriptorSetLayoutCreateInfo* create_info) {
        set_create_info(create_info);
    }
    ~SoftwareDescriptorSetLayout() = default;

    SoftwareDescriptorSetLayout(const SoftwareDescriptorSetLayout&) = delete;
    SoftwareDescriptorSetLayout& operator=(const SoftwareDescriptorSetLayout&) = delete;

    void set_create_info(const KxDescriptorSetLayoutCreateInfo* create_info) {
        if (!create_info) return;
        create_info_ = *create_info;

        // Deep copy bindings
        if (create_info->bindingCount > 0 && create_info->pBindings) {
            bindings_.resize(create_info->bindingCount);
            for (uint32_t i = 0; i < create_info->bindingCount; ++i) {
                bindings_[i] = create_info->pBindings[i];
            }
        }
    }

    const KxDescriptorSetLayoutCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    const std::vector<KxDescriptorSetLayoutBinding>& bindings() const noexcept {
        return bindings_;
    }

    uint32_t binding_count() const noexcept {
        return static_cast<uint32_t>(bindings_.size());
    }

    // Find binding by binding number
    const KxDescriptorSetLayoutBinding* find_binding(uint32_t binding) const noexcept {
        for (const auto& b : bindings_) {
            if (b.binding == binding) return &b;
        }
        return nullptr;
    }

private:
    KxDescriptorSetLayoutCreateInfo create_info_{};
    std::vector<KxDescriptorSetLayoutBinding> bindings_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORSETLAYOUT_H */