/*
 * SoftwareRasterizerVertex.cpp
 *
 * Vertex processing: attribute fetching, vertex shader execution, perspective divide, viewport transform.
 */

#include "SoftwareRasterizer.h"

#include <algorithm>

#include "SoftwareShaderModule.h"
#include "SoftwarePipeline.h"
#include "SoftwareBuffer.h"
#include "../shader/ShaderInterpreter.h"

namespace kx {

void VertexAttributeFetcher::fetch_attributes(uint32_t vertex_index, uint32_t instance_index, KxVertexInput& input) const {
    // Initialize all pointers to null
    input.position = nullptr;
    input.normal = nullptr;
    input.texcoord = nullptr;
    input.color = nullptr;
    input.instance_index = instance_index;
    input.vertex_index = vertex_index;
    input.environment = nullptr;

    if (!pipeline || !pipeline->vertex_input_state()) return;

    const auto* vi_state = pipeline->vertex_input_state();
    if (!vi_state->pVertexBindingDescriptions || !vi_state->pVertexAttributeDescriptions) return;

    // For each attribute description, fetch data from the corresponding binding
    for (uint32_t attr_idx = 0; attr_idx < vi_state->vertexAttributeDescriptionCount; ++attr_idx) {
        const auto& attr = vi_state->pVertexAttributeDescriptions[attr_idx];
        
        if (attr.binding >= vertex_buffers.binding_count) continue;
        
        const auto* buffer = vertex_buffers.buffers[attr.binding];
        if (!buffer) continue;

        uint64_t offset = vertex_buffers.offsets[attr.binding];
        const KxVertexInputBindingDescription* binding_desc = nullptr;
        
        // Find the binding description
        for (uint32_t b = 0; b < vi_state->vertexBindingDescriptionCount; ++b) {
            if (vi_state->pVertexBindingDescriptions[b].binding == attr.binding) {
                binding_desc = &vi_state->pVertexBindingDescriptions[b];
                break;
            }
        }
        
        if (!binding_desc) continue;

        uint32_t stride = binding_desc->stride;
        uint32_t input_rate = binding_desc->inputRate;
        uint32_t effective_vertex_index = (input_rate == KX_VERTEX_INPUT_RATE_INSTANCE) ? instance_index : vertex_index;

        // Calculate byte offset into buffer
        uint64_t attr_offset = offset + static_cast<uint64_t>(effective_vertex_index) * stride + attr.offset;
        const float* attr_ptr = reinterpret_cast<const float*>(buffer->mapped_ptr() + attr_offset);

        // Assign to appropriate field based on location
        switch (attr.location) {
            case 0: input.position = attr_ptr; break;  // position
            case 1: input.normal = attr_ptr; break;    // normal
            case 2: input.texcoord = attr_ptr; break;  // texcoord
            case 3: input.color = attr_ptr; break;     // color
            default: break;
        }
    }
}

static void run_vertex_shader_ir(const shader::Interpreter& interpreter,
                                 const shader::ShaderEnvironment& env,
                                 const KxVertexInput& input,
                                 Vertex& out) {
    // The interpreter would be called here with the vertex input as arguments
    // For now, we use a simplified path - the full interpreter integration
    // would require building a proper binding between the IR inputs and the
    // vertex attribute data. This is a placeholder.
    (void)interpreter;
    (void)env;
    (void)input;
    (void)out;
}

// Forward declarations for static functions
static void build_vertex_shader_environment(SoftwareShaderModule* shader,
                                            const KxVertexInput& input,
                                            const void* push_constants,
                                            shader::ShaderEnvironment& env);

static void extract_vertex_outputs(SoftwareShaderModule* shader,
                                   const shader::Value& result,
                                   Vertex& out);

Vertex SoftwareRasterizer::run_vertex_shader(SoftwareShaderModule* shader,
                                             const KxVertexInput& input,
                                             const void* push_constants) const {
    Vertex out{};
    if (shader && shader->vertex_callback) {
        KxVertexOutput vs_out{};
        shader->vertex_callback(&input, &vs_out, push_constants);
        out.position[0] = vs_out.position[0];
        out.position[1] = vs_out.position[1];
        out.position[2] = vs_out.position[2];
        out.position[3] = vs_out.position[3];
        out.normal[0] = vs_out.normal[0];
        out.normal[1] = vs_out.normal[1];
        out.normal[2] = vs_out.normal[2];
        out.texcoord[0] = vs_out.texcoord[0];
        out.texcoord[1] = vs_out.texcoord[1];
        out.color[0] = vs_out.color[0];
        out.color[1] = vs_out.color[1];
        out.color[2] = vs_out.color[2];
        out.color[3] = vs_out.color[3];
    } else if (shader && shader->has_ir() && shader->interpreter) {
        // Run through IR interpreter
        shader::ShaderEnvironment env;
        build_vertex_shader_environment(shader, input, push_constants, env);
        
        shader::Value result;
        shader->interpreter->execute(env, nullptr, &result);
        
        // Extract outputs from result
        extract_vertex_outputs(shader, result, out);
    }
    return out;
}

// Build shader environment from bound resources
static void build_vertex_shader_environment(SoftwareShaderModule* shader,
                                            const KxVertexInput& input,
                                            const void* push_constants,
                                            shader::ShaderEnvironment& env) {
    // Uniform blocks and push constants would be set up here
    // For now, we pass push constants
    if (push_constants) {
        env.push_constants = push_constants;
        env.push_constants_size = 256; // Default size, would need proper tracking
    }
}

// Extract vertex outputs from interpreter result
static void extract_vertex_outputs(SoftwareShaderModule* shader,
                                   const shader::Value& result,
                                   Vertex& out) {
    // In a full implementation, we'd map the interpreter outputs to the Vertex struct
    // based on the shader's output variables. For now, use defaults.
    out.position[0] = 0.0f;
    out.position[1] = 0.0f;
    out.position[2] = 0.0f;
    out.position[3] = 1.0f;
    out.normal[0] = 0.0f;
    out.normal[1] = 0.0f;
    out.normal[2] = 1.0f;
    out.texcoord[0] = 0.0f;
    out.texcoord[1] = 0.0f;
    out.color[0] = 1.0f;
    out.color[1] = 1.0f;
    out.color[2] = 1.0f;
    out.color[3] = 1.0f;
}

void SoftwareRasterizer::perspective_divide(Vertex* vs_out, int count) const {
    for (int i = 0; i < count; ++i) {
        if (vs_out[i].position[3] != 0.0f) {
            float w = 1.0f / vs_out[i].position[3];
            vs_out[i].position[0] *= w;
            vs_out[i].position[1] *= w;
            vs_out[i].position[2] *= w;
            vs_out[i].position[3] = w;
        }
    }
}

void SoftwareRasterizer::viewport_transform(Vertex* vs_out, int count,
                                            uint32_t width, uint32_t height,
                                            const SoftwarePipeline* pipeline) const {
    // Use pipeline viewport if available, else default to full framebuffer
    float vp_x = 0, vp_y = 0, vp_width = width, vp_height = height;
    float vp_min_depth = 0.0f, vp_max_depth = 1.0f;
    
    if (pipeline->viewport_state() && pipeline->viewport_state()->pViewports) {
        const auto& vp = pipeline->viewport_state()->pViewports[0];
        vp_x = vp.x;
        vp_y = vp.y;
        vp_width = vp.width;
        vp_height = vp.height;
        vp_min_depth = vp.minDepth;
        vp_max_depth = vp.maxDepth;
    }

    for (int i = 0; i < count; ++i) {
        vs_out[i].position[0] = vp_x + (vs_out[i].position[0] * 0.5f + 0.5f) * vp_width;
        vs_out[i].position[1] = vp_y + (vs_out[i].position[1] * 0.5f + 0.5f) * vp_height;
        vs_out[i].position[2] = vs_out[i].position[2] * (vp_max_depth - vp_min_depth) + vp_min_depth;
    }
}

bool SoftwareRasterizer::backface_cull(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                       const SoftwarePipeline* pipeline, bool& out_is_front_facing) const {
    const auto* rs = pipeline->rasterization_state();
    KxCullModeFlags cull_mode = KX_CULL_MODE_BACK_BIT;
    KxFrontFace front_face = KX_FRONT_FACE_COUNTER_CLOCKWISE;
    
    if (rs) {
        cull_mode = rs->cullMode;
        front_face = rs->frontFace;
    }

    float area = edge_function(v0, v1, v2.position[0], v2.position[1]);
    
    bool is_front_facing = (area > 0.0f);
    if (front_face == KX_FRONT_FACE_CLOCKWISE) {
        is_front_facing = !is_front_facing;
    }
    out_is_front_facing = is_front_facing;

    bool cull = false;
    if (cull_mode & KX_CULL_MODE_FRONT_BIT && is_front_facing) cull = true;
    if (cull_mode & KX_CULL_MODE_BACK_BIT && !is_front_facing) cull = true;
    if (area == 0.0f) cull = true;  // Degenerate
    
    return cull;
}

} /* namespace kx */