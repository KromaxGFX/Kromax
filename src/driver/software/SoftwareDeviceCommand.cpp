/*
 * SoftwareDeviceCommand.cpp
 *
 * Command buffer and queue operations.
 */

#include "SoftwareDevice.h"

#include <cstring>

#include "SoftwareCommandBuffer.h"
#include "SoftwarePipeline.h"
#include "SoftwareShaderModule.h"
#include "SoftwareRasterizer.h"
#include "SoftwareFramebuffer.h"
#include "SoftwareRasterizerCompute.h"

namespace kx {

void SoftwareDevice::execute_command(const CommandVariant& cmd) {
    std::visit([this](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, CmdClearImage>) {
            execute_clear_image(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyBuffer>) {
            execute_copy_buffer(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyBufferToImage>) {
            execute_copy_buffer_to_image(arg);
        } else if constexpr (std::is_same_v<T, CmdBindPipeline>) {
            // Pipeline binding - store for draw/dispatch
            current_pipeline_ = static_cast<SoftwarePipeline*>(arg.pipeline);
        } else if constexpr (std::is_same_v<T, CmdBindVertexBuffers>) {
            current_vertex_buffers_ = arg;
        } else if constexpr (std::is_same_v<T, CmdBindIndexBuffer>) {
            current_index_buffer_ = arg;
        } else if constexpr (std::is_same_v<T, CmdPushConstants>) {
            // Apply push constants to the current pipeline's shader modules
            if (current_pipeline_) {
                const void* values = arg.values.empty() ? nullptr : arg.values.data();
                if (arg.stage_flags & KX_SHADER_STAGE_VERTEX_BIT) {
                    if (current_pipeline_->vertex_shader()) {
                        current_pipeline_->vertex_shader()->push_constants = values;
                        current_pipeline_->vertex_shader()->push_constants_size = arg.size;
                    }
                }
                if (arg.stage_flags & KX_SHADER_STAGE_FRAGMENT_BIT) {
                    if (current_pipeline_->fragment_shader()) {
                        current_pipeline_->fragment_shader()->push_constants = values;
                        current_pipeline_->fragment_shader()->push_constants_size = arg.size;
                    }
                }
                if (arg.stage_flags & KX_SHADER_STAGE_COMPUTE_BIT) {
                    if (current_pipeline_->compute_shader()) {
                        current_pipeline_->compute_shader()->push_constants = values;
                        current_pipeline_->compute_shader()->push_constants_size = arg.size;
                    }
                }
            }
        } else if constexpr (std::is_same_v<T, CmdBindDescriptorSets>) {
            execute_bind_descriptor_sets(arg);
        } else if constexpr (std::is_same_v<T, CmdDraw>) {
            execute_draw(arg);
        } else if constexpr (std::is_same_v<T, CmdDrawIndexed>) {
            execute_draw_indexed(arg);
        } else if constexpr (std::is_same_v<T, CmdDispatch>) {
            execute_dispatch(arg);
        } else if constexpr (std::is_same_v<T, CmdBeginRenderPass>) {
            execute_begin_render_pass(arg);
        } else if constexpr (std::is_same_v<T, CmdEndRenderPass>) {
            execute_end_render_pass(arg);
        } else if constexpr (std::is_same_v<T, CmdNextSubpass>) {
            execute_next_subpass(arg);
        }
        // Dynamic Rendering (VK_KHR_dynamic_rendering)
        else if constexpr (std::is_same_v<T, CmdBeginRendering>) {
            execute_begin_rendering(arg);
        } else if constexpr (std::is_same_v<T, CmdEndRendering>) {
            execute_end_rendering(arg);
        }
        // Synchronization 2 (VK_KHR_synchronization2)
        else if constexpr (std::is_same_v<T, CmdPipelineBarrier2>) {
            execute_pipeline_barrier2(arg);
        } else if constexpr (std::is_same_v<T, CmdWriteTimestamp2>) {
            execute_write_timestamp2(arg);
        } else if constexpr (std::is_same_v<T, CmdSetEvent2>) {
            execute_set_event2(arg);
        } else if constexpr (std::is_same_v<T, CmdResetEvent2>) {
            execute_reset_event2(arg);
        } else if constexpr (std::is_same_v<T, CmdWaitEvents2>) {
            execute_wait_events2(arg);
        }
        // Copy Commands v2 (VK_KHR_copy_commands2)
        else if constexpr (std::is_same_v<T, CmdCopyBuffer2>) {
            execute_copy_buffer2(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyImage2>) {
            execute_copy_image2(arg);
        } else if constexpr (std::is_same_v<T, CmdBlitImage2>) {
            execute_blit_image2(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyBufferToImage2>) {
            execute_copy_buffer_to_image2(arg);
        } else if constexpr (std::is_same_v<T, CmdCopyImageToBuffer2>) {
            execute_copy_image_to_buffer2(arg);
        } else if constexpr (std::is_same_v<T, CmdResolveImage2>) {
            execute_resolve_image2(arg);
        }
        // Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
        else if constexpr (std::is_same_v<T, CmdSetDepthClampEnable>) {
            execute_set_depth_clamp_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetPolygonMode>) {
            execute_set_polygon_mode(arg);
        } else if constexpr (std::is_same_v<T, CmdSetRasterizationSamples>) {
            execute_set_rasterization_samples(arg);
        } else if constexpr (std::is_same_v<T, CmdSetSampleMask>) {
            execute_set_sample_mask(arg);
        } else if constexpr (std::is_same_v<T, CmdSetAlphaToCoverageEnable>) {
            execute_set_alpha_to_coverage_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetAlphaToOneEnable>) {
            execute_set_alpha_to_one_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetLogicOpEnable>) {
            execute_set_logic_op_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetColorBlendEnable>) {
            execute_set_color_blend_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetColorBlendEquation>) {
            execute_set_color_blend_equation(arg);
        } else if constexpr (std::is_same_v<T, CmdSetColorWriteMask>) {
            execute_set_color_write_mask(arg);
        } else if constexpr (std::is_same_v<T, CmdSetTessellationDomainOrigin>) {
            execute_set_tessellation_domain_origin(arg);
        } else if constexpr (std::is_same_v<T, CmdSetRasterizationStream>) {
            execute_set_rasterization_stream(arg);
        } else if constexpr (std::is_same_v<T, CmdSetConservativeRasterizationMode>) {
            execute_set_conservative_rasterization_mode(arg);
        } else if constexpr (std::is_same_v<T, CmdSetExtraPrimitiveOverestimationSize>) {
            execute_set_extra_primitive_overestimation_size(arg);
        } else if constexpr (std::is_same_v<T, CmdSetDepthClipEnable>) {
            execute_set_depth_clip_enable(arg);
        } else if constexpr (std::is_same_v<T, CmdSetDepthClipNegativeOneToOne>) {
            execute_set_depth_clip_negative_one_to_one(arg);
        } else if constexpr (std::is_same_v<T, CmdSetViewportWScalingEnableNV>) {
            execute_set_viewport_w_scaling_enable_nv(arg);
        } else if constexpr (std::is_same_v<T, CmdSetViewportSwizzleNV>) {
            execute_set_viewport_swizzle_nv(arg);
        }
    }, cmd);
}

KxResult SoftwareDevice::begin_command_buffer(
    KxCommandBuffer command_buffer,
    const KxCommandBufferBeginInfo* begin_info) {
    if (!begin_info) return KX_ERROR_INVALID_ARGUMENT;
    if (begin_info->sType != KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

    cmd->begin();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::end_command_buffer(KxCommandBuffer command_buffer) {
    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

    cmd->end();
    return KX_SUCCESS;
}

KxResult SoftwareDevice::reset_command_buffer(KxCommandBuffer command_buffer,
                                              KxCommandBufferResetFlags flags) {
    (void)flags;
    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

    cmd->begin(); // Reset by beginning again
    return KX_SUCCESS;
}

void SoftwareDevice::cmd_begin_render_pass(KxCommandBuffer command_buffer,
                                           const KxRenderPassBeginInfo* render_pass_begin,
                                           KxSubpassContents contents) {
    if (!command_buffer || !render_pass_begin) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    auto* rp = find_render_pass(render_pass_begin->renderPass);
    auto* fb = find_framebuffer(render_pass_begin->framebuffer);
    if (!rp || !fb) return;

    std::vector<KxClearValue> clear_values;
    if (render_pass_begin->clearValueCount > 0 && render_pass_begin->pClearValues) {
        clear_values.assign(render_pass_begin->pClearValues,
                            render_pass_begin->pClearValues + render_pass_begin->clearValueCount);
    }

    cmd->record_begin_render_pass(rp, fb, render_pass_begin->renderArea,
                                  clear_values, contents);
}

void SoftwareDevice::cmd_end_render_pass(KxCommandBuffer command_buffer) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_end_render_pass();
}

void SoftwareDevice::cmd_next_subpass(KxCommandBuffer command_buffer,
                                      KxSubpassContents contents) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_next_subpass(contents);
}

// Dynamic Rendering (VK_KHR_dynamic_rendering)
void SoftwareDevice::cmd_begin_rendering(KxCommandBuffer command_buffer,
                                         const KxRenderingInfoKHR* rendering_info) {
    if (!command_buffer || !rendering_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_begin_rendering(rendering_info);
}

void SoftwareDevice::cmd_end_rendering(KxCommandBuffer command_buffer) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_end_rendering();
}

// Synchronization 2 (VK_KHR_synchronization2)
void SoftwareDevice::cmd_pipeline_barrier2(KxCommandBuffer command_buffer,
                                           const KxDependencyInfoKHR* dependency_info) {
    if (!command_buffer || !dependency_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_pipeline_barrier2(dependency_info);
}

void SoftwareDevice::cmd_write_timestamp2(KxCommandBuffer command_buffer,
                                          KxPipelineStageFlags2KHR stage,
                                          KxQueryPool query_pool,
                                          uint32_t query) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_write_timestamp2(stage, query_pool, query);
}

void SoftwareDevice::cmd_set_event2(KxCommandBuffer command_buffer,
                                    KxEvent event,
                                    const KxDependencyInfoKHR* dependency_info) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_event2(event, dependency_info);
}

void SoftwareDevice::cmd_reset_event2(KxCommandBuffer command_buffer,
                                      KxEvent event,
                                      KxPipelineStageFlags2KHR stage_mask) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_reset_event2(event, stage_mask);
}

void SoftwareDevice::cmd_wait_events2(KxCommandBuffer command_buffer,
                                      uint32_t event_count,
                                      const KxEvent* events,
                                      const KxDependencyInfoKHR* dependency_infos) {
    if (!command_buffer || !events || !dependency_infos) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_wait_events2(event_count, events, dependency_infos);
}

// Copy Commands v2 (VK_KHR_copy_commands2)
void SoftwareDevice::cmd_copy_buffer2(KxCommandBuffer command_buffer,
                                      const void* copy_buffer_info) {
    if (!command_buffer || !copy_buffer_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_copy_buffer2(copy_buffer_info);
}

void SoftwareDevice::cmd_copy_image2(KxCommandBuffer command_buffer,
                                     const void* copy_image_info) {
    if (!command_buffer || !copy_image_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_copy_image2(copy_image_info);
}

void SoftwareDevice::cmd_blit_image2(KxCommandBuffer command_buffer,
                                     const void* blit_image_info) {
    if (!command_buffer || !blit_image_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_blit_image2(blit_image_info);
}

void SoftwareDevice::cmd_copy_buffer_to_image2(KxCommandBuffer command_buffer,
                                               const void* copy_buffer_to_image_info) {
    if (!command_buffer || !copy_buffer_to_image_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_copy_buffer_to_image2(copy_buffer_to_image_info);
}

void SoftwareDevice::cmd_copy_image_to_buffer2(KxCommandBuffer command_buffer,
                                               const void* copy_image_to_buffer_info) {
    if (!command_buffer || !copy_image_to_buffer_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_copy_image_to_buffer2(copy_image_to_buffer_info);
}

void SoftwareDevice::cmd_resolve_image2(KxCommandBuffer command_buffer,
                                        const void* resolve_image_info) {
    if (!command_buffer || !resolve_image_info) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_resolve_image2(resolve_image_info);
}

// Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3)
void SoftwareDevice::cmd_set_depth_clamp_enable(KxCommandBuffer command_buffer,
                                                KxBool32 depth_clamp_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_depth_clamp_enable(depth_clamp_enable);
}

void SoftwareDevice::cmd_set_polygon_mode(KxCommandBuffer command_buffer,
                                          KxPolygonMode polygon_mode) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_polygon_mode(polygon_mode);
}

void SoftwareDevice::cmd_set_rasterization_samples(KxCommandBuffer command_buffer,
                                                   KxSampleCountFlagBits rasterization_samples) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_rasterization_samples(rasterization_samples);
}

void SoftwareDevice::cmd_set_sample_mask(KxCommandBuffer command_buffer,
                                         KxSampleMask sample_mask) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_sample_mask(sample_mask);
}

void SoftwareDevice::cmd_set_alpha_to_coverage_enable(KxCommandBuffer command_buffer,
                                                      KxBool32 alpha_to_coverage_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_alpha_to_coverage_enable(alpha_to_coverage_enable);
}

void SoftwareDevice::cmd_set_alpha_to_one_enable(KxCommandBuffer command_buffer,
                                                 KxBool32 alpha_to_one_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_alpha_to_one_enable(alpha_to_one_enable);
}

void SoftwareDevice::cmd_set_logic_op_enable(KxCommandBuffer command_buffer,
                                             KxBool32 logic_op_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_logic_op_enable(logic_op_enable);
}

void SoftwareDevice::cmd_set_color_blend_enable(KxCommandBuffer command_buffer,
                                                uint32_t first_attachment,
                                                uint32_t attachment_count,
                                                const KxBool32* color_blend_enables) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_color_blend_enable(first_attachment, attachment_count, color_blend_enables);
}

void SoftwareDevice::cmd_set_color_blend_equation(KxCommandBuffer command_buffer,
                                                  uint32_t first_attachment,
                                                  uint32_t attachment_count,
                                                  const void* color_blend_equations) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_color_blend_equation(first_attachment, attachment_count, color_blend_equations);
}

void SoftwareDevice::cmd_set_color_write_mask(KxCommandBuffer command_buffer,
                                              uint32_t first_attachment,
                                              uint32_t attachment_count,
                                              const KxColorComponentFlags* color_write_masks) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_color_write_mask(first_attachment, attachment_count, color_write_masks);
}

void SoftwareDevice::cmd_set_tessellation_domain_origin(KxCommandBuffer command_buffer,
                                                        KxTessellationDomainOriginKHR domain_origin) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_tessellation_domain_origin(domain_origin);
}

