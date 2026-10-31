// Integration test: Material PBR API verification (create/destroy, set/get params, textures, descriptor set)
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>

#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>

// Create a simple colored texture (procedural)
void create_test_texture_data(uint8_t* data, int width, int height, int pattern) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            float u = (float)x / width;
            float v = (float)y / height;

            switch (pattern) {
                case 0: // Checkerboard
                    data[idx] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 255 : 0;
                    data[idx+1] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 255 : 0;
                    data[idx+2] = ((int)(u * 8) + (int)(v * 8)) % 2 ? 255 : 0;
                    data[idx+3] = 255;
                    break;
                case 1: // Red gradient
                    data[idx] = (uint8_t)(u * 255);
                    data[idx+1] = 0;
                    data[idx+2] = (uint8_t)(v * 255);
                    data[idx+3] = 255;
                    break;
                case 2: // Normal map style (blue-ish)
                    data[idx] = 128;
                    data[idx+1] = 128;
                    data[idx+2] = 255;
                    data[idx+3] = 255;
                    break;
                case 3: // Metallic/roughness (green=roughness, blue=metallic)
                    data[idx] = 0;
                    data[idx+1] = (uint8_t)(u * 255);  // roughness
                    data[idx+2] = (uint8_t)(v * 255);  // metallic
                    data[idx+3] = 255;
                    break;
                default:
                    data[idx] = 128; data[idx+1] = 128; data[idx+2] = 128; data[idx+3] = 255;
            }
        }
    }
}

// Create and upload a texture
KxImageView create_texture_image(KxDevice device, int width, int height, int pattern,
                                 KxDeviceMemory* out_memory, KxImage* out_image) {
    // Create image
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
    REQUIRE(res == KX_SUCCESS);

    // Allocate memory
    KxMemoryAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = width * height * 4;
    alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory memory;
    res = kxAllocateMemory(device, &alloc_info, nullptr, &memory);
    REQUIRE(res == KX_SUCCESS);
    res = kxBindImageMemory(device, image, memory, 0);
    REQUIRE(res == KX_SUCCESS);

    // Generate texture data and upload
    std::vector<uint8_t> tex_data(width * height * 4);
    create_test_texture_data(tex_data.data(), width, height, pattern);

    void* mapped;
    res = kxMapMemory(device, memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    REQUIRE(res == KX_SUCCESS);
    memcpy(mapped, tex_data.data(), tex_data.size());
    kxUnmapMemory(device, memory);

    // Create image view
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
    REQUIRE(res == KX_SUCCESS);

    if (out_memory) *out_memory = memory;
    if (out_image) *out_image = image;

    return view;
}

// Create a default sampler
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
    REQUIRE(res == KX_SUCCESS);
    return sampler;
}

