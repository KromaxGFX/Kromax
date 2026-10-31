#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZERCOMPUTE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZERCOMPUTE_H

/*
 * SoftwareRasterizerCompute.h
 *
 * Compute shader execution on the software rasterizer.
 */

#include <cstdint>
#include <vector>

#include "SoftwareShaderModule.h"
#include "SoftwarePipeline.h"
#include "SoftwareDescriptorSet.h"
#include "SoftwareBuffer.h"
#include "../shader/ShaderInterpreter.h"

namespace kx {

struct ComputeDispatchInfo {
    uint32_t group_count_x = 1;
    uint32_t group_count_y = 1;
    uint32_t group_count_z = 1;
    uint32_t local_size_x = 1;
    uint32_t local_size_y = 1;
    uint32_t local_size_z = 1;
};

struct ComputeInput {
    uint32_t group_id_x = 0;
    uint32_t group_id_y = 0;
    uint32_t group_id_z = 0;
    uint32_t local_id_x = 0;
    uint32_t local_id_y = 0;
    uint32_t local_id_z = 0;
    uint32_t global_id_x = 0;
    uint32_t global_id_y = 0;
    uint32_t global_id_z = 0;
    uint32_t local_index = 0;
};

class SoftwareRasterizerCompute {
public:
    explicit SoftwareRasterizerCompute(const SoftwareDevice* device) : device_(device) {}

    void dispatch(const ComputeDispatchInfo& info,
                  SoftwareShaderModule* compute_shader,
                  const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                  const void* push_constants,
                  uint32_t push_constants_size) const;

private:
    void execute_workgroup(const ComputeDispatchInfo& info,
                           uint32_t group_x, uint32_t group_y, uint32_t group_z,
                           SoftwareShaderModule* compute_shader,
                           const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                           const void* push_constants,
                           uint32_t push_constants_size) const;

    void execute_local_invocation(const ComputeDispatchInfo& info,
                                  uint32_t group_x, uint32_t group_y, uint32_t group_z,
                                  uint32_t local_x, uint32_t local_y, uint32_t local_z,
                                  SoftwareShaderModule* compute_shader,
                                  const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                  const void* push_constants,
                                  uint32_t push_constants_size) const;

    void build_shader_environment(const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                  shader::ShaderEnvironment& env) const;

    const SoftwareDevice* device_ = nullptr;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZERCOMPUTE_H */