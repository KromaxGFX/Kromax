/*
 * SoftwareDeviceCopy.cpp
 *
 * Clear and copy operations.
 */

#include "SoftwareDevice.h"

#include <cstring>

#include "SoftwareCommandBuffer.h"
#include "SoftwareImage.h"
#include "SoftwareBuffer.h"

namespace kx {

void SoftwareDevice::execute_clear_image(const CmdClearImage& cmd) {
    if (!cmd.image) return;
    uint8_t* data = cmd.image->data();
    if (!data) return;

    uint32_t width = cmd.image->width();
    uint32_t height = cmd.image->height();
    uint32_t pixel_size = 4; // RGBA8 default

    // Simple clear - fill with color
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t* pixel = data + (y * width + x) * pixel_size;
            pixel[0] = static_cast<uint8_t>(cmd.color[0] * 255.0f);
            pixel[1] = static_cast<uint8_t>(cmd.color[1] * 255.0f);
            pixel[2] = static_cast<uint8_t>(cmd.color[2] * 255.0f);
            pixel[3] = static_cast<uint8_t>(cmd.color[3] * 255.0f);
        }
    }
}

void SoftwareDevice::execute_copy_buffer(const CmdCopyBuffer& cmd) {
    if (!cmd.src || !cmd.dst) return;
    if (cmd.size == 0) return;

    uint8_t* src_ptr = cmd.src->mapped_ptr() + cmd.src_offset;
    uint8_t* dst_ptr = cmd.dst->mapped_ptr() + cmd.dst_offset;

    if (src_ptr && dst_ptr) {
        std::memcpy(dst_ptr, src_ptr, static_cast<size_t>(cmd.size));
    }
}

void SoftwareDevice::execute_copy_buffer_to_image(const CmdCopyBufferToImage& cmd) {
    if (!cmd.src || !cmd.dst) return;
    if (cmd.dst_width == 0 || cmd.dst_height == 0) return;

    uint8_t* src_ptr = cmd.src->mapped_ptr() + cmd.src_offset;
    uint8_t* dst_ptr = cmd.dst->data();

    if (src_ptr && dst_ptr) {
        // Simple row-by-row copy assuming tightly packed
        uint32_t pixel_size = 4;
        for (uint32_t y = 0; y < cmd.dst_height; ++y) {
            std::memcpy(dst_ptr + y * cmd.dst_width * pixel_size,
                        src_ptr + y * cmd.dst_width * pixel_size,
                        cmd.dst_width * pixel_size);
        }
    }
}

} /* namespace kx */