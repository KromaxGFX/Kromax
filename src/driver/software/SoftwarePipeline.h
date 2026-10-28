#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H

/*
 * SoftwarePipeline.h
 *
 * Software pipeline object: holds graphics/compute state used when
 * replaying commands.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class SoftwarePipeline {
public:
    SoftwarePipeline(KxPipelineCreateInfo create_info) noexcept
        : create_info_(create_info) {}
    ~SoftwarePipeline() = default;

    const KxPipelineCreateInfo& create_info() const noexcept {
        return create_info_;
    }

private:
    KxPipelineCreateInfo create_info_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREPIPELINE_H */