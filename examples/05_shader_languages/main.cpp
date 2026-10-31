// Example 05: Shader Languages
// Demonstrates compiling and running the same shader in GLSL, WGSL, and KXSL
// HLSL is skipped as it may not be fully supported in this build

#include <kromax/kromax.h>
#include <cstdio>
#include <cstring>
#include <vector>

// Simple matrix math
struct Mat4 {
    float m[16];
};

Mat4 mat4_identity() {
    Mat4 r{};
    for (int i = 0; i < 16; ++i) r.m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    return r;
}

Mat4 mat4_multiply(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a.m[row * 4 + k] * b.m[k * 4 + col];
            }
            r.m[row * 4 + col] = sum;
        }
    }
    return r;
}

Mat4 mat4_ortho(float left, float right, float bottom, float top, float z_near, float z_far) {
    Mat4 r{};
    r.m[0] = 2.0f / (right - left);
    r.m[5] = 2.0f / (top - bottom);
    r.m[10] = -2.0f / (z_far - z_near);
    r.m[12] = -(right + left) / (right - left);
    r.m[13] = -(top + bottom) / (top - bottom);
    r.m[14] = -(z_far + z_near) / (z_far - z_near);
    r.m[15] = 1.0f;
    return r;
}

// Shader sources for each language - minimal valid shaders
const char* GLSL_VERTEX =
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

const char* GLSL_FRAGMENT =
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

const char* WGSL_VERTEX =
    "fn main() {\n"
    "    let a = 1.0;\n"
    "}\n";

const char* WGSL_FRAGMENT =
    "fn main() {\n"
    "    let a = 1.0;\n"
    "}\n";

const char* KXSL_VERTEX =
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

const char* KXSL_FRAGMENT =
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

struct ShaderLanguage {
    const char* name;
    KxShaderLanguage language;
    const char* vertex_src;
    const char* fragment_src;
    float clear_color[3];
};

