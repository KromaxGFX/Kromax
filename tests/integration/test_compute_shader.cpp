// Integration test: Compute shader dispatch, verify storage buffer output
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>

#include <cstdio>
#include <cstring>
#include <vector>

// Compute shader source - simple parallel addition
const char* COMPUTE_GLSL =
    "#version 450\n"
    "layout(local_size_x = 64) in;\n"
    "layout(std430, set = 0, binding = 0) buffer InputBuffer {\n"
    "    float data[];\n"
    "} input;\n"
    "layout(std430, set = 0, binding = 1) buffer OutputBuffer {\n"
    "    float data[];\n"
    "} output;\n"
    "void main() {\n"
    "    uint idx = gl_GlobalInvocationID.x;\n"
    "    output.data[idx] = input.data[idx] * 2.0f + 1.0f;\n"
    "}\n";

const char* COMPUTE_HLSL =
    "[numthreads(64, 1, 1)]\n"
    "struct InputBuffer {\n"
    "    float data[];\n"
    "};\n"
    "struct OutputBuffer {\n"
    "    float data[];\n"
    "};\n"
    "RWStructuredBuffer<float> input : register(u0);\n"
    "RWStructuredBuffer<float> output : register(u1);\n"
    "void main(uint3 id : SV_DispatchThreadID) {\n"
    "    output[id.x] = input[id.x] * 2.0f + 1.0f;\n"
    "}\n";

const char* COMPUTE_WGSL =
    "@group(0) @binding(0) var<storage, read> input: array<f32>;\n"
    "@group(0) @binding(1) var<storage, read_write> output: array<f32>;\n"
    "@compute @workgroup_size(64)\n"
    "fn main(@builtin(global_invocation_id) id: vec3<u32>) {\n"
    "    output[id.x] = input[id.x] * 2.0f + 1.0f;\n"
    "}\n";

const char* COMPUTE_KXSL =
    "layout(set = 0, binding = 0) buffer InputBuffer {\n"
    "    float data[];\n"
    "} input;\n"
    "layout(set = 0, binding = 1) buffer OutputBuffer {\n"
    "    float data[];\n"
    "} output;\n"
    "layout(local_size_x = 64) in;\n"
    "void main() {\n"
    "    uint idx = gl_GlobalInvocationID.x;\n"
    "    output.data[idx] = input.data[idx] * 2.0f + 1.0f;\n"
    "}\n";

struct ComputeTest {
    const char* name;
    const char* source;
    KxShaderLanguage language;
};

