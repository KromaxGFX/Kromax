#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZER_H

/*
 * SoftwareRasterizer.h
 *
 * Simple software rasterizer for the software driver.
 * Performs vertex processing, clipping, rasterization, and fragment shading.
 */

#include <cstdint>
#include <vector>

#include "kx_core.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareDescriptorSet.h"

namespace kx {

class SoftwareDevice;
class SoftwarePipeline;
class SoftwareShaderModule;
class SoftwareImage;
class SoftwareBuffer;

struct Vertex {
    float position[4];  // clip space
    float normal[3];
    float texcoord[2];
    float color[4];
};

struct Fragment {
    float x, y;
    float depth;
    float normal[3];
    float texcoord[2];
    float color[4];
};

// Vertex attribute fetcher - reads vertex attributes from buffers
struct VertexAttributeFetcher {
    const SoftwarePipeline* pipeline;
    const CmdBindVertexBuffers& vertex_buffers;
    const CmdBindIndexBuffer* index_buffer;  // null for non-indexed draw

    // Fetch vertex attributes for a given vertex index
    void fetch_attributes(uint32_t vertex_index, uint32_t instance_index, KxVertexInput& input) const;
};

class SoftwareRasterizer {
public:
    SoftwareRasterizer() = default;
    ~SoftwareRasterizer() = default;

    // Draw triangles using vertex buffers (non-indexed)
    void draw(SoftwareDevice* device,
              SoftwarePipeline* pipeline,
              const CmdBindVertexBuffers& vertex_buffers,
              const CmdBindIndexBuffer* index_buffer,
              uint32_t vertex_count,
              uint32_t instance_count,
              uint32_t first_vertex,
              uint32_t first_instance,
              SoftwareImage* color_attachment,
              SoftwareImage* depth_attachment);

    // Draw indexed triangles
    void draw_indexed(SoftwareDevice* device,
                      SoftwarePipeline* pipeline,
                      const CmdBindVertexBuffers& vertex_buffers,
                      const CmdBindIndexBuffer& index_buffer,
                      uint32_t index_count,
                      uint32_t instance_count,
                      uint32_t first_index,
                      int32_t vertex_offset,
                      uint32_t first_instance,
                      SoftwareImage* color_attachment,
                      SoftwareImage* depth_attachment);

private:
    // Vertex processing
    Vertex run_vertex_shader(SoftwareShaderModule* shader,
                             const KxVertexInput& input,
                             const void* push_constants) const;
    void perspective_divide(Vertex* vs_out, int count) const;
    void viewport_transform(Vertex* vs_out, int count,
                            uint32_t width, uint32_t height,
                            const SoftwarePipeline* pipeline) const;
    bool backface_cull(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                       const SoftwarePipeline* pipeline, bool& out_is_front_facing) const;

    // Fragment processing
    void run_fragment_shader(SoftwareShaderModule* shader,
                             const KxFragmentInput& input,
                             KxFragmentOutput* output,
                             const void* push_constants) const;
    bool depth_test(float frag_depth, float& depth_buffer,
                    KxCompareOp depth_compare, bool depth_write_enable) const;
    void write_color_attachment(uint8_t* color_data, uint32_t width, uint32_t height,
                                uint32_t x, uint32_t y,
                                const KxFragmentOutput& frag_out) const;
    void process_fragments(const std::vector<Fragment>& fragments,
                           uint32_t width, uint32_t height,
                           SoftwareImage* color_attachment,
                           SoftwareImage* depth_attachment,
                           const SoftwarePipeline* pipeline,
                           bool is_front_facing,
                           const void* push_constants,
                           const std::vector<KxBoundTexture>& bound_textures) const;

    // Triangle clipping against view frustum
    bool clip_triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                       std::vector<Vertex>& output) const;

    // Rasterize triangle to fragments
    void rasterize_triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                            uint32_t width, uint32_t height,
                            std::vector<Fragment>& fragments) const;

    // Edge function for barycentric coordinates
    static float edge_function(const Vertex& a, const Vertex& b, float x, float y);

    // Interpolate vertex attributes
    static void interpolate_attributes(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                       float w0, float w1, float w2,
                                       Fragment& frag);
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARERASTERIZER_H */