void SoftwareDevice::cmd_set_rasterization_stream(KxCommandBuffer command_buffer,
                                                  uint32_t rasterization_stream) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_rasterization_stream(rasterization_stream);
}

void SoftwareDevice::cmd_set_conservative_rasterization_mode(KxCommandBuffer command_buffer,
                                                             KxConservativeRasterizationModeEXT conservative_rasterization_mode) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_conservative_rasterization_mode(conservative_rasterization_mode);
}

void SoftwareDevice::cmd_set_extra_primitive_overestimation_size(KxCommandBuffer command_buffer,
                                                                 float extra_primitive_overestimation_size) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_extra_primitive_overestimation_size(extra_primitive_overestimation_size);
}

void SoftwareDevice::cmd_set_depth_clip_enable(KxCommandBuffer command_buffer,
                                               KxBool32 depth_clip_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_depth_clip_enable(depth_clip_enable);
}

void SoftwareDevice::cmd_set_depth_clip_negative_one_to_one(KxCommandBuffer command_buffer,
                                                            KxBool32 negative_one_to_one) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_depth_clip_negative_one_to_one(negative_one_to_one);
}

void SoftwareDevice::cmd_set_viewport_w_scaling_enable_nv(KxCommandBuffer command_buffer,
                                                          KxBool32 viewport_w_scaling_enable) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_viewport_w_scaling_enable_nv(viewport_w_scaling_enable);
}

