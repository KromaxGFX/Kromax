/*
 * SoftwareDeviceRenderPass.cpp
 *
 * Render pass execution (begin/end/next subpass).
 */

#include "SoftwareDevice.h"

#include <cstring>

#include "SoftwareCommandBuffer.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"

namespace kx {

void SoftwareDevice::execute_begin_render_pass(const CmdBeginRenderPass& cmd) {
    current_render_pass_ = cmd.render_pass;
    current_framebuffer_ = cmd.framebuffer;
    current_subpass_ = 0;

    // Clear attachments if clear values provided
    if (cmd.framebuffer && cmd.framebuffer->color_attachments().size() > 0) {
        // Clear color attachments
        for (size_t i = 0; i < cmd.framebuffer->color_attachments().size() && i < cmd.clear_values.size(); ++i) {
            auto* img = cmd.framebuffer->color_attachments()[i];
            if (img) {
                const auto& clear_val = cmd.clear_values[i];
                uint8_t* data = img->data();
                if (data) {
                    uint32_t width = img->width();
                    uint32_t height = img->height();
                    for (uint32_t y = 0; y < height; ++y) {
                        for (uint32_t x = 0; x < width; ++x) {
                            uint8_t* pixel = data + (y * width + x) * 4;
                            pixel[0] = static_cast<uint8_t>(clear_val.color[0] * 255.0f);
                            pixel[1] = static_cast<uint8_t>(clear_val.color[1] * 255.0f);
                            pixel[2] = static_cast<uint8_t>(clear_val.color[2] * 255.0f);
                            pixel[3] = static_cast<uint8_t>(clear_val.color[3] * 255.0f);
                        }
                    }
                }
            }
        }
    }

    // Clear depth attachment
    if (cmd.framebuffer && cmd.framebuffer->depth_attachment()) {
        auto* img = cmd.framebuffer->depth_attachment();
        if (img) {
            const auto& clear_val = cmd.clear_values.size() > cmd.framebuffer->color_attachments().size() 
                ? cmd.clear_values[cmd.framebuffer->color_attachments().size()]
                : cmd.clear_values[0];
            float* depth_data = reinterpret_cast<float*>(img->data());
            if (depth_data) {
                uint32_t width = img->width();
                uint32_t height = img->height();
                for (uint32_t y = 0; y < height; ++y) {
                    for (uint32_t x = 0; x < width; ++x) {
                        depth_data[y * width + x] = clear_val.depthStencil.depth;
                    }
                }
            }
        }
    }
}

void SoftwareDevice::execute_end_render_pass(const CmdEndRenderPass& cmd) {
    (void)cmd;
    current_render_pass_ = nullptr;
    current_framebuffer_ = nullptr;
    current_subpass_ = 0;
}

void SoftwareDevice::execute_next_subpass(const CmdNextSubpass& cmd) {
    (void)cmd;
    // Subpass transition handled by command buffer state
}

} /* namespace kx */