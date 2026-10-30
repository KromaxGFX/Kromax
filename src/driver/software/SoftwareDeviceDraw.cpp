/*
 * SoftwareDeviceDraw.cpp
 *
 * Draw command execution (draw/draw_indexed).
 */

#include "SoftwareDevice.h"

#include "SoftwareCommandBuffer.h"
#include "SoftwarePipeline.h"
#include "SoftwareRasterizer.h"
#include "SoftwareFramebuffer.h"

namespace kx {

void SoftwareDevice::execute_draw(const CmdDraw& cmd) {
    if (!current_pipeline_) return;
    if (cmd.vertex_count == 0) return;
    if (!current_framebuffer_) return;

    // Get color attachment (first color attachment)
    SoftwareImage* color_attachment = nullptr;
    if (!current_framebuffer_->color_attachments().empty()) {
        color_attachment = current_framebuffer_->color_attachments()[0];
    }

    // Get depth attachment
    SoftwareImage* depth_attachment = current_framebuffer_->depth_attachment();

    if (!color_attachment) return;

    // Create rasterizer and draw
    SoftwareRasterizer rasterizer;
    rasterizer.draw(this, current_pipeline_,
                    current_vertex_buffers_, &current_index_buffer_,
                    cmd.vertex_count, cmd.instance_count,
                    cmd.first_vertex, cmd.first_instance,
                    color_attachment, depth_attachment);
}

void SoftwareDevice::execute_draw_indexed(const CmdDrawIndexed& cmd) {
    if (!current_pipeline_) return;
    if (cmd.index_count == 0) return;
    if (!current_framebuffer_) return;
    if (!current_index_buffer_.buffer) return;

    // Get color attachment (first color attachment)
    SoftwareImage* color_attachment = nullptr;
    if (!current_framebuffer_->color_attachments().empty()) {
        color_attachment = current_framebuffer_->color_attachments()[0];
    }

    // Get depth attachment
    SoftwareImage* depth_attachment = current_framebuffer_->depth_attachment();

    if (!color_attachment) return;

    // Create rasterizer and draw indexed
    SoftwareRasterizer rasterizer;
    rasterizer.draw_indexed(this, current_pipeline_,
                            current_vertex_buffers_, current_index_buffer_,
                            cmd.index_count, cmd.instance_count,
                            cmd.first_index, cmd.vertex_offset,
                            cmd.first_instance,
                            color_attachment, depth_attachment);
}

} /* namespace kx */