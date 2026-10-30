/*
 * SoftwareDevicePipeline.cpp
 *
 * Pipeline and pipeline layout creation/destruction.
 */

#include "SoftwareDevice.h"

#include "SoftwarePipeline.h"
#include "SoftwarePipelineLayout.h"

namespace kx {

/* Pipeline Layout */

KxResult SoftwareDevice::create_pipeline_layout(
    const KxPipelineLayoutCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxPipelineLayout* pipeline_layout) {
    (void)allocator;
    if (!pipeline_layout) return KX_ERROR_INVALID_ARGUMENT;
    *pipeline_layout = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    auto layout = std::make_unique<SoftwarePipelineLayout>();
    layout->set_create_info(create_info);
    *pipeline_layout = register_pipeline_layout(std::move(layout));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_pipeline_layout(
    KxPipelineLayout pipeline_layout,
    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_pipeline_layout(pipeline_layout);
}

/* Pipeline */

KxResult SoftwareDevice::create_pipeline(
    const KxPipelineCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxPipeline* pipeline) {
    (void)allocator;
    if (!pipeline) return KX_ERROR_INVALID_ARGUMENT;
    *pipeline = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    // Find pipeline layout
    auto* layout = find_pipeline_layout(create_info->layout);
    if (!layout) return KX_ERROR_INVALID_ARGUMENT;

    // Find shader modules
    SoftwareShaderModule* vertex_shader = nullptr;
    SoftwareShaderModule* fragment_shader = nullptr;
    SoftwareShaderModule* compute_shader = nullptr;

    if (create_info->vertexShaderModule) {
        vertex_shader = find_shader_module(create_info->vertexShaderModule);
        if (!vertex_shader) return KX_ERROR_INVALID_ARGUMENT;
    }
    if (create_info->fragmentShaderModule) {
        fragment_shader = find_shader_module(create_info->fragmentShaderModule);
        if (!fragment_shader) return KX_ERROR_INVALID_ARGUMENT;
    }
    if (create_info->computeShaderModule) {
        compute_shader = find_shader_module(create_info->computeShaderModule);
        if (!compute_shader) return KX_ERROR_INVALID_ARGUMENT;
    }

    // Create pipeline
    auto pipe = std::make_unique<SoftwarePipeline>();
    pipe->set_create_info(create_info);
    pipe->set_pipeline_layout(layout);
    if (vertex_shader) pipe->set_vertex_shader(vertex_shader);
    if (fragment_shader) pipe->set_fragment_shader(fragment_shader);
    if (compute_shader) pipe->set_compute_shader(compute_shader);

    *pipeline = register_pipeline(std::move(pipe));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_pipeline(
    KxPipeline pipeline,
    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_pipeline(pipeline);
}

} /* namespace kx */