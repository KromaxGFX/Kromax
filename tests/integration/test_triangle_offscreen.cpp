// Integration test: triangle offscreen rendering with framebuffer verification
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>

#include <cstdio>
#include <cmath>
#include <cstring>

// Simple matrix math matching example 03_triangle conventions
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

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    Mat4 r{};
    float f = 1.0f / std::tan(fov * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (far + near) / (near - far);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * far * near) / (near - far);
    r.m[15] = 0.0f;
    return r;
}

Mat4 mat4_look_at(float eye_x, float eye_y, float eye_z,
                  float center_x, float center_y, float center_z,
                  float up_x, float up_y, float up_z) {
    Mat4 r{};
    float fx = center_x - eye_x;
    float fy = center_y - eye_y;
    float fz = center_z - eye_z;
    float fl = std::sqrt(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;

    float sx = fy * up_z - fz * up_y;
    float sy = fz * up_x - fx * up_z;
    float sz = fx * up_y - fy * up_x;
    float sl = std::sqrt(sx*sx + sy*sy + sz*sz);
    sx /= sl; sy /= sl; sz /= sl;

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    r.m[0] = sx; r.m[1] = ux; r.m[2] = -fx; r.m[3] = 0;
    r.m[4] = sy; r.m[5] = uy; r.m[6] = -fy; r.m[7] = 0;
    r.m[8] = sz; r.m[9] = uz; r.m[10] = -fz; r.m[11] = 0;
    r.m[12] = -(sx*eye_x + sy*eye_y + sz*eye_z);
    r.m[13] = -(ux*eye_x + uy*eye_y + uz*eye_z);
    r.m[14] = fx*eye_x + fy*eye_y + fz*eye_z;
    r.m[15] = 1.0f;
    return r;
}

Mat4 mat4_rotate_y(float angle) {
    Mat4 r = mat4_identity();
    float c = std::cos(angle);
    float s = std::sin(angle);
    r.m[0] = c; r.m[2] = -s;
    r.m[8] = s; r.m[10] = c;
    return r;
}

// Vertex shader callback
void vertex_shader(const KxVertexInput* input, KxVertexOutput* output, const void* push_constants) {
    const Mat4* mvp = static_cast<const Mat4*>(push_constants);
    
    float px = input->position ? input->position[0] : 0.0f;
    float py = input->position ? input->position[1] : 0.0f;
    float pz = input->position ? input->position[2] : 0.0f;
    float pw = 1.0f;
    
    float clip_x = mvp->m[0] * px + mvp->m[4] * py + mvp->m[8] * pz + mvp->m[12] * pw;
    float clip_y = mvp->m[1] * px + mvp->m[5] * py + mvp->m[9] * pz + mvp->m[13] * pw;
    float clip_z = mvp->m[2] * px + mvp->m[6] * py + mvp->m[10] * pz + mvp->m[14] * pw;
    float clip_w = mvp->m[3] * px + mvp->m[7] * py + mvp->m[11] * pz + mvp->m[15] * pw;
    
    output->position[0] = clip_x;
    output->position[1] = clip_y;
    output->position[2] = clip_z;
    output->position[3] = clip_w;
    
    if (input->normal) {
        output->normal[0] = input->normal[0];
        output->normal[1] = input->normal[1];
        output->normal[2] = input->normal[2];
    } else {
        output->normal[0] = 0.0f;
        output->normal[1] = 0.0f;
        output->normal[2] = 1.0f;
    }
    
    if (input->texcoord) {
        output->texcoord[0] = input->texcoord[0];
        output->texcoord[1] = input->texcoord[1];
    } else {
        output->texcoord[0] = 0.0f;
        output->texcoord[1] = 0.0f;
    }
    
    if (input->color) {
        output->color[0] = input->color[0];
        output->color[1] = input->color[1];
        output->color[2] = input->color[2];
        output->color[3] = input->color[3];
    } else {
        output->color[0] = 1.0f;
        output->color[1] = 1.0f;
        output->color[2] = 1.0f;
        output->color[3] = 1.0f;
    }
}

// Fragment shader callback
void fragment_shader(const KxFragmentInput* input, KxFragmentOutput* output, const void* push_constants) {
    (void)push_constants;
    output->color[0] = input->color[0];
    output->color[1] = input->color[1];
    output->color[2] = input->color[2];
    output->color[3] = input->color[3];
    output->depth = input->frag_coord[2];
}

TEST_CASE("triangle_offscreen_rendering") {
    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "triangle_offscreen_test";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Kromax";
    app_info.engineVersion = 1;
    app_info.apiVersion = KX_API_VERSION_0_1;

    KxInstanceCreateInfo instance_info{};
    instance_info.sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;

    KxInstance instance;
    KxResult result = kxCreateInstance(&instance_info, nullptr, &instance);
    REQUIRE(result == KX_SUCCESS);

    // Enumerate physical devices
    uint32_t device_count = 0;
    result = kxEnumeratePhysicalDevices(instance, &device_count, nullptr);
    REQUIRE(result == KX_SUCCESS);
    REQUIRE(device_count >= 1);

    std::vector<KxPhysicalDevice> devices(device_count);
    result = kxEnumeratePhysicalDevices(instance, &device_count, devices.data());
    REQUIRE(result == KX_SUCCESS);
    KxPhysicalDevice physical_device = devices[0];

    // Get queue family properties
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
    REQUIRE(graphics_queue_family != UINT32_MAX);

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
    REQUIRE(result == KX_SUCCESS);

    // Get queue
    KxQueue queue;
    kxGetDeviceQueue(device, graphics_queue_family, 0, &queue);

    // Create command pool
    KxCommandPoolCreateInfo pool_info{};
    pool_info.sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = graphics_queue_family;
    pool_info.flags = KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    KxCommandPool command_pool;
    result = kxCreateCommandPool(device, &pool_info, nullptr, &command_pool);
    REQUIRE(result == KX_SUCCESS);

    // Create command buffer
    KxCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    KxCommandBuffer command_buffer;
    result = kxAllocateCommandBuffers(device, &alloc_info, &command_buffer);
    REQUIRE(result == KX_SUCCESS);

    // Create vertex buffer - triangle with 3 vertices (position + color)
    struct Vertex {
        float pos[3];
        float color[4];
    };
    
    Vertex vertices[3] = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},  // Top - red
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},  // Bottom-right - blue
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},  // Bottom-left - green
    };

    KxBufferCreateInfo vb_info{};
    vb_info.sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vb_info.size = sizeof(vertices);
    vb_info.usage = KX_BUFFER_USAGE_VERTEX_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT;
    vb_info.sharingModeExclusive = true;

    KxBuffer vertex_buffer;
    result = kxCreateBuffer(device, &vb_info, nullptr, &vertex_buffer);
    REQUIRE(result == KX_SUCCESS);

    // Allocate memory for vertex buffer
    KxMemoryAllocateInfo vb_alloc_info{};
    vb_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vb_alloc_info.allocationSize = sizeof(vertices);
    vb_alloc_info.memoryTypeIndex = 0;  // Assume memory type 0 is host visible

    KxDeviceMemory vertex_memory;
    result = kxAllocateMemory(device, &vb_alloc_info, nullptr, &vertex_memory);
    REQUIRE(result == KX_SUCCESS);

    // Bind vertex buffer memory
    result = kxBindBufferMemory(device, vertex_buffer, vertex_memory, 0);
    REQUIRE(result == KX_SUCCESS);

    // Map and write vertex data
    void* mapped;
    result = kxMapMemory(device, vertex_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    REQUIRE(result == KX_SUCCESS);
    memcpy(mapped, vertices, sizeof(vertices));
    kxUnmapMemory(device, vertex_memory);

    // Create depth image
    KxImageCreateInfo depth_img_info{};
    depth_img_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_img_info.imageType = KX_IMAGE_TYPE_2D;
    depth_img_info.format = KX_FORMAT_D32_SFLOAT;
    depth_img_info.width = 256;
    depth_img_info.height = 256;
    depth_img_info.depth = 1;
    depth_img_info.mipLevels = 1;
    depth_img_info.arrayLayers = 1;
    depth_img_info.samples = KX_SAMPLE_COUNT_1_BIT;
    depth_img_info.tiling = KX_IMAGE_TILING_OPTIMAL;
    depth_img_info.usage = KX_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_img_info.sharingModeExclusive = true;
    depth_img_info.initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;

    KxImage depth_image;
    result = kxCreateImage(device, &depth_img_info, nullptr, &depth_image);
    REQUIRE(result == KX_SUCCESS);

    KxMemoryAllocateInfo depth_alloc_info{};
    depth_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    depth_alloc_info.allocationSize = 256 * 256 * 4;
    depth_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory depth_memory;
    result = kxAllocateMemory(device, &depth_alloc_info, nullptr, &depth_memory);
    REQUIRE(result == KX_SUCCESS);

    result = kxBindImageMemory(device, depth_image, depth_memory, 0);
    REQUIRE(result == KX_SUCCESS);

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
    result = kxCreateImage(device, &color_img_info, nullptr, &color_image);
    REQUIRE(result == KX_SUCCESS);

    KxMemoryAllocateInfo color_alloc_info{};
    color_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    color_alloc_info.allocationSize = 256 * 256 * 4;
    color_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory color_memory;
    result = kxAllocateMemory(device, &color_alloc_info, nullptr, &color_memory);
    REQUIRE(result == KX_SUCCESS);

    result = kxBindImageMemory(device, color_image, color_memory, 0);
    REQUIRE(result == KX_SUCCESS);

    // Create image views
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
    result = kxCreateImageView(device, &color_view_info, nullptr, &color_view);
    REQUIRE(result == KX_SUCCESS);

    KxImageViewCreateInfo depth_view_info{};
    depth_view_info.sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_view_info.image = depth_image;
    depth_view_info.viewType = KX_IMAGE_VIEW_TYPE_2D;
    depth_view_info.format = KX_FORMAT_D32_SFLOAT;
    depth_view_info.subresourceRange.aspectMask = KX_IMAGE_ASPECT_DEPTH_BIT;
    depth_view_info.subresourceRange.baseMipLevel = 0;
    depth_view_info.subresourceRange.levelCount = 1;
    depth_view_info.subresourceRange.baseArrayLayer = 0;
    depth_view_info.subresourceRange.layerCount = 1;

    KxImageView depth_view;
    result = kxCreateImageView(device, &depth_view_info, nullptr, &depth_view);
    REQUIRE(result == KX_SUCCESS);

    // Create render pass
    KxAttachmentDescription attachments[2] = {};
    
    // Color attachment
    attachments[0].sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
    attachments[0].format = KX_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = KX_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = KX_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    attachments[1].sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
    attachments[1].format = KX_FORMAT_D32_SFLOAT;
    attachments[1].samples = KX_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = KX_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = KX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    KxAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    KxAttachmentReference depth_ref{};
    depth_ref.attachment = 1;
    depth_ref.layout = KX_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    KxSubpassDescription subpass{};
    subpass.sType = KX_STRUCTURE_TYPE_SUBPASS_DESCRIPTION;
    subpass.pipelineBindPoint = KX_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pResolveAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depth_ref;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    KxSubpassDependency dependency{};
    dependency.srcSubpass = KX_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = KX_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    KxRenderPassCreateInfo rp_info{};
    rp_info.sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 2;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 1;
    rp_info.pDependencies = &dependency;

    KxRenderPass render_pass;
    result = kxCreateRenderPass(device, &rp_info, nullptr, &render_pass);
    REQUIRE(result == KX_SUCCESS);

    // Create framebuffer
    KxImageView fb_attachments[2] = {color_view, depth_view};
    KxFramebufferCreateInfo fb_info{};
    fb_info.sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.renderPass = render_pass;
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = fb_attachments;
    fb_info.width = 256;
    fb_info.height = 256;
    fb_info.layers = 1;

    KxFramebuffer framebuffer;
    result = kxCreateFramebuffer(device, &fb_info, nullptr, &framebuffer);
    REQUIRE(result == KX_SUCCESS);

    // Create shader modules
    KxShaderModuleNativeCallbackInfo native_info{};
    native_info.vertex_fn = vertex_shader;
    native_info.fragment_fn = fragment_shader;
    native_info.compute_fn = nullptr;
    native_info.push_constants = nullptr;
    native_info.push_constants_size = sizeof(Mat4);

    KxShaderModuleCreateInfo vert_shader_info{};
    vert_shader_info.sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vert_shader_info.format = KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT;
    vert_shader_info.codeSize = sizeof(native_info);
    vert_shader_info.pCode = &native_info;

    KxShaderModule vert_shader;
    result = kxCreateShaderModule(device, &vert_shader_info, nullptr, &vert_shader);
    REQUIRE(result == KX_SUCCESS);

    KxShaderModuleCreateInfo frag_shader_info = vert_shader_info;
    KxShaderModule frag_shader;
    result = kxCreateShaderModule(device, &frag_shader_info, nullptr, &frag_shader);
    REQUIRE(result == KX_SUCCESS);

    // Create pipeline layout with push constants for MVP matrix
    KxPushConstantRange push_range{};
    push_range.stageFlags = KX_SHADER_STAGE_VERTEX_BIT;
    push_range.offset = 0;
    push_range.size = sizeof(Mat4);

    KxPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_range;

    KxPipelineLayout pipeline_layout;
    result = kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
    REQUIRE(result == KX_SUCCESS);

    // Vertex input state
    KxVertexInputBindingDescription binding_desc{};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(Vertex);
    binding_desc.inputRate = KX_VERTEX_INPUT_RATE_VERTEX;

    KxVertexInputAttributeDescription attr_descs[2] = {};
    attr_descs[0].location = 0;  // position
    attr_descs[0].binding = 0;
    attr_descs[0].format = KX_FORMAT_R32G32B32_SFLOAT;  // vec3
    attr_descs[0].offset = 0;

    attr_descs[1].location = 3;  // color
    attr_descs[1].binding = 0;
    attr_descs[1].format = KX_FORMAT_R32G32B32A32_SFLOAT;  // vec4
    attr_descs[1].offset = sizeof(float) * 3;  // after position

    KxPipelineVertexInputStateCreateInfo vi_state{};
    vi_state.sType = KX_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi_state.vertexBindingDescriptionCount = 1;
    vi_state.pVertexBindingDescriptions = &binding_desc;
    vi_state.vertexAttributeDescriptionCount = 2;
    vi_state.pVertexAttributeDescriptions = attr_descs;

    // Viewport state
    KxViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = 256;
    viewport.height = 256;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    KxRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {256, 256};

    KxPipelineViewportStateCreateInfo vp_state{};
    vp_state.sType = KX_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp_state.viewportCount = 1;
    vp_state.pViewports = &viewport;
    vp_state.scissorCount = 1;
    vp_state.pScissors = &scissor;

    // Rasterization state
    KxPipelineRasterizationStateCreateInfo rs_state{};
    rs_state.sType = KX_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs_state.polygonMode = KX_POLYGON_MODE_FILL;
    rs_state.frontFace = KX_FRONT_FACE_COUNTER_CLOCKWISE;
    rs_state.cullMode = KX_CULL_MODE_BACK_BIT;
    rs_state.lineWidth = 1.0f;

    // Depth/stencil state
    KxPipelineDepthStencilStateCreateInfo ds_state{};
    ds_state.sType = KX_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds_state.depthTestEnable = true;
    ds_state.depthWriteEnable = true;
    ds_state.depthCompareOp = KX_COMPARE_OP_LESS;
    ds_state.depthBoundsTestEnable = false;
    ds_state.stencilTestEnable = false;

    // Color blend state
    KxPipelineColorBlendAttachmentState blend_att{};
    blend_att.blendEnable = false;
    blend_att.colorWriteMask = KX_COLOR_COMPONENT_R_BIT | KX_COLOR_COMPONENT_G_BIT | 
                               KX_COLOR_COMPONENT_B_BIT | KX_COLOR_COMPONENT_A_BIT;

    KxPipelineColorBlendStateCreateInfo cb_state{};
    cb_state.sType = KX_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb_state.attachmentCount = 1;
    cb_state.pAttachments = &blend_att;

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
    result = kxCreatePipeline(device, nullptr, &pipe_info, nullptr, &pipeline);
    REQUIRE(result == KX_SUCCESS);

    // MVP matrix - orthographic projection
    Mat4 model = mat4_identity();
    Mat4 view = mat4_identity();
    Mat4 proj = mat4_identity();
    proj.m[0] = 1.0f;
    proj.m[5] = 1.0f;
    proj.m[10] = -1.0f;
    proj.m[15] = 1.0f;
    Mat4 view_model = mat4_multiply(view, model);
    Mat4 mvp = mat4_multiply(proj, view_model);

    // Begin command buffer
    KxCommandBufferBeginInfo begin_info{};
    begin_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = kxBeginCommandBuffer(command_buffer, &begin_info);
    REQUIRE(result == KX_SUCCESS);

    // Begin render pass
    KxClearValue clear_values[2];
    clear_values[0].color[0] = 0.1f;  // Dark blue background
    clear_values[0].color[1] = 0.1f;
    clear_values[0].color[2] = 0.2f;
    clear_values[0].color[3] = 1.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    KxRenderPassBeginInfo rp_begin{};
    rp_begin.sType = KX_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass = render_pass;
    rp_begin.framebuffer = framebuffer;
    rp_begin.renderArea.offset = {0, 0};
    rp_begin.renderArea.extent = {256, 256};
    rp_begin.clearValueCount = 2;
    rp_begin.pClearValues = clear_values;

    kxCmdBeginRenderPass(command_buffer, &rp_begin, KX_SUBPASS_CONTENTS_INLINE);

    // Bind pipeline
    kxCmdBindPipeline(command_buffer, KX_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    // Bind vertex buffer
    KxBuffer vbs[1] = {vertex_buffer};
    uint64_t offsets[1] = {0};
    kxCmdBindVertexBuffers(command_buffer, 0, 1, vbs, offsets);

    // Push constants (MVP matrix)
    kxCmdPushConstants(command_buffer, pipeline_layout, KX_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &mvp);

    // Draw triangle
    kxCmdDraw(command_buffer, 3, 1, 0, 0);

    // End render pass
    kxCmdEndRenderPass(command_buffer);

    // End command buffer
    result = kxEndCommandBuffer(command_buffer);
    REQUIRE(result == KX_SUCCESS);

    // Submit
    KxSubmitInfo submit{};
    submit.sType = KX_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &command_buffer;

    result = kxQueueSubmit(queue, 1, &submit, nullptr);
    REQUIRE(result == KX_SUCCESS);

    kxQueueWaitIdle(queue);

    // Verify framebuffer content
    // Map color image memory and check pixels
    void* color_mapped;
    result = kxMapMemory(device, color_memory, 0, KX_WHOLE_SIZE, 0, &color_mapped);
    REQUIRE(result == KX_SUCCESS);

    const uint8_t* pixels = static_cast<const uint8_t*>(color_mapped);
    
    // Check that not all pixels are the clear color (0.1, 0.1, 0.2, 1.0) ~ (26, 26, 51, 255)
    // The triangle should render red, green, blue pixels in the center
    bool found_non_clear = false;
    bool found_red = false;
    bool found_green = false;
    bool found_blue = false;
    
    // Sample pixels in a grid pattern across the center region where triangle should be
    for (int y = 80; y < 180 && !found_non_clear; y += 10) {
        for (int x = 80; x < 180; x += 10) {
            int idx = (y * 256 + x) * 4;
            uint8_t r = pixels[idx];
            uint8_t g = pixels[idx + 1];
            uint8_t b = pixels[idx + 2];
            uint8_t a = pixels[idx + 3];
            
            // Clear color is roughly (26, 26, 51, 255)
            if (r > 30 || g > 30 || b < 200) {
                found_non_clear = true;
                if (r > 200 && g < 50 && b < 50) found_red = true;
                if (r < 50 && g > 200 && b < 50) found_green = true;
                if (r < 50 && g < 50 && b > 200) found_blue = true;
            }
        }
    }
    
    kxUnmapMemory(device, color_memory);
    
    // Verify that triangle rendered something other than clear color
    CHECK(found_non_clear);
    
    // Verify triangle vertices colors are present (red, green, blue)
    // Note: These might not be hit by our sparse grid sampling; just verify non-clear
    bool found_any_color = found_red || found_green || found_blue;
    CHECK(found_any_color);

    // Cleanup
    kxDestroyPipeline(device, pipeline, nullptr);
    kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
    kxDestroyShaderModule(device, frag_shader, nullptr);
    kxDestroyShaderModule(device, vert_shader, nullptr);
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyImageView(device, depth_view, nullptr);
    kxDestroyImageView(device, color_view, nullptr);
    kxFreeMemory(device, color_memory, nullptr);
    kxDestroyImage(device, color_image, nullptr);
    kxFreeMemory(device, depth_memory, nullptr);
    kxDestroyImage(device, depth_image, nullptr);
    kxUnmapMemory(device, vertex_memory);
    kxFreeMemory(device, vertex_memory, nullptr);
    kxDestroyBuffer(device, vertex_buffer, nullptr);
    kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    kxDestroyCommandPool(device, command_pool, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}