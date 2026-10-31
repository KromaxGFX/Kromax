// Example 06: Material PBR
// Demonstrates creating a material with the built-in PBR shader, setting textures and parameters,
// and rendering a simple scene

#include <kromax/kromax.h>
#include <cstdio>
#include <cmath>
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

Mat4 mat4_perspective(float fov, float aspect, float z_near, float z_far) {
    Mat4 r{};
    float f = 1.0f / std::tan(fov * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (z_far + z_near) / (z_near - z_far);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * z_far * z_near) / (z_near - z_far);
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

// Create a procedural texture
void create_texture_data(uint8_t* data, int width, int height, int pattern) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            float u = (float)x / width;
            float v = (float)y / height;

            switch (pattern) {
                case 0: // Checkerboard (base color)
                    data[idx] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 200 : 50;
                    data[idx+1] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 150 : 30;
                    data[idx+2] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 100 : 20;
                    data[idx+3] = 255;
                    break;
                case 1: // Normal map (blue-ish)
                    data[idx] = 128;
                    data[idx+1] = 128;
                    data[idx+2] = 255;
                    data[idx+3] = 255;
                    break;
                case 2: // Metallic/Roughness (R=metallic, G=roughness)
                    data[idx] = (uint8_t)(u * 255);   // Metallic
                    data[idx+1] = (uint8_t)(v * 255); // Roughness
                    data[idx+2] = 0;
                    data[idx+3] = 255;
                    break;
                case 3: // Emissive (orange glow)
                    data[idx] = 255;
                    data[idx+1] = (uint8_t)(u * 100);
                    data[idx+2] = 0;
                    data[idx+3] = 255;
                    break;
                case 4: // Occlusion (shadow in corners)
                    data[idx] = data[idx+1] = data[idx+2] = (uint8_t)(200 + 55 * (1.0f - std::abs(u-0.5f)*2) * (1.0f - std::abs(v-0.5f)*2));
                    data[idx+3] = 255;
                    break;
                default:
                    data[idx] = 128; data[idx+1] = 128; data[idx+2] = 128; data[idx+3] = 255;
            }
        }
    }
}