void SoftwareDevice::cmd_set_viewport_swizzle_nv(KxCommandBuffer command_buffer,
                                                 uint32_t first_viewport,
                                                 uint32_t viewport_count,
                                                 const void* viewport_swizzles) {
    if (!command_buffer) return;

    auto* cmd = find_command_buffer(command_buffer);
    if (!cmd) return;

    cmd->record_set_viewport_swizzle_nv(first_viewport, viewport_count, viewport_swizzles);
}

KxResult SoftwareDevice::queue_submit(
    uint32_t submit_count,
    const KxSubmitInfo* submits,
    KxFence fence) {
    (void)fence;
    if (!submits || submit_count == 0) return KX_SUCCESS;

    for (uint32_t i = 0; i < submit_count; ++i) {
        const auto& submit = submits[i];
        if (submit.sType != KX_STRUCTURE_TYPE_SUBMIT_INFO) return KX_ERROR_INVALID_ARGUMENT;

        for (uint32_t j = 0; j < submit.commandBufferCount; ++j) {
            auto* cmd = find_command_buffer(submit.pCommandBuffers[j]);
            if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

            // Execute all commands in the command buffer
            for (const auto& command : cmd->commands()) {
                execute_command(command);
            }
        }
    }
    return KX_SUCCESS;
}

