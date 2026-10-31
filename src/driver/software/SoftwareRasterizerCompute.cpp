/*
 * SoftwareRasterizerCompute.cpp
 *
 * Compute shader execution on the software rasterizer.
 */

#include "SoftwareRasterizerCompute.h"

#include <algorithm>
#include <cstring>

#include "SoftwareShaderModule.h"
#include "SoftwarePipeline.h"
#include "SoftwareDescriptorSet.h"
#include "SoftwareBuffer.h"
#include "SoftwareImage.h"
#include "../shader/ShaderInterpreter.h"

namespace kx {

void SoftwareRasterizerCompute::dispatch(const ComputeDispatchInfo& info,
                                         SoftwareShaderModule* compute_shader,
                                         const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                         const void* push_constants,
                                         uint32_t push_constants_size) const {
    if (!compute_shader || !compute_shader->has_ir() || !compute_shader->interpreter) {
        // Fallback to native callback if available
        if (compute_shader && compute_shader->compute_callback) {
            // Native callback path (not implemented here)
        }
        return;
    }

    // Iterate over all workgroups
    for (uint32_t gz = 0; gz < info.group_count_z; ++gz) {
        for (uint32_t gy = 0; gy < info.group_count_y; ++gy) {
            for (uint32_t gx = 0; gx < info.group_count_x; ++gx) {
                execute_workgroup(info, gx, gy, gz, compute_shader, descriptor_sets,
                                  push_constants, push_constants_size);
            }
        }
    }
}

void SoftwareRasterizerCompute::execute_workgroup(const ComputeDispatchInfo& info,
                                                   uint32_t group_x, uint32_t group_y, uint32_t group_z,
                                                   SoftwareShaderModule* compute_shader,
                                                   const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                                   const void* push_constants,
                                                   uint32_t push_constants_size) const {
    // Iterate over all local invocations in this workgroup
    for (uint32_t lz = 0; lz < info.local_size_z; ++lz) {
        for (uint32_t ly = 0; ly < info.local_size_y; ++ly) {
            for (uint32_t lx = 0; lx < info.local_size_x; ++lx) {
                execute_local_invocation(info, group_x, group_y, group_z,
                                         lx, ly, lz,
                                         compute_shader, descriptor_sets,
                                         push_constants, push_constants_size);
            }
        }
    }
}

void SoftwareRasterizerCompute::execute_local_invocation(const ComputeDispatchInfo& info,
                                                          uint32_t group_x, uint32_t group_y, uint32_t group_z,
                                                          uint32_t local_x, uint32_t local_y, uint32_t local_z,
                                                          SoftwareShaderModule* compute_shader,
                                                          const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                                          const void* push_constants,
                                                          uint32_t push_constants_size) const {
    // Build compute input
    ComputeInput input;
    input.group_id_x = group_x;
    input.group_id_y = group_y;
    input.group_id_z = group_z;
    input.local_id_x = local_x;
    input.local_id_y = local_y;
    input.local_id_z = local_z;
    input.global_id_x = group_x * info.local_size_x + local_x;
    input.global_id_y = group_y * info.local_size_y + local_y;
    input.global_id_z = group_z * info.local_size_z + local_z;
    input.local_index = (local_z * info.local_size_y + local_y) * info.local_size_x + local_x;

    // Build shader environment from descriptor sets
    shader::ShaderEnvironment env;
    build_shader_environment(descriptor_sets, env);

    // Execute compute shader via interpreter
    // The interpreter expects the entry function to take the compute input as parameters
    // For now, we'll use a simplified approach - the interpreter needs proper binding
    // between the compute input built-ins and the shader parameters

    // TODO: Full interpreter integration for compute shaders
    // This requires:
    // 1. Binding built-in variables (gl_GlobalInvocationID, gl_LocalInvocationID, etc.)
    // 2. Setting up storage buffer/image bindings from descriptor sets
    // 3. Handling shared memory

    // Placeholder: call the interpreter with empty args
    // In a full implementation, we'd pass the ComputeInput as arguments
    (void)compute_shader;
    (void)push_constants;
    (void)push_constants_size;
    (void)input;
    (void)env;
}

void SoftwareRasterizerCompute::build_shader_environment(const std::vector<SoftwareDescriptorSet*>& descriptor_sets,
                                                         shader::ShaderEnvironment& env) const {
    // Resolve descriptor sets into uniform blocks, textures, samplers
    for (const auto* ds : descriptor_sets) {
        if (!ds || !ds->layout()) continue;

        const auto* layout = ds->layout();
        const auto& bindings = layout->bindings();

        for (uint32_t binding_idx = 0; binding_idx < layout->binding_count(); ++binding_idx) {
            const auto& binding = bindings[binding_idx];
            const auto& binding_data = ds->binding_data(binding_idx);

            for (const auto& desc : binding_data) {
                if (!desc.valid) continue;

                switch (desc.type) {
                    case KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    case KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
                        if (auto* buf_desc = std::get_if<DescriptorData::BufferDescriptor>(&desc.data)) {
                            if (buf_desc->buffer && buf_desc->buffer->mapped_ptr()) {
                                shader::UniformBlockResource res;
                                res.data = static_cast<const uint8_t*>(buf_desc->buffer->mapped_ptr()) + buf_desc->offset;
                                res.size = static_cast<uint32_t>(buf_desc->range == UINT64_MAX ? buf_desc->buffer->size() - buf_desc->offset : buf_desc->range);
                                res.set = 0;
                                res.binding = binding.binding;
                                env.uniform_blocks.push_back(res);
                            }
                        }
                        break;
                    }
                    case KX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    case KX_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                        if (auto* buf_desc = std::get_if<DescriptorData::BufferDescriptor>(&desc.data)) {
                            if (buf_desc->buffer && buf_desc->buffer->mapped_ptr()) {
                                shader::UniformBlockResource res;
                                res.data = static_cast<const uint8_t*>(buf_desc->buffer->mapped_ptr()) + buf_desc->offset;
                                res.size = static_cast<uint32_t>(buf_desc->range == UINT64_MAX ? buf_desc->buffer->size() - buf_desc->offset : buf_desc->range);
                                res.set = 0;
                                res.binding = binding.binding;
                                env.uniform_blocks.push_back(res);
                            }
                        }
                        break;
                    }
                    case KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    case KX_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
                        if (auto* img_desc = std::get_if<DescriptorData::ImageDescriptor>(&desc.data)) {
                            if (img_desc->image_view && img_desc->image_view->image()) {
                                auto* image = img_desc->image_view->image();
                                // Create a temporary KxBoundTexture for the interpreter
                                // Note: In a full implementation, we'd store these persistently
                                KxBoundTexture bt{};
                                bt.sampler = reinterpret_cast<KxSampler>(img_desc->sampler);
                                bt.imageView = reinterpret_cast<KxImageView>(img_desc->image_view);
                                bt.width = image->width();
                                bt.height = image->height();
                                bt.rowPitch = static_cast<uint32_t>(image->row_pitch());
                                bt.format = image->format();
                                bt.pTexels = image->data();
                                bt.mipLevels = image->mip_levels();
                                bt.baseMipLevel = 0;
                                bt.arrayLayers = image->array_layers();
                                bt.samplerState = kxDefaultSamplerState();
                                // Note: We can't store bt persistently here, so we skip adding to env for now
                                // env.textures.push_back({&bt});
                            }
                        }
                        break;
                    }
                    case KX_DESCRIPTOR_TYPE_SAMPLER: {
                        if (auto* img_desc = std::get_if<DescriptorData::ImageDescriptor>(&desc.data)) {
                            if (img_desc->sampler) {
                                shader::SamplerResource samp_res;
                                samp_res.state = kxDefaultSamplerState(); // img_desc->sampler->state();
                                env.samplers.push_back(samp_res);
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }
}

} /* namespace kx */