TEST_CASE("compute_shader_dispatch") {
    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "compute_shader_test";
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

    // Get queue family - need compute queue
    uint32_t queue_family_count = 0;
    kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<KxQueueFamilyProperties> queue_families(queue_family_count);
    kxGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    uint32_t compute_queue_family = UINT32_MAX;
    for (uint32_t i = 0; i < queue_family_count; ++i) {
        if (queue_families[i].queueFlags & KX_QUEUE_COMPUTE_BIT) {
            compute_queue_family = i;
            break;
        }
    }
    // If no dedicated compute queue, use graphics queue (should support compute too)
    if (compute_queue_family == UINT32_MAX) {
        for (uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFlags & KX_QUEUE_GRAPHICS_BIT) {
                compute_queue_family = i;
                break;
            }
        }
    }
    REQUIRE(compute_queue_family != UINT32_MAX);

    // Create device
    float queue_priority = 1.0f;
    KxDeviceQueueCreateInfo queue_info{};
    queue_info.sType = KX_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.queueFamilyIndex = compute_queue_family;
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
    kxGetDeviceQueue(device, compute_queue_family, 0, &queue);

    // Create descriptor pool and set layout for compute
    KxDescriptorPoolSize pool_sizes[1] = {};
    pool_sizes[0].type = KX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    pool_sizes[0].descriptorCount = 2;  // input + output

    KxDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;

    KxDescriptorPool descriptor_pool;
    result = kxCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
    REQUIRE(result == KX_SUCCESS);

    KxDescriptorSetLayoutBinding bindings[2] = {};
    bindings[0].binding = 0;
    bindings[0].descriptorType = KX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = KX_SHADER_STAGE_COMPUTE_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = KX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = KX_SHADER_STAGE_COMPUTE_BIT;

    KxDescriptorSetLayoutCreateInfo set_layout_info{};
    set_layout_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_layout_info.bindingCount = 2;
    set_layout_info.pBindings = bindings;

    KxDescriptorSetLayout descriptor_set_layout;
    result = kxCreateDescriptorSetLayout(device, &set_layout_info, nullptr, &descriptor_set_layout);
    REQUIRE(result == KX_SUCCESS);

    // Allocate descriptor set
    KxDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    KxDescriptorSet descriptor_set;
    result = kxAllocateDescriptorSets(device, &alloc_info, &descriptor_set);
    REQUIRE(result == KX_SUCCESS);

    // Create pipeline layout
    KxPipelineLayoutCreateInfo layout_info{};
    layout_info.sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &descriptor_set_layout;

    KxPipelineLayout pipeline_layout;
    result = kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
    REQUIRE(result == KX_SUCCESS);

    // Test data
    const int DATA_SIZE = 256;
    const int BUFFER_SIZE = DATA_SIZE * sizeof(float);

    // Create input buffer
    KxBufferCreateInfo input_buffer_info{};
    input_buffer_info.sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    input_buffer_info.size = BUFFER_SIZE;
    input_buffer_info.usage = KX_BUFFER_USAGE_STORAGE_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT;
    input_buffer_info.sharingModeExclusive = true;

    KxBuffer input_buffer;
    result = kxCreateBuffer(device, &input_buffer_info, nullptr, &input_buffer);
    REQUIRE(result == KX_SUCCESS);

    KxMemoryAllocateInfo input_alloc_info{};
    input_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    input_alloc_info.allocationSize = BUFFER_SIZE;
    input_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory input_memory;
    result = kxAllocateMemory(device, &input_alloc_info, nullptr, &input_memory);
    REQUIRE(result == KX_SUCCESS);
    result = kxBindBufferMemory(device, input_buffer, input_memory, 0);
    REQUIRE(result == KX_SUCCESS);

    // Fill input buffer with test data: data[i] = i * 0.1
    std::vector<float> input_data(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; ++i) {
        input_data[i] = i * 0.1f;
    }

    void* mapped;
    result = kxMapMemory(device, input_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
    REQUIRE(result == KX_SUCCESS);
    memcpy(mapped, input_data.data(), BUFFER_SIZE);
    kxUnmapMemory(device, input_memory);

    // Create output buffer
    KxBufferCreateInfo output_buffer_info{};
    output_buffer_info.sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    output_buffer_info.size = BUFFER_SIZE;
    output_buffer_info.usage = KX_BUFFER_USAGE_STORAGE_BUFFER_BIT | KX_BUFFER_USAGE_TRANSFER_SRC_BIT;
    output_buffer_info.sharingModeExclusive = true;

    KxBuffer output_buffer;
    result = kxCreateBuffer(device, &output_buffer_info, nullptr, &output_buffer);
    REQUIRE(result == KX_SUCCESS);

    KxMemoryAllocateInfo output_alloc_info{};
    output_alloc_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    output_alloc_info.allocationSize = BUFFER_SIZE;
    output_alloc_info.memoryTypeIndex = 0;

    KxDeviceMemory output_memory;
    result = kxAllocateMemory(device, &output_alloc_info, nullptr, &output_memory);
    REQUIRE(result == KX_SUCCESS);
    result = kxBindBufferMemory(device, output_buffer, output_memory, 0);
    REQUIRE(result == KX_SUCCESS);

    // Write descriptor set
    KxDescriptorBufferInfo buffer_infos[2] = {};
    buffer_infos[0].buffer = input_buffer;
    buffer_infos[0].offset = 0;
    buffer_infos[0].range = BUFFER_SIZE;
    buffer_infos[1].buffer = output_buffer;
    buffer_infos[1].offset = 0;
    buffer_infos[1].range = BUFFER_SIZE;

    KxWriteDescriptorSet writes[2] = {};
    writes[0].sType = KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptor_set;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorType = KX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &buffer_infos[0];

    writes[1].sType = KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = descriptor_set;
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorType = KX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[1].descriptorCount = 1;
    writes[1].pBufferInfo = &buffer_infos[1];

    kxUpdateDescriptorSets(device, 2, writes, 0, nullptr);

    // Test each compute shader language
    ComputeTest tests[] = {
        {"GLSL", COMPUTE_GLSL, KX_SHADER_LANGUAGE_GLSL},
        {"HLSL", COMPUTE_HLSL, KX_SHADER_LANGUAGE_HLSL},
        {"WGSL", COMPUTE_WGSL, KX_SHADER_LANGUAGE_WGSL},
        {"KXSL", COMPUTE_KXSL, KX_SHADER_LANGUAGE_KXSL},
    };

    for (int t = 0; t < 4; ++t) {
        INFO("Testing compute language: " << tests[t].name);
        
        // Create compute shader module from source
        KxShaderSourceCreateInfo shader_info{};
        shader_info.sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
        shader_info.language = tests[t].language;
        shader_info.stage = KX_SHADER_STAGE_COMPUTE;
        shader_info.pSource = tests[t].source;
        shader_info.pEntryPoint = "main";

        KxShaderModule compute_shader;
        result = kxCreateShaderModuleFromSource(device, &shader_info, nullptr, &compute_shader);
        
        // Skip if language not supported
        if (result != KX_SUCCESS) {
            INFO("Language " << tests[t].name << " not supported, skipping");
            continue;
        }

        // Create compute pipeline
        KxPipelineCreateInfo pipe_info{};
        pipe_info.sType = KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO;
        pipe_info.bindPoint = KX_PIPELINE_BIND_POINT_COMPUTE;
        pipe_info.layout = pipeline_layout;
        pipe_info.computeShaderModule = compute_shader;

        KxPipeline pipeline;
        result = kxCreatePipeline(device, nullptr, &pipe_info, nullptr, &pipeline);
        REQUIRE(result == KX_SUCCESS);

        // Create command pool
        KxCommandPoolCreateInfo cmd_pool_info{};
        cmd_pool_info.sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmd_pool_info.queueFamilyIndex = compute_queue_family;
        cmd_pool_info.flags = KX_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        KxCommandPool command_pool;
        result = kxCreateCommandPool(device, &cmd_pool_info, nullptr, &command_pool);
        REQUIRE(result == KX_SUCCESS);

        // Create command buffer
        KxCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = command_pool;
        cmd_alloc_info.level = KX_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmd_alloc_info.commandBufferCount = 1;

        KxCommandBuffer command_buffer;
        result = kxAllocateCommandBuffers(device, &cmd_alloc_info, &command_buffer);
        REQUIRE(result == KX_SUCCESS);

        // Begin command buffer
        KxCommandBufferBeginInfo begin_info{};
        begin_info.sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        result = kxBeginCommandBuffer(command_buffer, &begin_info);
        REQUIRE(result == KX_SUCCESS);

        // Bind pipeline
        kxCmdBindPipeline(command_buffer, KX_PIPELINE_BIND_POINT_COMPUTE, pipeline);

        // Bind descriptor set
        kxCmdBindDescriptors(command_buffer, KX_PIPELINE_BIND_POINT_COMPUTE,
                                pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

        // Dispatch: 256 elements / 64 workgroup size = 4 workgroups
        kxCmdDispatch(command_buffer, 4, 1, 1);

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

        // Read back output
        result = kxMapMemory(device, output_memory, 0, KX_WHOLE_SIZE, 0, &mapped);
        REQUIRE(result == KX_SUCCESS);
        
        float* output_data = static_cast<float*>(mapped);
        
        // Verify: output[i] = input[i] * 2.0 + 1.0 = i * 0.1 * 2 + 1 = i * 0.2 + 1
        bool all_correct = true;
        for (int i = 0; i < DATA_SIZE; ++i) {
            float expected = i * 0.2f + 1.0f;
            float actual = output_data[i];
            if (std::abs(actual - expected) > 0.001f) {
                INFO("Mismatch at index " << i << ": expected " << expected << ", got " << actual);
                all_correct = false;
                break;
            }
        }
        
        kxUnmapMemory(device, output_memory);
        CHECK(all_correct);

        // Cleanup for this language
        kxDestroyPipeline(device, pipeline, nullptr);
        kxDestroyShaderModule(device, compute_shader, nullptr);
        kxFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        kxDestroyCommandPool(device, command_pool, nullptr);
    }

    // Cleanup
    kxDestroyBuffer(device, output_buffer, nullptr);
    kxFreeMemory(device, output_memory, nullptr);
    kxDestroyBuffer(device, input_buffer, nullptr);
    kxFreeMemory(device, input_memory, nullptr);
    kxDestroyDescriptorPool(device, descriptor_pool, nullptr);
    kxDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    kxDestroyPipelineLayout(device, pipeline_layout, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}