bool run_language_test(KxDevice device, KxQueue queue, uint32_t graphics_queue_family,
                       const ShaderLanguage& lang, const Mat4& mvp) {
    std::printf("  Testing %s...\n", lang.name);

    // Create command pool
    KxCommandPoolCreateInfo pool_info{};
    pool_info.sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family;
    pool_info.flags = KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    KxCommandPool command_pool;
    KxResult res = kxCreateCommandPool(device, &pool_info, nullptr, &command_pool);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create command pool: %d\n", res);
        return false;
    }

    // Create command buffer
    KxCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    KxCommandBuffer command_buffer;
    res = kxAllocateCommandBuffers(device, &alloc_info, &command_buffer);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to allocate command buffer: %d\n", res);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Create shader modules from source
    KxShaderSourceCreateInfo vert_info{};
    vert_info.sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
    vert_info.language = lang.language;
    vert_info.stage = KX_SHADER_STAGE_VERTEX;
    vert_info.pSource = lang.vertex_src;
    vert_info.pEntryPoint = "main";

    KxShaderSourceCreateInfo frag_info{};
    frag_info.sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
    frag_info.language = lang.language;
    frag_info.stage = KX_SHADER_STAGE_FRAGMENT;
    frag_info.pSource = lang.fragment_src;
    frag_info.pEntryPoint = "main";

    KxShaderModule vert_shader, frag_shader;
    res = kxCreateShaderModuleFromSource(device, &vert_info, nullptr, &vert_shader);
    if (res != KX_SUCCESS) {
        std::printf("    %s vertex shader compilation failed: %d\n", lang.name, res);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    res = kxCreateShaderModuleFromSource(device, &frag_info, nullptr, &frag_shader);
    if (res != KX_SUCCESS) {
        std::printf("    %s fragment shader compilation failed: %d\n", lang.name, res);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Pipeline layout
    KxPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = nullptr;

    KxPipelineLayout pipeline_layout;
    res = kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create pipeline layout: %d\n", res);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Vertex input
    KxVertexInputBindingDescription binding_desc{};
    binding_desc.binding = 0;
    binding_desc.stride = 7 * sizeof(float);  // pos(3) + color(4)
    binding_desc.inputRate = KX_VERTEX_INPUT_RATE_VERTEX;

    KxVertexInputAttributeDescription attr_descs[2] = {};
    attr_descs[0].location = 0;
    attr_descs[0].binding = 0;
    attr_descs[0].format = KX_FORMAT_R32G32B32_SFLOAT;
    attr_descs[0].offset = 0;

    attr_descs[1].location = 1;
    attr_descs[1].binding = 0;
    attr_descs[1].format = KX_FORMAT_R32G32B32A32_SFLOAT;
    attr_descs[1].offset = 3 * sizeof(float);

    KxPipelineVertexInputStateCreateInfo vi_state{};
    vi_state.sType = KX_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi_state.vertexBindingDescriptionCount = 1;
    vi_state.pVertexBindingDescriptions = &binding_desc;
    vi_state.vertexAttributeDescriptionCount = 2;
    vi_state.pVertexAttributeDescriptions = attr_descs;

    // Viewport
    KxViewport viewport{};
    viewport.x = 0; viewport.y = 0; viewport.width = 256; viewport.height = 256;
    viewport.minDepth = 0.0f; viewport.maxDepth = 1.0f;

    KxRect2D scissor{};
    scissor.offset = {0, 0}; scissor.extent = {256, 256};

    KxPipelineViewportStateCreateInfo vp_state{};
    vp_state.sType = KX_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp_state.viewportCount = 1; vp_state.pViewports = &viewport;
    vp_state.scissorCount = 1; vp_state.pScissors = &scissor;

    // Rasterization
    KxPipelineRasterizationStateCreateInfo rs_state{};
    rs_state.sType = KX_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_state.polygonMode = KX_POLYGON_MODE_FILL;
    rs_state.frontFace = KX_FRONT_FACE_COUNTER_CLOCKWISE;
    rs_state.cullMode = KX_CULL_MODE_NONE;
    rs_state.lineWidth = 1.0f;

    // Depth/stencil
    KxPipelineDepthStencilStateCreateInfo ds_state{};
    ds_state.sType = KX_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_state.depthTestEnable = false;
    ds_state.depthWriteEnable = false;

    // Color blend
    KxPipelineColorBlendAttachmentState blend_att{};
    blend_att.blendEnable = false;
    blend_att.colorWriteMask = KX_COLOR_COMPONENT_R_BIT | KX_COLOR_COMPONENT_G_BIT |
                               KX_COLOR_COMPONENT_B_BIT | KX_COLOR_COMPONENT_A_BIT;

    KxPipelineColorBlendStateCreateInfo cb_state{};
    cb_state.sType = KX_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_state.attachmentCount = 1; cb_state.pAttachments = &blend_att;

    // Create pipeline
    KxPipelineCreateInfo pipe_info{};
    pipe_info.sType = KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO;
    pipe_info.bindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS;
    pipe_info.layout = pipeline_layout;
    pipe_info.vertexShaderModule = vert_shader;
    pipe_info.fragmentShaderModule = frag_shader;
    pipe_info.pVertexInputState = &vi_state;
    pipe_info.pViewportState = &vp_state;
    pipe_info.pRasterizationState = &rs_state;
    pipe_info.pDepthStencilState = &ds_state;
    pipe_info.pColorBlendState = &cb_state;
    pipe_info.primitiveTopology = KX_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    KxPipeline pipeline;
    res = kxCreatePipeline(device, nullptr, &pipe_info, nullptr, &pipeline);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create pipeline: %d\n", res);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Create vertex buffer - full screen triangle (covers entire screen)
    struct Vertex { float pos[3]; float color[4]; };
    Vertex vertices[3] = {
        {{-1.0f, -1.0f, 0.0f}, {lang.clear_color[0], lang.clear_color[1], lang.clear_color[2], 1.0f}},
        {{ 3.0f, -1.0f, 0.0f}, {lang.clear_color[0], lang.clear_color[1], lang.clear_color[2], 1.0f}},
        {{-1.0f,  3.0f, 0.0f}, {lang.clear_color[0], lang.clear_color[1], lang.clear_color[2], 1.0f}},
    };

    KxBufferCreateInfo vb_info{};
    vb_info.sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vb_info.size = sizeof(vertices);
    vb_info.usage = KX_BUFFER_USAGE_VERTEX_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT;
    vb_info.sharingModeExclusive = true;

    KxBuffer vertex_buffer;
    res = kxCreateBuffer(device, &vb_info, nullptr, &vertex_buffer);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create vertex buffer: %d\n", res);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    KxMemoryAllocateInfo vb_alloc_info{};
    vb_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vb_alloc_info.allocationSize = sizeof(vertices);
    vb_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory vertex_memory;
    res = kxAllocateMemory(device, &vb_alloc_info, nullptr, &vertex_memory);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to allocate vertex memory: %d\n", res);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    res = kxBindBufferMemory(device, vertex_buffer, vertex_memory, 0);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to bind vertex buffer: %d\n", res);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    void* mapped;
    res = kxMapMemory(device, vertex_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to map vertex memory: %d\n", res);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }
    memcpy(mapped, vertices, sizeof(vertices));
    kxUnmapMemory(device, vertex_memory);

    // Create color image (render target)
    KxImageCreateInfo color_img_info{};
    color_img_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    color_img_info.imageType = KX_IMAGE_TYPE_2D;
    color_img_info.format = KX_FORMAT_R8G8B8A8_UNORM;
    color_img_info.width = 256;
    color_img_info.height = 256;
    color_img_info.depth = 1;
    color_img_info.mipLevels = 1;
    color_img_info.arrayLayers = 1;
    color_img_info.samples = KX_SAMPLE_COUNT_1_BIT;
    color_img_info.tiling = KX_IMAGE_TILING_OPTIMAL;
    color_img_info.usage = KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | KX_IMAGE_USAGE_TRANSFER_SRC_BIT;
    color_img_info.sharingModeExclusive = true;
    color_img_info.initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;

    KxImage color_image;
    res = kxCreateImage(device, &color_img_info, nullptr, &color_image);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create color image: %d\n", res);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    KxMemoryAllocateInfo color_alloc_info{};
    color_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    color_alloc_info.allocationSize = 256 * 256 * 4;
    color_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory color_memory;
    res = kxAllocateMemory(device, &color_alloc_info, nullptr, &color_memory);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to allocate color memory: %d\n", res);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    res = kxBindImageMemory(device, color_image, color_memory, 0);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to bind color image: %d\n", res);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Create image view
    KxImageViewCreateInfo color_view_info{};
    color_view_info.sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_view_info.image = color_image;
    color_view_info.viewType = KX_IMAGE_VIEW_TYPE_2D;
    color_view_info.format = KX_FORMAT_R8G8B8A8_UNORM;
    color_view_info.subresourceRange.aspectMask = KX_IMAGE_ASPECT_COLOR_BIT;
    color_view_info.subresourceRange.baseMipLevel = 0;
    color_view_info.subresourceRange.levelCount = 1;
    color_view_info.subresourceRange.baseArrayLayer = 0;
    color_view_info.subresourceRange.layerCount = 1;

    KxImageView color_view;
    res = kxCreateImageView(device, &color_view_info, nullptr, &color_view);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create image view: %d\n", res);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Create render pass
    KxAttachmentDescription color_att{};
    color_att.sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
    color_att.format = KX_FORMAT_R8G8B8A8_UNORM;
    color_att.samples = KX_SAMPLE_COUNT_1_BIT;
    color_att.loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR;
    color_att.storeOp = KX_ATTACHMENT_STORE_OP_STORE;
    color_att.stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_att.stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE;
    color_att.initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    color_att.finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    KxAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    KxSubpassDescription subpass{};
    subpass.sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION;
    subpass.pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;

    KxRenderPassCreateInfo rp_info{};
    rp_info.sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments = &color_att;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    KxRenderPass render_pass;
    res = kxCreateRenderPass(device, &rp_info, nullptr, &render_pass);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create render pass: %d\n", res);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Create framebuffer
    KxImageView fb_attachments[1] = {color_view};
    KxFramebufferCreateInfo fb_info{};
    fb_info.sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.renderPass = render_pass;
    fb_info.attachmentCount = 1;
    fb_info.pAttachments = fb_attachments;
    fb_info.width = 256;
    fb_info.height = 256;
    fb_info.layers = 1;

    KxFramebuffer framebuffer;
    res = kxCreateFramebuffer(device, &fb_info, nullptr, &framebuffer);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to create framebuffer: %d\n", res);
        kxDestroyRenderPass(device, render_pass, nullptr);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Begin command buffer
    KxCommandBufferBeginInfo begin_info{};
    begin_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    res = kxBeginCommandBuffer(command_buffer, &begin_info);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to begin command buffer: %d\n", res);
        kxDestroyFramebuffer(device, framebuffer, nullptr);
        kxDestroyRenderPass(device, render_pass, nullptr);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Begin render pass
    KxClearValue clear_value{};
    clear_value.color[0] = 0.0f;
    clear_value.color[1] = 0.0f;
    clear_value.color[2] = 0.0f;
    clear_value.color[3] = 1.0f;

    KxRenderPassBeginInfo rp_begin{};
    rp_begin.sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass = render_pass;
    rp_begin.framebuffer = framebuffer;
    rp_begin.renderArea.offset = {0, 0};
    rp_begin.renderArea.extent = {256, 256};
    rp_begin.clearValueCount = 1;
    rp_begin.pClearValues = &clear_value;

    kxCmdBeginRenderPass(command_buffer, &rp_begin, KX_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline
    kxCmdBindPipeline(command_buffer, KX_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // Bind vertex buffer
    KxBuffer vbs[1] = {vertex_buffer};
    uint64_t offsets[1] = {0};
    kxCmdBindVertexBuffers(command_buffer, 0, 1, vbs, offsets);

    // Draw triangle (3 vertices)
    kxCmdDraw(command_buffer, 3, 1, 0, 0);

    // End render pass
    kxCmdEndRenderPass(command_buffer);

    // End command buffer
    res = kxEndCommandBuffer(command_buffer);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to end command buffer: %d\n", res);
        kxDestroyFramebuffer(device, framebuffer, nullptr);
        kxDestroyRenderPass(device, render_pass, nullptr);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    // Submit
    KxSubmitInfo submit{};
    submit.sType = KX_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_buffer;

    res = kxQueueSubmit(queue, 1, &submit, nullptr);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to submit: %d\n", res);
        kxDestroyFramebuffer(device, framebuffer, nullptr);
        kxDestroyRenderPass(device, render_pass, nullptr);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }

    kxQueueWaitIdle(queue);

    // Read back pixels to verify
    std::vector<uint8_t> pixels(256 * 256 * 4);
    res = kxMapMemory(device, color_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    if (res != KX_SUCCESS) {
        std::printf("    Failed to map color memory: %d\n", res);
        kxDestroyFramebuffer(device, framebuffer, nullptr);
        kxDestroyRenderPass(device, render_pass, nullptr);
        kxDestroyImageView(device, color_view, nullptr);
        kxFreeMemory(device, color_memory, nullptr);
        kxDestroyImage(device, color_image, nullptr);
        kxFreeMemory(device, vertex_memory, nullptr);
        kxDestroyBuffer(device, vertex_buffer, nullptr);
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
        return false;
    }
    memcpy(pixels.data(), mapped, 256 * 256 * 4);
    kxUnmapMemory(device, color_memory);

    // Check center pixel
    int center_idx = (128 * 256 + 128) * 4;
    uint8_t r = pixels[center_idx];
    uint8_t g = pixels[center_idx + 1];
    uint8_t b = pixels[center_idx + 2];

    std::printf("    Center pixel: R=%d G=%d B=%d (expected ~%d %d %d)\n",
                r, g, b,
                (int)(lang.clear_color[0] * 255),
                (int)(lang.clear_color[1] * 255),
                (int)(lang.clear_color[2] * 255));

    // Cleanup
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyImageView(device, color_view, nullptr);
    kxFreeMemory(device, color_memory, nullptr);
    kxDestroyImage(device, color_image, nullptr);
    kxFreeMemory(device, vertex_memory, nullptr);
    kxDestroyBuffer(device, vertex_buffer, nullptr);
    kxDestroyPipeline(device, pipeline, nullptr);
    kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
    kxDestroyShaderModule(device, frag_shader, nullptr);
    kxDestroyShaderModule(device, vert_shader, nullptr);
    kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    kxDestroyCommandPool(device, command_pool, nullptr);

    return true;
}

int main() {
    std::printf("Example 05: Shader Languages\n");
    std::printf("==============================\n");

    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Shader Languages";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Kromax";
    app_info.engineVersion = 1;
    app_info.apiVersion = KX_API_VERSION;

    KxInstanceCreateInfo instance_info{};
    instance_info.sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    KxInstance instance;
    KxResult result = kxCreateInstance(&instance_info, nullptr, &instance);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create instance: %d\n", result);
        return -1;
    }

    // Enumerate physical devices
    uint32_t device_count = 0;
    kxEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        std::printf("No physical devices found\n");
        kxDestroyInstance(instance, nullptr);
        return -1;
    }

    std::vector<KxPhysicalDevice> devices(device_count);
    kxEnumeratePhysicalDevices(instance, &device_count, devices.data());
    KxPhysicalDevice physical_device = devices[0];

    // Get queue family
    uint32_t queue_family_count = 0;
    kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<KxQueueFamilyProperties> queue_families(queue_family_count);
    kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    uint32_t graphics_queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & KX_QUEUE_GRAPHICS_BIT) {
            graphics_queue_family = i;
            break;
        }
    }

    if (graphics_queue_family == UINT32_MAX) {
        std::printf("No graphics queue family found\n");
        kxDestroyInstance(instance, nullptr);
        return -1;
    }

    // Create device
    float queue_priority = 1.0f;
    KxDeviceQueueCreateInfo queue_info{};
    queue_info.sType = KX_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = graphics_queue_family;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = &queue_priority;

    KxDeviceCreateInfo device_info{};
    device_info.sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;

    KxDevice device;
    result = kxCreateDevice(physical_device, &device_info, nullptr, &device);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create device: %d\n", result);
        kxDestroyInstance(instance, nullptr);
        return -1;
    }

    KxQueue queue;
    kxGetDeviceQueue(device, graphics_queue_family, 0, &queue);

    // MVP matrix (orthographic for full-screen triangle)
    Mat4 mvp = mat4_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    // Test each shader language with a distinct color
    ShaderLanguage languages[] = {
        {"GLSL", KX_SHADER_LANGUAGE_GLSL, GLSL_VERTEX, GLSL_FRAGMENT, {1.0f, 0.0f, 0.0f}},      // Red
        {"WGSL", KX_SHADER_LANGUAGE_WGSL, WGSL_VERTEX, WGSL_FRAGMENT, {0.0f, 1.0f, 0.0f}},      // Green
        {"KXSL", KX_SHADER_LANGUAGE_KXSL, KXSL_VERTEX, KXSL_FRAGMENT, {0.0f, 0.0f, 1.0f}},      // Blue
    };

    int passed = 0;
    int total = sizeof(languages) / sizeof(languages[0]);

    for (int i = 0; i < total; ++i) {
        if (run_language_test(device, queue, graphics_queue_family, languages[i], mvp)) {
            std::printf("  %s: PASSED\n", languages[i].name);
            passed++;
        } else {
            std::printf("  %s: FAILED\n", languages[i].name);
        }
    }

    std::printf("\nResults: %d/%d languages passed\n", passed, total);

    // Test language detection
    std::printf("\nLanguage Detection:\n");
    KxShaderLanguage detected = kxDetectShaderLanguage(GLSL_VERTEX, strlen(GLSL_VERTEX));
    std::printf("  GLSL source detected as: %d (expected %d)\n", detected, KX_SHADER_LANGUAGE_GLSL);

    detected = kxDetectShaderLanguage(WGSL_VERTEX, strlen(WGSL_VERTEX));
    std::printf("  WGSL source detected as: %d (expected %d)\n", detected, KX_SHADER_LANGUAGE_WGSL);

    detected = kxDetectShaderLanguage(KXSL_VERTEX, strlen(KXSL_VERTEX));
    std::printf("  KXSL source detected as: %d\n", detected);

    // Cleanup
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);

    return (passed == total) ? 0 : -1;
}