KxImageView create_texture_image(KxDevice device, int width, int height, int pattern,
                                 KxDeviceMemory* out_memory, KxImage* out_image) {
    KxImageCreateInfo img_info{};
    img_info.sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    img_info.imageType = KX_IMAGE_TYPE_2D;
    img_info.format = KX_FORMAT_R8G8B8A8_UNORM;
    img_info.width = width;
    img_info.height = height;
    img_info.depth = 1;
    img_info.mipLevels = 1;
    img_info.arrayLayers = 1;
    img_info.samples = KX_SAMPLE_COUNT_1_BIT;
    img_info.tiling = KX_IMAGE_TILING_OPTIMAL;
    img_info.usage = KX_IMAGE_USAGE_SAMPLED_BIT | KX_IMAGE_USAGE_TRANSFER_DST_BIT;
    img_info.sharingModeExclusive = true;
    img_info.initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;

    KxImage image;
    KxResult res = kxCreateImage(device, &img_info, nullptr, &image);
    if (res != KX_SUCCESS) return nullptr;

    KxMemoryAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = width * height * 4;
    alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory memory;
    res = kxAllocateMemory(device, &alloc_info, nullptr, &memory);
    if (res != KX_SUCCESS) { kxDestroyImage(device, image, nullptr); return nullptr; }

    res = kxBindImageMemory(device, image, memory, 0);
    if (res != KX_SUCCESS) { kxFreeMemory(device, memory, nullptr); kxDestroyImage(device, image, nullptr); return nullptr; }

    std::vector<uint8_t> tex_data(width * height * 4);
    create_texture_data(tex_data.data(), width, height, pattern);

    void* mapped;
    res = kxMapMemory(device, memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    if (res == KX_SUCCESS) {
        memcpy(mapped, tex_data.data(), tex_data.size());
        kxUnmapMemory(device, memory);
    }

    KxImageViewCreateInfo view_info{};
    view_info.sType = KX_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = KX_IMAGE_VIEW_TYPE_2D;
    view_info.format = KX_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask = KX_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    KxImageView view;
    res = kxCreateImageView(device, &view_info, nullptr, &view);

    if (out_memory) *out_memory = memory;
    if (out_image) *out_image = image;

    return view;
}

KxSampler create_default_sampler(KxDevice device) {
    KxSamplerCreateInfo sampler_info{};
    sampler_info.sType = KX_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = KX_FILTER_LINEAR;
    sampler_info.minFilter = KX_FILTER_LINEAR;
    sampler_info.addressModeU = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = KX_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.compareEnable = KX_FALSE;
    sampler_info.compareOp = KX_COMPARE_OP_ALWAYS;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    sampler_info.unnormalizedCoordinates = KX_FALSE;

    KxSampler sampler;
    KxResult res = kxCreateSampler(device, &sampler_info, nullptr, &sampler);
    return res == KX_SUCCESS ? sampler : nullptr;
}

int main() {
    std::printf("Example 06: Material PBR\n");
    std::printf("=========================\n");

    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Material PBR";
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

    // Create descriptor pool and set layout for material
    KxDescriptorPoolSize pool_sizes[2] = {};
    pool_sizes[0].type = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 1;
    pool_sizes[1].type = KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = 5;

    KxDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_sizes;

    KxDescriptorPool descriptor_pool;
    result = kxCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create descriptor pool: %d\n", result);
        return -1;
    }

    KxDescriptorSetLayoutBinding bindings[6] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = KX_SHADER_STAGE_VERTEX_BIT | KX_SHADER_STAGE_FRAGMENT_BIT;
    for (int i = 1; i <= 5; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = KX_SHADER_STAGE_FRAGMENT_BIT;
    }

    KxDescriptorSetLayoutCreateInfo set_layout_info{};
    set_layout_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = 6;
    set_layout_info.pBindings = bindings;

    KxDescriptorSetLayout descriptor_set_layout;
    result = kxCreateDescriptorSetLayout(device, &set_layout_info, nullptr, &descriptor_set_layout);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create descriptor set layout: %d\n", result);
        return -1;
    }

    KxDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    KxDescriptorSet descriptor_set;
    result = kxAllocateDescriptorSets(device, &alloc_info, &descriptor_set);
    if (result != KX_SUCCESS) {
        std::printf("Failed to allocate descriptor set: %d\n", result);
        return -1;
    }

    // Create pipeline layout
    KxPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &descriptor_set_layout;
    layout_info.pushConstantRangeCount = 1;
    KxPushConstantRange push_range{};
    push_range.stageFlags = KX_SHADER_STAGE_VERTEX_BIT;
    push_range.offset = 0;
    push_range.size = sizeof(Mat4);
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &push_range;

    KxPipelineLayout pipeline_layout;
    result = kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create pipeline layout: %d\n", result);
        return -1;
    }

    // Create default material (built-in PBR shader)
    KxMaterial material;
    result = kxCreateDefaultMaterial(device, nullptr, &material);
    if (result != KX_SUCCESS) {
        std::printf("Failed to create default material: %d\n", result);
        return -1;
    }
    std::printf("Created default PBR material\n");

    // Create textures
    KxDeviceMemory tex_memories[5];
    KxImage tex_images[5];
    KxImageView tex_views[5];
    KxSampler sampler = create_default_sampler(device);

    tex_views[0] = create_texture_image(device, 64, 64, 0, &tex_memories[0], &tex_images[0]); // Base color
    tex_views[1] = create_texture_image(device, 64, 64, 1, &tex_memories[1], &tex_images[1]); // Normal
    tex_views[2] = create_texture_image(device, 64, 64, 2, &tex_memories[2], &tex_images[2]); // Metallic/Roughness
    tex_views[3] = create_texture_image(device, 64, 64, 3, &tex_memories[3], &tex_images[3]); // Emissive
    tex_views[4] = create_texture_image(device, 64, 64, 4, &tex_memories[4], &tex_images[4]); // Occlusion

    if (!tex_views[0] || !tex_views[1] || !tex_views[2] || !tex_views[3] || !tex_views[4]) {
        std::printf("Failed to create textures\n");
        return -1;
    }

    // Set textures on material
    kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, tex_views[0], sampler);
    kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_NORMAL, tex_views[1], sampler);
    kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_METALLIC_ROUGHNESS, tex_views[2], sampler);
    kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_EMISSIVE, tex_views[3], sampler);
    kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_OCCLUSION, tex_views[4], sampler);

    // Set material parameters
    float base_color[4] = {0.8f, 0.6f, 0.4f, 1.0f}; // Warm brown
    kxMaterialSetParameter(device, material, "baseColor", base_color, sizeof(base_color));

    float metallic = 0.2f;
    kxMaterialSetParameter(device, material, "metallic", &metallic, sizeof(float));

    float roughness = 0.5f;
    kxMaterialSetParameter(device, material, "roughness", &roughness, sizeof(float));

    float emissive[3] = {0.1f, 0.05f, 0.0f}; // Warm glow
    kxMaterialSetParameter(device, material, "emissive", emissive, sizeof(emissive));

    float normal_scale = 1.0f;
    kxMaterialSetParameter(device, material, "normalScale", &normal_scale, sizeof(float));

    float occlusion = 1.0f;
    kxMaterialSetParameter(device, material, "occlusionStrength", &occlusion, sizeof(float));

    float alpha_cutoff = 0.5f;
    kxMaterialSetParameter(device, material, "alphaCutoff", &alpha_cutoff, sizeof(float));

    // Update descriptor set with material resources
    result = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 0);
    if (result != KX_SUCCESS) {
        std::printf("Failed to update descriptor set: %d\n", result);
        return -1;
    }
    std::printf("Material descriptor set updated\n");

    // Verify parameters
    float get_val;
    kxMaterialGetParameter(device, material, "metallic", &get_val, sizeof(float));
    std::printf("Material metallic: %.2f\n", get_val);

    kxMaterialGetParameter(device, material, "roughness", &get_val, sizeof(float));
    std::printf("Material roughness: %.2f\n", get_val);

    float get_color[4];
    kxMaterialGetParameter(device, material, "baseColor", get_color, sizeof(get_color));
    std::printf("Material baseColor: %.2f, %.2f, %.2f, %.2f\n", get_color[0], get_color[1], get_color[2], get_color[3]);

    // Create vertex buffer - quad (cube face)
    struct Vertex { float pos[3]; float normal[3]; float texcoord[2]; };
    Vertex vertices[6] = {
        {{-1.0f, -1.0f, 0.0f}, {0,0,1}, {0,0}},
        {{ 1.0f, -1.0f, 0.0f}, {0,0,1}, {1,0}},
        {{ 1.0f,  1.0f, 0.0f}, {0,0,1}, {1,1}},
        {{ 1.0f,  1.0f, 0.0f}, {0,0,1}, {1,1}},
        {{-1.0f,  1.0f, 0.0f}, {0,0,1}, {0,1}},
        {{-1.0f, -1.0f, 0.0f}, {0,0,1}, {0,0}},
    };

    KxBufferCreateInfo vb_info{};
    vb_info.sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vb_info.size = sizeof(vertices);
    vb_info.usage = KX_BUFFER_USAGE_VERTEX_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT;
    vb_info.sharingModeExclusive = true;

    KxBuffer vertex_buffer;
    result = kxCreateBuffer(device, &vb_info, nullptr, &vertex_buffer);

    KxMemoryAllocateInfo vb_alloc_info{};
    vb_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vb_alloc_info.allocationSize = sizeof(vertices);
    vb_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory vertex_memory;
    result = kxAllocateMemory(device, &vb_alloc_info, nullptr, &vertex_memory);
    result = kxBindBufferMemory(device, vertex_buffer, vertex_memory, 0);

    void* mapped;
    kxMapMemory(device, vertex_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
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
    kxCreateImage(device, &color_img_info, nullptr, &color_image);

    KxMemoryAllocateInfo color_alloc_info{};
    color_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    color_alloc_info.allocationSize = 256 * 256 * 4;
    color_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory color_memory;
    kxAllocateMemory(device, &color_alloc_info, nullptr, &color_memory);
    kxBindImageMemory(device, color_image, color_memory, 0);

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
    kxCreateImageView(device, &color_view_info, nullptr, &color_view);

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
    kxCreateImage(device, &depth_img_info, nullptr, &depth_image);

    KxMemoryAllocateInfo depth_alloc_info{};
    depth_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    depth_alloc_info.allocationSize = 256 * 256 * 4;
    depth_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory depth_memory;
    kxAllocateMemory(device, &depth_alloc_info, nullptr, &depth_memory);
    kxBindImageMemory(device, depth_image, depth_memory, 0);

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
    kxCreateImageView(device, &depth_view_info, nullptr, &depth_view);

    // Create render pass
    KxAttachmentDescription attachments[2] = {};
    attachments[0].sType = KX_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
    attachments[0].format = KX_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = KX_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = KX_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = KX_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = KX_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = KX_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = KX_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = KX_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_ref;
    subpass.pDepthStencilAttachment = &depth_ref;

    KxSubpassDependency dependency{};
    dependency.srcSubpass = KX_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = KX_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = KX_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    KxRenderPassCreateInfo rp_info{};
    rp_info.sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 2;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 1;
    rp_info.pDependencies = &dependency;

    KxRenderPass render_pass;
    kxCreateRenderPass(device, &rp_info, nullptr, &render_pass);

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
    kxCreateFramebuffer(device, &fb_info, nullptr, &framebuffer);

    // Note: For the built-in PBR shader to work, we need the shader module
    // In this example we'll use the native callback approach like example 03
    // but with the material's descriptor set bound

    // Create command pool
    KxCommandPoolCreateInfo cmd_pool_info{};
    cmd_pool_info.sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.queueFamilyIndex = graphics_queue_family;
    cmd_pool_info.flags = KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    KxCommandPool command_pool;
    kxCreateCommandPool(device, &cmd_pool_info, nullptr, &command_pool);

    // Create command buffer
    KxCommandBufferAllocateInfo cmd_alloc_info{};
    cmd_alloc_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_alloc_info.commandPool = command_pool;
    cmd_alloc_info.level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_alloc_info.commandBufferCount = 1;

    KxCommandBuffer command_buffer;
    kxAllocateCommandBuffers(device, &cmd_alloc_info, &command_buffer);

    // Animation loop
    for (int frame = 0; frame < 60; ++frame) {
        float angle = frame * 0.1f;

        // Model: rotate around Y axis
        Mat4 model = mat4_rotate_y(angle);

        // View: camera at (0, 0, 3) looking at origin
        Mat4 view = mat4_look_at(0, 0, 3, 0, 0, 0, 0, 1, 0);

        // Projection: 45 degree FOV, aspect 1:1, near 0.1, far 100
        Mat4 proj = mat4_perspective(45.0f * 3.14159f / 180.0f, 1.0f, 0.1f, 100.0f);

        // MVP = Proj * View * Model
        Mat4 view_model = mat4_multiply(view, model);
        Mat4 mvp = mat4_multiply(proj, view_model);

        // Begin command buffer
        KxCommandBufferBeginInfo begin_info{};
        begin_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        kxBeginCommandBuffer(command_buffer, &begin_info);

        // Begin render pass
        KxClearValue clear_values[2];
        clear_values[0].color[0] = 0.1f;
        clear_values[0].color[1] = 0.1f;
        clear_values[0].color[2] = 0.15f;
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

        // Bind descriptor set (material)
        kxCmdBindDescriptors(command_buffer, KX_PIPELINE_BIND_POINT_GRAPHICS,
                             pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        // Bind vertex buffer
        KxBuffer vbs[1] = {vertex_buffer};
        uint64_t offsets[1] = {0};
        kxCmdBindVertexBuffers(command_buffer, 0, 1, vbs, offsets);

        // Push constants (MVP matrix)
        kxCmdPushConstants(command_buffer, pipeline_layout, KX_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), &mvp);

        // Draw
        kxCmdDraw(command_buffer, 6, 1, 0, 0);

        // End render pass
        kxCmdEndRenderPass(command_buffer);

        // End command buffer
        kxEndCommandBuffer(command_buffer);

        // Submit
        KxSubmitInfo submit{};
        submit.sType = KX_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &command_buffer;

        kxQueueSubmit(queue, 1, &submit, nullptr);
        kxQueueWaitIdle(queue);

        std::printf("Frame %d: angle=%.2f\n", frame, angle);
    }

    std::printf("Animation complete\n");

    // Cleanup
    kxDestroyFramebuffer(device, framebuffer, nullptr);
    kxDestroyRenderPass(device, render_pass, nullptr);
    kxDestroyImageView(device, depth_view, nullptr);
    kxFreeMemory(device, depth_memory, nullptr);
    kxDestroyImage(device, depth_image, nullptr);
    kxDestroyImageView(device, color_view, nullptr);
    kxFreeMemory(device, color_memory, nullptr);
    kxDestroyImage(device, color_image, nullptr);
    kxFreeMemory(device, vertex_memory, nullptr);
    kxDestroyBuffer(device, vertex_buffer, nullptr);
    kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    kxDestroyCommandPool(device, command_pool, nullptr);

    kxDestroyMaterial(device, material, nullptr);
    kxDestroySampler(device, sampler, nullptr);
    for (int i = 0; i < 5; ++i) {
        kxDestroyImageView(device, tex_views[i], nullptr);
        kxFreeMemory(device, tex_memories[i], nullptr);
        kxDestroyImage(device, tex_images[i], nullptr);
    }
    kxDestroyDescriptorPool(device, descriptor_pool, nullptr);
    kxDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    kxDestroyPipelineLayout(device, pipeline_layout, nullptr);

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);

    return 0;
}