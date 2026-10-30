/*
 * SoftwareRasterizerTriangle.cpp
 *
 * Triangle rasterization: edge functions, clipping, barycentric interpolation.
 */

#include "SoftwareRasterizer.h"

#include <algorithm>
#include <cmath>

namespace kx {

bool SoftwareRasterizer::clip_triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                       std::vector<Vertex>& output) const {
    output.push_back(v0);
    output.push_back(v1);
    output.push_back(v2);
    return true;
}

void SoftwareRasterizer::rasterize_triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                            uint32_t width, uint32_t height,
                                            std::vector<Fragment>& fragments) const {
    // Bounding box
    int min_x = std::max(0, static_cast<int>(std::floor(std::min({v0.position[0], v1.position[0], v2.position[0]}))));
    int max_x = std::min(static_cast<int>(width) - 1, static_cast<int>(std::ceil(std::max({v0.position[0], v1.position[0], v2.position[0]}))));
    int min_y = std::max(0, static_cast<int>(std::floor(std::min({v0.position[1], v1.position[1], v2.position[1]}))));
    int max_y = std::min(static_cast<int>(height) - 1, static_cast<int>(std::ceil(std::max({v0.position[1], v1.position[1], v2.position[1]}))));

    float area = edge_function(v0, v1, v2.position[0], v2.position[1]);
    if (area <= 0.0f) return;

    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            float fx = static_cast<float>(x);
            float fy = static_cast<float>(y);
            float w0 = edge_function(v1, v2, fx + 0.5f, fy + 0.5f) / area;
            float w1 = edge_function(v2, v0, fx + 0.5f, fy + 0.5f) / area;
            float w2 = edge_function(v0, v1, fx + 0.5f, fy + 0.5f) / area;

            if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                Fragment frag;
                frag.x = fx + 0.5f;
                frag.y = fy + 0.5f;
                interpolate_attributes(v0, v1, v2, w0, w1, w2, frag);
                fragments.push_back(frag);
            }
        }
    }
}

float SoftwareRasterizer::edge_function(const Vertex& a, const Vertex& b, float x, float y) {
    return (x - a.position[0]) * (b.position[1] - a.position[1]) -
           (y - a.position[1]) * (b.position[0] - a.position[0]);
}

void SoftwareRasterizer::interpolate_attributes(const Vertex& v0, const Vertex& v1, const Vertex& v2,
                                                float w0, float w1, float w2,
                                                Fragment& frag) {
    // Perspective-correct interpolation using 1/w
    float inv_w0 = v0.position[3];
    float inv_w1 = v1.position[3];
    float inv_w2 = v2.position[3];

    float denom = w0 * inv_w0 + w1 * inv_w1 + w2 * inv_w2;
    if (denom == 0.0f) denom = 1.0f;

    frag.depth = (w0 * v0.position[2] * inv_w0 + w1 * v1.position[2] * inv_w1 + w2 * v2.position[2] * inv_w2) / denom;

    frag.normal[0] = (w0 * v0.normal[0] * inv_w0 + w1 * v1.normal[0] * inv_w1 + w2 * v2.normal[0] * inv_w2) / denom;
    frag.normal[1] = (w0 * v0.normal[1] * inv_w0 + w1 * v1.normal[1] * inv_w1 + w2 * v2.normal[1] * inv_w2) / denom;
    frag.normal[2] = (w0 * v0.normal[2] * inv_w0 + w1 * v1.normal[2] * inv_w1 + w2 * v2.normal[2] * inv_w2) / denom;

    frag.texcoord[0] = (w0 * v0.texcoord[0] * inv_w0 + w1 * v1.texcoord[0] * inv_w1 + w2 * v2.texcoord[0] * inv_w2) / denom;
    frag.texcoord[1] = (w0 * v0.texcoord[1] * inv_w0 + w1 * v1.texcoord[1] * inv_w1 + w2 * v2.texcoord[1] * inv_w2) / denom;

    frag.color[0] = (w0 * v0.color[0] * inv_w0 + w1 * v1.color[0] * inv_w1 + w2 * v2.color[0] * inv_w2) / denom;
    frag.color[1] = (w0 * v0.color[1] * inv_w0 + w1 * v1.color[1] * inv_w1 + w2 * v2.color[1] * inv_w2) / denom;
    frag.color[2] = (w0 * v0.color[2] * inv_w0 + w1 * v1.color[2] * inv_w1 + w2 * v2.color[2] * inv_w2) / denom;
    frag.color[3] = (w0 * v0.color[3] * inv_w0 + w1 * v1.color[3] * inv_w1 + w2 * v2.color[3] * inv_w2) / denom;
}

} /* namespace kx */