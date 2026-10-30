/*
 * SoftwareRasterizerFragment.cpp
 *
 * Fragment processing: depth testing, fragment shader execution, color output.
 */

#include "SoftwareRasterizer.h"

#include <algorithm>

#include "SoftwareShaderModule.h"
#include "SoftwarePipeline.h"
#include "SoftwareImage.h"

namespace kx {

void SoftwareRasterizer::run_fragment_shader(SoftwareShaderModule* shader,
                                             const KxFragmentInput& input,
                                             KxFragmentOutput* output,
                                             const void* push_constants) const {
    if (shader && shader->fragment_callback) {
        shader->fragment_callback(&input, output, push_constants);
    }
}

bool SoftwareRasterizer::depth_test(float frag_depth, float& depth_buffer,
                                    KxCompareOp depth_compare, bool depth_write_enable) const {
    bool pass = false;
    switch (depth_compare) {
        case KX_COMPARE_OP_LESS:
            pass = frag_depth < depth_buffer;
            break;
        case KX_COMPARE_OP_LESS_OR_EQUAL:
            pass = frag_depth <= depth_buffer;
            break;
        case KX_COMPARE_OP_GREATER:
            pass = frag_depth > depth_buffer;
            break;
        case KX_COMPARE_OP_GREATER_OR_EQUAL:
            pass = frag_depth >= depth_buffer;
            break;
        case KX_COMPARE_OP_EQUAL:
            pass = frag_depth == depth_buffer;
            break;
        case KX_COMPARE_OP_NOT_EQUAL:
            pass = frag_depth != depth_buffer;
            break;
        case KX_COMPARE_OP_NEVER:
            pass = false;
            break;
        case KX_COMPARE_OP_ALWAYS:
        default:
            pass = true;
            break;
    }
    
    if (pass && depth_write_enable) {
        depth_buffer = frag_depth;
    }
    return pass;
}

void SoftwareRasterizer::write_color_attachment(uint8_t* color_data, uint32_t width, uint32_t height,
                                                uint32_t x, uint32_t y,
                                                const KxFragmentOutput& frag_out) const {
    if (x >= width || y >= height) return;
    uint8_t* pixel = color_data + (y * width + x) * 4;
    pixel[0] = static_cast<uint8_t>(std::clamp(frag_out.color[0], 0.0f, 1.0f) * 255.0f);
    pixel[1] = static_cast<uint8_t>(std::clamp(frag_out.color[1], 0.0f, 1.0f) * 255.0f);
    pixel[2] = static_cast<uint8_t>(std::clamp(frag_out.color[2], 0.0f, 1.0f) * 255.0f);
    pixel[3] = static_cast<uint8_t>(std::clamp(frag_out.color[3], 0.0f, 1.0f) * 255.0f);
}

void SoftwareRasterizer::process_fragments(const std::vector<Fragment>& fragments,
                                           uint32_t width, uint32_t height,
                                           SoftwareImage* color_attachment,
                                           SoftwareImage* depth_attachment,
                                           const SoftwarePipeline* pipeline,
                                           bool is_front_facing,
                                           const void* push_constants,
                                           const std::vector<KxBoundTexture>& bound_textures) const {
    uint8_t* color_data = color_attachment->data();
    float* depth_data = depth_attachment ? reinterpret_cast<float*>(depth_attachment->data()) : nullptr;
    bool depth_test_enable = pipeline->depth_test_enable();
    bool depth_write_enable = pipeline->depth_write_enable();
    KxCompareOp depth_compare = pipeline->depth_compare_op();

    // Get viewport and scissor state from pipeline
    const auto* rs_state = pipeline->rasterization_state();
    const auto* vp_state = pipeline->viewport_state();
    KxRect2D scissor_rect{};
    bool has_scissor = false;
    if (vp_state && rs_state) {
        if (vp_state->scissorCount > 0 && vp_state->pScissors && vp_state->pScissors[0].extent.width > 0 && vp_state->pScissors[0].extent.height > 0) {
            scissor_rect = vp_state->pScissors[0];
            has_scissor = true;
        }
    }

    for (const auto& frag : fragments) {
        // Make a mutable copy of the fragment for modification
        Fragment frag_mod = frag;
        // Apply scissor rectangle clipping
        uint32_t x = static_cast<uint32_t>(frag_mod.x);
        uint32_t y = static_cast<uint32_t>(frag_mod.y);
        if (has_scissor) {
            int32_t x_min = scissor_rect.offset.x;
            int32_t y_min = scissor_rect.offset.y;
            int32_t x_max = x_min + static_cast<int32_t>(scissor_rect.extent.width);
            int32_t y_max = y_min + static_cast<int32_t>(scissor_rect.extent.height);
            if (x < static_cast<uint32_t>(x_min) || x >= x_max ||
                y < static_cast<uint32_t>(y_min) || y >= y_max) {
                continue; // Outside scissor rectangle
            }
            // Adjust coordinates relative to scissor rectangle
            x = x - static_cast<uint32_t>(x_min);
            y = y - static_cast<uint32_t>(y_min);
        }
        if (x >= width || y >= height) continue;

        // Depth test
        if (depth_data && depth_test_enable) {
            float& depth_buffer = depth_data[y * width + x];
            if (!depth_test(frag_mod.depth, depth_buffer, depth_compare, depth_write_enable)) {
                continue;
            }
        }

        // Run fragment shader
        KxFragmentOutput frag_out{};
        if (pipeline->fragment_shader() && pipeline->fragment_shader()->fragment_callback) {
            KxFragmentInput frag_in{};
            frag_in.frag_coord[0] = frag_mod.x;
            frag_in.frag_coord[1] = frag_mod.y;
            frag_in.frag_coord[2] = frag_mod.depth;
            frag_in.frag_coord[3] = 1.0f;
            frag_in.front_facing = is_front_facing ? 1.0f : 0.0f;
            frag_in.normal[0] = frag_mod.normal[0];
            frag_in.normal[1] = frag_mod.normal[1];
            frag_in.normal[2] = frag_mod.normal[2];
            frag_in.texcoord[0] = frag_mod.texcoord[0];
            frag_in.texcoord[1] = frag_mod.texcoord[1];
            frag_in.color[0] = frag_mod.color[0];
            frag_in.color[1] = frag_mod.color[1];
            frag_in.color[2] = frag_mod.color[2];
            frag_in.color[3] = frag_mod.color[3];
            frag_in.boundTextureCount = static_cast<uint32_t>(bound_textures.size());
            frag_in.pBoundTextures = bound_textures.data();

            run_fragment_shader(pipeline->fragment_shader(), frag_in, &frag_out, push_constants);
        } else {
            // Default: output vertex color
            frag_out.color[0] = frag_mod.color[0];
            frag_out.color[1] = frag_mod.color[1];
            frag_out.color[2] = frag_mod.color[2];
            frag_out.color[3] = frag_mod.color[3];
            frag_out.depth = frag_mod.depth;
        }

        // Apply pipeline blend state
        const auto* blender = pipeline->color_blend_state();
        if (blender && blender->attachmentCount > 0 && blender->pAttachments) {
            const auto& attachment = blender->pAttachments[0];
            if (attachment.blendEnable) {
                // Simple source over destination blend
                // Assume destination is cleared to (0,0,0,0) for simplicity
                float src_a = frag_out.color[3];
                float dst_a = 1.0f; // Destination alpha (from previous frame or clear)

                // Blend each color channel
                for (int i = 0; i < 4; ++i) {
                    frag_out.color[i] = frag_out.color[i] * src_a + 0.0f * dst_a;
                }
                frag_out.color[3] = src_a;
            }
        }

        // Write to color attachment (assume RGBA8)
        write_color_attachment(color_data, width, height, x, y, frag_out);
    }
}

} /* namespace kx */