KxResult SoftwareDevice::device_wait_idle() {
    // For software driver, operations are synchronous, so wait_idle is a no-op
    return KX_SUCCESS;
}

KxResult SoftwareDevice::queue_wait_idle() {
    // For software driver, queue operations are synchronous
    return KX_SUCCESS;
}

void SoftwareDevice::execute_bind_descriptor_sets(const CmdBindDescriptorSets& cmd) {
    current_pipeline_layout_ = cmd.layout;
    current_first_descriptor_set_ = cmd.first_set;
    current_descriptor_set_count_ = cmd.descriptor_set_count;
    for (uint32_t i = 0; i < cmd.descriptor_set_count && i < 16; ++i) {
        current_descriptor_sets_[cmd.first_set + i] = cmd.sets[i];
    }
    // Dynamic offsets not implemented yet
    (void)cmd.dynamic_offset_count;
    (void)cmd.dynamic_offsets;
}

void SoftwareDevice::execute_dispatch(const CmdDispatch& cmd) {
    if (!current_pipeline_ || !current_pipeline_->compute_shader()) {
        return;
    }

    auto* compute_shader = current_pipeline_->compute_shader();

    // Collect bound descriptor sets
    std::vector<SoftwareDescriptorSet*> descriptor_sets;
    for (uint32_t i = 0; i < current_descriptor_set_count_ && i < 16; ++i) {
        KxDescriptorSet ds = current_descriptor_sets_[current_first_descriptor_set_ + i];
        if (ds) {
            auto* sw_ds = find_descriptor_set(ds);
            if (sw_ds) {
                descriptor_sets.push_back(sw_ds);
            }
        }
    }

    // Build compute dispatch info from pipeline
    ComputeDispatchInfo dispatch_info;
    dispatch_info.group_count_x = cmd.group_x;
    dispatch_info.group_count_y = cmd.group_y;
    dispatch_info.group_count_z = cmd.group_z;

    // Get local workgroup size from shader module (if available)
    // For now, use defaults - in a full impl this comes from the shader
    dispatch_info.local_size_x = 1;
    dispatch_info.local_size_y = 1;
    dispatch_info.local_size_z = 1;

    // Get push constants
    const void* push_constants = compute_shader->push_constants;
    uint32_t push_constants_size = compute_shader->push_constants_size;

    // Execute compute dispatch
    SoftwareRasterizerCompute compute_rasterizer(this);
    compute_rasterizer.dispatch(dispatch_info, compute_shader, descriptor_sets,
                                push_constants, push_constants_size);
}

