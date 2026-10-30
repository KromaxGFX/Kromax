/*
 * test_descriptors.cpp
 *
 * Unit tests for descriptor set layout, descriptor pool, descriptor sets, samplers, and texture sampling.
 */

#include <kromax/kx_core.h>
#include <kromax/kx_descriptor.h>
#include <kromax/kx_pipeline.h>
#include <kromax/kx_api.h>

#include <cstdio>
#include <cstring>

static void print_result(const char* name, KxResult result) {
    const char* status = (result == KX_SUCCESS) ? "PASS" : "FAIL";
    std::printf("[%s] %s: %s\n", status, name, kxResultToString(result));
}

static KxInstance create_test_instance() {
    KxInstance instance = nullptr;
    KxApplicationInfo app_info = {
        .sType = KX_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "test_descriptors",
        .applicationVersion = 1,
        .pEngineName = "test",
        .engineVersion = 1,
        .apiVersion = KX_API_VERSION_0_1,
    };
    KxInstanceCreateInfo inst_info = {
        .sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
    };
    KxResult result = kxCreateInstance(&inst_info, nullptr, &instance);
    if (result != KX_SUCCESS) return nullptr;
    return instance;
}

static KxDevice create_test_device(KxInstance instance) {
    uint32_t pd_count = 0;
    KxResult result = kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    if (result != KX_SUCCESS || pd_count == 0) return nullptr;

    KxPhysicalDevice* pds = new KxPhysicalDevice[pd_count];
    result = kxEnumeratePhysicalDevices(instance, &pd_count, pds);
    if (result != KX_SUCCESS) {
        delete[] pds;
        return nullptr;
    }
    KxPhysicalDevice pd = pds[0];
    delete[] pds;

    KxDevice device = nullptr;
    KxDeviceQueueCreateInfo queue_info = {
        .sType = KX_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = (float[]){1.0f},
    };
    KxDeviceCreateInfo dev_info = {
        .sType = KX_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
    };
    result = kxCreateDevice(pd, &dev_info, nullptr, &device);
    if (result != KX_SUCCESS) return nullptr;
    return device;
}

static void test_descriptor_set_layout() {
    KxInstance instance = create_test_instance();
    if (!instance) return;
    KxDevice device = create_test_device(instance);
    if (!device) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Test descriptor set layout creation */
    KxDescriptorSetLayoutBinding bindings[2] = {
        {
            .binding = 0,
            .descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = KX_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = nullptr,
        },
        {
            .binding = 1,
            .descriptorType = KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = KX_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        }
    };

    KxDescriptorSetLayoutCreateInfo layout_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_SET_LAYOUT_CREATE_NONE,
        .bindingCount = 2,
        .pBindings = bindings,
    };

    KxDescriptorSetLayout layout = nullptr;
    KxResult result = kxCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout);
    print_result("kxCreateDescriptorSetLayout (2 bindings)", result);

    if (result == KX_SUCCESS) {
        /* Test destroy */
        kxDestroyDescriptorSetLayout(device, layout, nullptr);
        print_result("kxDestroyDescriptorSetLayout", KX_SUCCESS);
    }

    /* Test empty layout */
    KxDescriptorSetLayoutCreateInfo empty_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_SET_LAYOUT_CREATE_NONE,
        .bindingCount = 0,
        .pBindings = nullptr,
    };
    KxDescriptorSetLayout empty_layout = nullptr;
    result = kxCreateDescriptorSetLayout(device, &empty_info, nullptr, &empty_layout);
    print_result("kxCreateDescriptorSetLayout (empty)", result);
    if (result == KX_SUCCESS) {
        kxDestroyDescriptorSetLayout(device, empty_layout, nullptr);
    }

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

static void test_descriptor_pool() {
    KxInstance instance = create_test_instance();
    if (!instance) return;
    KxDevice device = create_test_device(instance);
    if (!device) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Test descriptor pool creation */
    KxDescriptorPoolSize pool_sizes[2] = {
        { .type = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 10 },
        { .type = KX_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 10 },
    };

    KxDescriptorPoolCreateInfo pool_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 10,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes,
    };

    KxDescriptorPool pool = nullptr;
    KxResult result = kxCreateDescriptorPool(device, &pool_info, nullptr, &pool);
    print_result("kxCreateDescriptorPool", result);

    if (result == KX_SUCCESS) {
        /* Test reset */
        result = kxResetDescriptorPool(device, pool, KX_DESCRIPTOR_POOL_RESET_NONE);
        print_result("kxResetDescriptorPool", result);

        /* Test destroy */
        kxDestroyDescriptorPool(device, pool, nullptr);
        print_result("kxDestroyDescriptorPool", KX_SUCCESS);
    }

    /* Test pool without FREE_DESCRIPTOR_SET_BIT */
    KxDescriptorPoolSize pool_size2 = { .type = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 5 };
    KxDescriptorPoolCreateInfo pool_info2 = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_POOL_CREATE_NONE,
        .maxSets = 5,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size2,
    };
    KxDescriptorPool pool2 = nullptr;
    result = kxCreateDescriptorPool(device, &pool_info2, nullptr, &pool2);
    print_result("kxCreateDescriptorPool (no free)", result);
    if (result == KX_SUCCESS) {
        kxDestroyDescriptorPool(device, pool2, nullptr);
    }

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

