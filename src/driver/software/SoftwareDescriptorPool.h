#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORPOOL_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORPOOL_H

/*
 * SoftwareDescriptorPool.h
 *
 * Software descriptor pool: manages capacity per descriptor type and allocates descriptor sets.
 */

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "kx_core.h"
#include "SoftwareDescriptorSetLayout.h"

namespace kx {

class SoftwareDescriptorPool {
public:
    SoftwareDescriptorPool() = default;
    explicit SoftwareDescriptorPool(const KxDescriptorPoolCreateInfo* create_info) {
        init(create_info);
    }
    ~SoftwareDescriptorPool() = default;

    SoftwareDescriptorPool(const SoftwareDescriptorPool&) = delete;
    SoftwareDescriptorPool& operator=(const SoftwareDescriptorPool&) = delete;

    KxResult init(const KxDescriptorPoolCreateInfo* create_info) {
        if (!create_info) return KX_ERROR_INITIALIZATION_FAILED;

        create_info_ = *create_info;

        // Initialize capacity per descriptor type
        if (create_info->poolSizeCount > 0 && create_info->pPoolSizes) {
            for (uint32_t i = 0; i < create_info->poolSizeCount; ++i) {
                const auto& pool_size = create_info->pPoolSizes[i];
                capacity_[pool_size.type] = pool_size.descriptorCount;
                used_[pool_size.type] = 0;
            }
        }

        max_sets_ = create_info->maxSets;
        allocated_sets_ = 0;
        return KX_SUCCESS;
    }

    const KxDescriptorPoolCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    // Check if pool can allocate a set with given layout
    bool can_allocate(const SoftwareDescriptorSetLayout* layout) const noexcept {
        if (!layout) return false;
        if (allocated_sets_ >= max_sets_) return false;

        // Check capacity for each binding
        for (const auto& binding : layout->bindings()) {
            auto it = capacity_.find(binding.descriptorType);
            if (it == capacity_.end()) return false; // Type not in pool
            uint32_t needed = binding.descriptorCount;
            uint32_t available = it->second - used_.at(binding.descriptorType);
            if (available < needed) return false;
        }
        return true;
    }

    // Reserve capacity for a new set
    void reserve_for_set(const SoftwareDescriptorSetLayout* layout) noexcept {
        if (!layout) return;
        for (const auto& binding : layout->bindings()) {
            used_[binding.descriptorType] += binding.descriptorCount;
        }
        allocated_sets_++;
    }

    // Release capacity from a freed set
    void release_from_set(const SoftwareDescriptorSetLayout* layout) noexcept {
        if (!layout) return;
        for (const auto& binding : layout->bindings()) {
            auto it = used_.find(binding.descriptorType);
            if (it != used_.end() && it->second >= binding.descriptorCount) {
                it->second -= binding.descriptorCount;
            }
        }
        if (allocated_sets_ > 0) allocated_sets_--;
    }

    void reset() noexcept {
        for (auto& [type, count] : used_) count = 0;
        allocated_sets_ = 0;
    }

    uint32_t max_sets() const noexcept { return max_sets_; }
    uint32_t allocated_sets() const noexcept { return allocated_sets_; }
    uint32_t available_sets() const noexcept { return max_sets_ - allocated_sets_; }

    uint32_t capacity(KxDescriptorType type) const noexcept {
        auto it = capacity_.find(type);
        return it != capacity_.end() ? it->second : 0;
    }

    uint32_t used(KxDescriptorType type) const noexcept {
        auto it = used_.find(type);
        return it != used_.end() ? it->second : 0;
    }

    uint32_t available(KxDescriptorType type) const noexcept {
        return capacity(type) - used(type);
    }

    bool free_descriptor_set_allowed() const noexcept {
        return (create_info_.flags & KX_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0;
    }

private:
    KxDescriptorPoolCreateInfo create_info_{};
    std::unordered_map<KxDescriptorType, uint32_t> capacity_;
    std::unordered_map<KxDescriptorType, uint32_t> used_;
    uint32_t max_sets_ = 0;
    uint32_t allocated_sets_ = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDESCRIPTORPOOL_H */