// Dynamic Rendering (VK_KHR_dynamic_rendering) - stub implementations
void SoftwareDevice::execute_begin_rendering(const CmdBeginRendering&) {
    // No-op for software driver
}

void SoftwareDevice::execute_end_rendering(const CmdEndRendering&) {
    // No-op for software driver
}

// Synchronization 2 (VK_KHR_synchronization2) - stub implementations
void SoftwareDevice::execute_pipeline_barrier2(const CmdPipelineBarrier2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_write_timestamp2(const CmdWriteTimestamp2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_event2(const CmdSetEvent2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_reset_event2(const CmdResetEvent2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_wait_events2(const CmdWaitEvents2&) {
    // No-op for software driver
}

// Copy Commands v2 (VK_KHR_copy_commands2) - stub implementations
void SoftwareDevice::execute_copy_buffer2(const CmdCopyBuffer2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_copy_image2(const CmdCopyImage2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_blit_image2(const CmdBlitImage2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_copy_buffer_to_image2(const CmdCopyBufferToImage2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_copy_image_to_buffer2(const CmdCopyImageToBuffer2&) {
    // No-op for software driver
}

void SoftwareDevice::execute_resolve_image2(const CmdResolveImage2&) {
    // No-op for software driver
}

// Extended Dynamic State 3 (VK_EXT_extended_dynamic_state3) - stub implementations
void SoftwareDevice::execute_set_depth_clamp_enable(const CmdSetDepthClampEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_polygon_mode(const CmdSetPolygonMode&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_rasterization_samples(const CmdSetRasterizationSamples&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_sample_mask(const CmdSetSampleMask&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_alpha_to_coverage_enable(const CmdSetAlphaToCoverageEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_alpha_to_one_enable(const CmdSetAlphaToOneEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_logic_op_enable(const CmdSetLogicOpEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_color_blend_enable(const CmdSetColorBlendEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_color_blend_equation(const CmdSetColorBlendEquation&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_color_write_mask(const CmdSetColorWriteMask&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_tessellation_domain_origin(const CmdSetTessellationDomainOrigin&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_rasterization_stream(const CmdSetRasterizationStream&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_conservative_rasterization_mode(const CmdSetConservativeRasterizationMode&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_extra_primitive_overestimation_size(const CmdSetExtraPrimitiveOverestimationSize&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_depth_clip_enable(const CmdSetDepthClipEnable&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_depth_clip_negative_one_to_one(const CmdSetDepthClipNegativeOneToOne&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_viewport_w_scaling_enable_nv(const CmdSetViewportWScalingEnableNV&) {
    // No-op for software driver
}

void SoftwareDevice::execute_set_viewport_swizzle_nv(const CmdSetViewportSwizzleNV&) {
    // No-op for software driver
}

} /* namespace kx */