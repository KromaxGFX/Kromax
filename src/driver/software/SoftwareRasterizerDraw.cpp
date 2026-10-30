/*
 * SoftwareRasterizerDraw.cpp
 *
 * Main draw entry point: draw() orchestrating vertex/fragment/triangle processing.
 */

#include "SoftwareRasterizer.h"

#include "SoftwareDevice.h"
#include "SoftwarePipeline.h"
#include "SoftwareImage.h"
#include "SoftwareCommandBuffer.h"

namespace kx {

void SoftwareRasterizer::draw(SoftwareDevice* device,
                              SoftwarePipeline* pipeline,
                              const CmdBindVertexBuffers& vertex_buffers,
                              const CmdBindIndexBuffer* index_buffer,
                              uint32_t vertex_count,
                              uint32_t instance_count,
                              uint32_t first_vertex,
                              uint32_t first_instance,
                              SoftwareImage* color_attachment,
                              SoftwareImage* depth_attachment) {
    if (!pipeline || !color_attachment || vertex_count == 0) return;

    // Get framebuffer dimensions
    uint32_t width = color_attachment->width();
    uint32_t height = color_attachment->height();

    const void* push_constants = nullptr;
    if (pipeline->pipeline_layout() && pipeline->vertex_shader()) {
        push_constants = pipeline->vertex_shader()->push_constants;
    }

    // Process each instance
    for (uint32_t instance = 0; instance < instance_count; ++instance) {
        uint32_t instance_index = first_instance + instance;

        // Process vertices in groups of 3 (triangles)
        for (uint32_t v = 0; v + 2 < vertex_count; v += 3) {
            VertexAttributeFetcher fetcher{pipeline, vertex_buffers, index_buffer};

            // Fetch attributes for 3 vertices
            KxVertexInput vi[3];
            fetcher.fetch_attributes(first_vertex + v + 0, instance_index, vi[0]);
            fetcher.fetch_attributes(first_vertex + v + 1, instance_index, vi[1]);
            fetcher.fetch_attributes(first_vertex + v + 2, instance_index, vi[2]);

            // Run vertex shader for each vertex
            Vertex vs_out[3];
            for (int i = 0; i < 3; ++i) {
                vs_out[i] = run_vertex_shader(pipeline->vertex_shader(), vi[i], push_constants);
                
                // If no vertex shader, use pass-through
                if (!pipeline->vertex_shader() || !pipeline->vertex_shader()->vertex_callback) {
                    if (vi[i].position) {
                        vs_out[i].position[0] = vi[i].position[0];
                        vs_out[i].position[1] = vi[i].position[1];
                        vs_out[i].position[2] = vi[i].position[2] ? vi[i].position[2] : 0.0f;
                        vs_out[i].position[3] = vi[i].position[3] ? vi[i].position[3] : 1.0f;
                    } else {
                        vs_out[i].position[0] = 0.0f;
                        vs_out[i].position[1] = 0.0f;
                        vs_out[i].position[2] = 0.0f;
                        vs_out[i].position[3] = 1.0f;
                    }
                    if (vi[i].normal) {
                        vs_out[i].normal[0] = vi[i].normal[0];
                        vs_out[i].normal[1] = vi[i].normal[1];
                        vs_out[i].normal[2] = vi[i].normal[2];
                    } else {
                        vs_out[i].normal[0] = 0.0f;
                        vs_out[i].normal[1] = 0.0f;
                        vs_out[i].normal[2] = 1.0f;
                    }
                    if (vi[i].texcoord) {
                        vs_out[i].texcoord[0] = vi[i].texcoord[0];
                        vs_out[i].texcoord[1] = vi[i].texcoord[1];
                    } else {
                        vs_out[i].texcoord[0] = 0.0f;
                        vs_out[i].texcoord[1] = 0.0f;
                    }
                    if (vi[i].color) {
                        vs_out[i].color[0] = vi[i].color[0];
                        vs_out[i].color[1] = vi[i].color[1];
                        vs_out[i].color[2] = vi[i].color[2];
                        vs_out[i].color[3] = vi[i].color[3];
                    } else {
                        vs_out[i].color[0] = 1.0f;
                        vs_out[i].color[1] = 1.0f;
                        vs_out[i].color[2] = 1.0f;
                        vs_out[i].color[3] = 1.0f;
                    }
                }
            }

            // Perspective divide (convert from clip space to NDC)
            perspective_divide(vs_out, 3);

            // Viewport transform (NDC to screen space)
            viewport_transform(vs_out, 3, width, height, pipeline);

            // Backface culling
            bool is_front_facing;
            if (backface_cull(vs_out[0], vs_out[1], vs_out[2], pipeline, is_front_facing)) {
                continue;
            }

            // Rasterize
            std::vector<Fragment> fragments;
            rasterize_triangle(vs_out[0], vs_out[1], vs_out[2], width, height, fragments);

            // Process fragments
            process_fragments(fragments, width, height, color_attachment, depth_attachment,
                             pipeline, is_front_facing, push_constants);
        }
    }
}

} /* namespace kx */