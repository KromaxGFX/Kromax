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
        } else if constexpr (std::is_same_v<T, CmdDraw>) {
            execute_draw(arg);
        } else if constexpr (std::is_same_v<T, CmdDrawIndexed>) {
            execute_draw_indexed(arg);
        } else if constexpr (std::is_same_v<T, CmdDispatch>) {
            // Dispatch - not implemented yet
            (void)arg;
        } else if constexpr (std::is_same_v<T, CmdBeginRenderPass>) {
            execute_begin_render_pass(arg);
        } else if constexpr (std::is_same_v<T, CmdEndRenderPass>) {
            execute_end_render_pass(arg);
        } else if constexpr (std::is_same_v<T, CmdNextSubpass>) {
            execute_next_subpass(arg);
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

} /* namespace kx */