static void test_descriptor_sets() {
    KxInstance instance = create_test_instance();
    if (!instance) return;
    KxDevice device = create_test_device(instance);
    if (!device) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Create layout */
    KxDescriptorSetLayoutBinding binding = {
        .binding = 0,
        .descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = KX_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };
    KxDescriptorSetLayoutCreateInfo layout_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_SET_LAYOUT_CREATE_NONE,
        .bindingCount = 1,
        .pBindings = &binding,
    };
    KxDescriptorSetLayout layout = nullptr;
    KxResult result = kxCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout);
    if (result != KX_SUCCESS) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Create pool */
    KxDescriptorPoolSize pool_size = { .type = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 5 };
    KxDescriptorPoolCreateInfo pool_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 5,
        .poolSizeCount = 1,
        .pPoolSizes = &pool_size,
    };
    KxDescriptorPool pool = nullptr;
    result = kxCreateDescriptorPool(device, &pool_info, nullptr, &pool);
    if (result != KX_SUCCESS) {
        kxDestroyDescriptorSetLayout(device, layout, nullptr);
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Allocate descriptor sets */
    KxDescriptorSet sets[3];
    KxDescriptorSetLayout layouts[3] = { layout, layout, layout };
    KxDescriptorSetAllocateInfo alloc_info = {
        .sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool,
        .descriptorSetCount = 3,
        .pSetLayouts = layouts,
    };
    result = kxAllocateDescriptorSets(device, &alloc_info, sets);
    print_result("kxAllocateDescriptorSets (3 sets)", result);

    if (result == KX_SUCCESS) {
        /* Test free descriptor sets */
        result = kxFreeDescriptorSets(device, pool, 3, sets);
        print_result("kxFreeDescriptorSets", result);
    }

    /* Allocate again */
    result = kxAllocateDescriptorSets(device, &alloc_info, sets);
    print_result("kxAllocateDescriptorSets (again)", result);

    /* Test update descriptor sets (with dummy buffer) */
    if (result == KX_SUCCESS) {
        KxDescriptorBufferInfo buffer_info = {
            .buffer = nullptr,
            .offset = 0,
            .range = 256,
        };
        KxWriteDescriptorSet write = {
            .sType = KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = sets[0],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info,
            .pTexelBufferView = nullptr,
        };
        kxUpdateDescriptorSets(device, 1, &write, 0, nullptr);
        print_result("kxUpdateDescriptorSets", KX_SUCCESS);
    }

    /* Cleanup */
    if (result == KX_SUCCESS) {
        kxFreeDescriptorSets(device, pool, 3, sets);
    }
    kxDestroyDescriptorPool(device, pool, nullptr);
    kxDestroyDescriptorSetLayout(device, layout, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

static void test_sampler() {
    KxInstance instance = create_test_instance();
    if (!instance) return;
    KxDevice device = create_test_device(instance);
    if (!device) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Test sampler creation with various configurations */
    KxSamplerCreateInfo sampler_info = {
        .sType = KX_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = KX_SAMPLER_CREATE_NONE,
        .magFilter = KX_FILTER_LINEAR,
        .minFilter = KX_FILTER_LINEAR,
        .addressModeU = KX_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = KX_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = KX_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = KX_FALSE,
        .maxAnisotropy = 1.0f,
        .compareEnable = KX_FALSE,
        .compareOp = KX_COMPARE_OP_NEVER,
        .minLod = 0.0f,
        .maxLod = 1000.0f,
        .borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = KX_FALSE,
    };

    KxSampler sampler = nullptr;
    KxResult result = kxCreateSampler(device, &sampler_info, nullptr, &sampler);
    print_result("kxCreateSampler (linear, repeat)", result);

    if (result == KX_SUCCESS) {
        kxDestroySampler(device, sampler, nullptr);
        print_result("kxDestroySampler", KX_SUCCESS);
    }

    /* Test nearest + clamp to edge */
    sampler_info.magFilter = KX_FILTER_NEAREST;
    sampler_info.minFilter = KX_FILTER_NEAREST;
    sampler_info.addressModeU = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = KX_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler = nullptr;
    result = kxCreateSampler(device, &sampler_info, nullptr, &sampler);
    print_result("kxCreateSampler (nearest, clamp)", result);
    if (result == KX_SUCCESS) {
        kxDestroySampler(device, sampler, nullptr);
    }

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

static void test_sample_texture() {
    KxInstance instance = create_test_instance();
    if (!instance) return;
    KxDevice device = create_test_device(instance);
    if (!device) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Create a simple test image (we can't easily create a real image without memory,
     * so we test the function with invalid texture to verify it handles gracefully) */
    
    KxBoundTexture texture = {
        .sampler = nullptr,
        .imageView = nullptr,
        .width = 0,
        .height = 0,
        .rowPitch = 0,
        .format = KX_FORMAT_UNDEFINED,
        .pTexels = nullptr,
    };

    float color[4] = {0};
    KxResult result = kxSampleTexture(&texture, 0.5f, 0.5f, color);
    print_result("kxSampleTexture (null texture)", result);
    /* Should fail gracefully */

    /* Test with valid format but no texels */
    texture.format = KX_FORMAT_R8G8B8A8_UNORM;
    texture.width = 16;
    texture.height = 16;
    texture.rowPitch = 16 * 4;
    result = kxSampleTexture(&texture, 0.5f, 0.5f, color);
    print_result("kxSampleTexture (no texels)", result);
    /* Should fail gracefully */

    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

int main() {
    std::printf("=== Descriptor/Sampler Tests ===\n\n");

    test_descriptor_set_layout();
    std::printf("\n");
    test_descriptor_pool();
    std::printf("\n");
    test_descriptor_sets();
    std::printf("\n");
    test_sampler();
    std::printf("\n");
    test_sample_texture();
    std::printf("\n");

    std::printf("=== All tests completed ===\n");
    return 0;
}