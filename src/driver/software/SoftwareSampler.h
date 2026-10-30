#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESAMPLER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESAMPLER_H

/*
 * SoftwareSampler.h
 *
 * Software sampler: stores sampler create info (filter, address mode, compare, border, LOD).
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class SoftwareSampler {
public:
    SoftwareSampler() = default;
    explicit SoftwareSampler(const KxSamplerCreateInfo* create_info) {
        set_create_info(create_info);
    }
    ~SoftwareSampler() = default;

    SoftwareSampler(const SoftwareSampler&) = delete;
    SoftwareSampler& operator=(const SoftwareSampler&) = delete;

    void set_create_info(const KxSamplerCreateInfo* create_info) {
        if (!create_info) return;
        create_info_ = *create_info;
    }

    const KxSamplerCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    // Convenience accessors
    KxFilter mag_filter() const noexcept { return create_info_.magFilter; }
    KxFilter min_filter() const noexcept { return create_info_.minFilter; }
    KxSamplerAddressMode address_mode_u() const noexcept { return create_info_.addressModeU; }
    KxSamplerAddressMode address_mode_v() const noexcept { return create_info_.addressModeV; }
    KxSamplerAddressMode address_mode_w() const noexcept { return create_info_.addressModeW; }
    float mip_lod_bias() const noexcept { return create_info_.mipLodBias; }
    bool anisotropy_enable() const noexcept { return create_info_.anisotropyEnable; }
    float max_anisotropy() const noexcept { return create_info_.maxAnisotropy; }
    bool compare_enable() const noexcept { return create_info_.compareEnable; }
    KxCompareOp compare_op() const noexcept { return create_info_.compareOp; }
    float min_lod() const noexcept { return create_info_.minLod; }
    float max_lod() const noexcept { return create_info_.maxLod; }
    KxBorderColor border_color() const noexcept { return create_info_.borderColor; }
    bool unnormalized_coordinates() const noexcept { return create_info_.unnormalizedCoordinates; }

private:
    KxSamplerCreateInfo create_info_{};
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESAMPLER_H */