TEST_CASE("material_pbr_basic") {
    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "material_pbr_test";
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

    KxQueue queue;
    kxGetDeviceQueue(device, graphics_queue_family, 0, &queue);

    // Create descriptor pool and set layout for material
    // Material expects: set 0, binding 0 = uniform, bindings 1-5 = textures
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
    KxResult res = kxCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
    REQUIRE(res == KX_SUCCESS);

    KxDescriptorSetLayoutBinding bindings[6] = {};
    // Binding 0: Uniform buffer
    bindings[0].binding = 0;
    bindings[0].descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = KX_SHADER_STAGE_VERTEX_BIT | KX_SHADER_STAGE_FRAGMENT_BIT;
    // Bindings 1-5: Textures
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
    res = kxCreateDescriptorSetLayout(device, &set_layout_info, nullptr, &descriptor_set_layout);
    REQUIRE(res == KX_SUCCESS);

    // Allocate descriptor set
    KxDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    KxDescriptorSet descriptor_set;
    res = kxAllocateDescriptorSets(device, &alloc_info, &descriptor_set);
    REQUIRE(res == KX_SUCCESS);

    // Create pipeline layout
    KxPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &descriptor_set_layout;
    layout_info.pushConstantRangeCount = 0;
    layout_info.pPushConstantRanges = nullptr;

    KxPipelineLayout pipeline_layout;
    res = kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
    REQUIRE(res == KX_SUCCESS);

    // Create default material (built-in PBR shader)
    KxMaterial material;
    res = kxCreateDefaultMaterial(device, nullptr, &material);
    REQUIRE(res == KX_SUCCESS);
    REQUIRE(material != nullptr);

    // Create test textures
    KxDeviceMemory tex_memories[5];
    KxImage tex_images[5];
    KxImageView tex_views[5];
    KxSampler sampler = create_default_sampler(device);

    tex_views[0] = create_texture_image(device, 64, 64, 1, &tex_memories[0], &tex_images[0]); // Albedo (red gradient)
    tex_views[1] = create_texture_image(device, 64, 64, 2, &tex_memories[1], &tex_images[1]); // Normal (blue)
    tex_views[2] = create_texture_image(device, 64, 64, 3, &tex_memories[2], &tex_images[2]); // Metallic/roughness
    tex_views[3] = create_texture_image(device, 64, 64, 0, &tex_memories[3], &tex_images[3]); // Emissive (checkerboard)
    tex_views[4] = create_texture_image(device, 64, 64, 0, &tex_memories[4], &tex_images[4]); // Occlusion (checkerboard)

    // Set textures on material
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, tex_views[0], sampler);
    REQUIRE(res == KX_SUCCESS);
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_NORMAL, tex_views[1], sampler);
    REQUIRE(res == KX_SUCCESS);
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_METALLIC_ROUGHNESS, tex_views[2], sampler);
    REQUIRE(res == KX_SUCCESS);
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_EMISSIVE, tex_views[3], sampler);
    REQUIRE(res == KX_SUCCESS);
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_OCCLUSION, tex_views[4], sampler);
    REQUIRE(res == KX_SUCCESS);

    // Verify textures were set
    KxImageView get_view;
    KxSampler get_sampler;
    res = kxMaterialGetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, &get_view, &get_sampler);
    REQUIRE(res == KX_SUCCESS);
    CHECK(get_view == tex_views[0]);
    CHECK(get_sampler == sampler);

    // Update descriptor set with material resources
    res = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 0);
    REQUIRE(res == KX_SUCCESS);

    // Test setting/getting material parameters
    float new_metallic = 0.8f;
    res = kxMaterialSetParameter(device, material, "metallic", &new_metallic, sizeof(float));
    REQUIRE(res == KX_SUCCESS);

    float get_metallic = 0.0f;
    res = kxMaterialGetParameter(device, material, "metallic", &get_metallic, sizeof(float));
    REQUIRE(res == KX_SUCCESS);
    CHECK(get_metallic == doctest::Approx(0.8f).epsilon(0.01f));

    float new_roughness = 0.3f;
    res = kxMaterialSetParameter(device, material, "roughness", &new_roughness, sizeof(float));
    REQUIRE(res == KX_SUCCESS);

    float get_roughness = 0.0f;
    res = kxMaterialGetParameter(device, material, "roughness", &get_roughness, sizeof(float));
    REQUIRE(res == KX_SUCCESS);
    CHECK(get_roughness == doctest::Approx(0.3f).epsilon(0.01f));

    float new_base_color[4] = {1.0f, 0.5f, 0.2f, 1.0f};
    res = kxMaterialSetParameter(device, material, "baseColor", new_base_color, sizeof(new_base_color));
    REQUIRE(res == KX_SUCCESS);

    float get_base_color[4] = {0, 0, 0, 0};
    res = kxMaterialGetParameter(device, material, "baseColor", get_base_color, sizeof(get_base_color));
    REQUIRE(res == KX_SUCCESS);
    CHECK(get_base_color[0] == doctest::Approx(1.0f).epsilon(0.01f));
    CHECK(get_base_color[1] == doctest::Approx(0.5f).epsilon(0.01f));
    CHECK(get_base_color[2] == doctest::Approx(0.2f).epsilon(0.01f));
    CHECK(get_base_color[3] == doctest::Approx(1.0f).epsilon(0.01f));

    float new_emissive[3] = {0.1f, 0.05f, 0.02f};
    res = kxMaterialSetParameter(device, material, "emissive", new_emissive, sizeof(new_emissive));
    REQUIRE(res == KX_SUCCESS);

    float new_normal_scale = 2.0f;
    res = kxMaterialSetParameter(device, material, "normalScale", &new_normal_scale, sizeof(float));
    REQUIRE(res == KX_SUCCESS);

    float new_occlusion = 0.5f;
    res = kxMaterialSetParameter(device, material, "occlusionStrength", &new_occlusion, sizeof(float));
    REQUIRE(res == KX_SUCCESS);

    float new_alpha_cutoff = 0.5f;
    res = kxMaterialSetParameter(device, material, "alphaCutoff", &new_alpha_cutoff, sizeof(float));
    REQUIRE(res == KX_SUCCESS);

    // Re-update descriptor set after parameter changes
    res = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 0);
    REQUIRE(res == KX_SUCCESS);

    // Test error handling: invalid parameter name
    res = kxMaterialSetParameter(device, material, "invalidParameter", &new_metallic, sizeof(float));
    CHECK(res != KX_SUCCESS);

    res = kxMaterialGetParameter(device, material, "invalidParameter", &get_metallic, sizeof(float));
    CHECK(res != KX_SUCCESS);

    // Test setting texture to NULL (clear slot)
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, nullptr, nullptr);
    REQUIRE(res == KX_SUCCESS);

    res = kxMaterialGetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, &get_view, &get_sampler);
    REQUIRE(res == KX_SUCCESS);
    CHECK(get_view == nullptr);
    CHECK(get_sampler == nullptr);

    // Restore texture
    res = kxMaterialSetTexture(device, material, KX_MATERIAL_TEXTURE_BASE_COLOR, tex_views[0], sampler);
    REQUIRE(res == KX_SUCCESS);

    // Update descriptor set again
    res = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 0);
    REQUIRE(res == KX_SUCCESS);

    // Cleanup material
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
}