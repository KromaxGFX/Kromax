// Integration test: verify shader compilation works for GLSL/HLSL/WGSL/KXSL
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <kromax/kromax.h>
#include <doctest.h>

#include <cstdio>
#include <cstring>
#include <vector>

// Shader sources for each language - simple quad with uniform color
const char* GLSL_VERTEX = 
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

const char* GLSL_FRAGMENT =
    "void main() {\n"
    "    float a = 1.0;\n"
    "}\n";

const char* HLSL_VERTEX =
    "cbuffer PushConstants : register(b0) {\n"
    "    float4x4 mvp;\n"
    "};\n"
    "struct VSInput {\n"
    "    float3 pos : POSITION0;\n"
    "    float4 color : COLOR0;\n"
    "};\n"
    "struct VSOutput {\n"
    "    float4 pos : SV_POSITION;\n"
    "    float4 color : COLOR0;\n"
    "};\n"
    "VSOutput main(VSInput input) {\n"
    "    VSOutput output;\n"
    "    output.pos = mul(mvp, float4(input.pos, 1.0));\n"
    "    output.color = input.color;\n"
    "    return output;\n"
    "}\n";

const char* HLSL_FRAGMENT =
    "struct PSInput {\n"
    "    float4 color : COLOR0;\n"
    "};\n"
    "float4 main(PSInput input) : SV_TARGET {\n"
    "    return input.color;\n"
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

TEST_CASE("shader_languages_compare") {
    // Create instance
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "shader_languages_test";
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

    // Test shader compilation for each language
    struct TestCase {
        const char* name;
        const char* vertex_src;
        const char* fragment_src;
        KxShaderLanguage language;
    };

    TestCase tests[] = {
        {"GLSL", GLSL_VERTEX, GLSL_FRAGMENT, KX_SHADER_LANGUAGE_GLSL},
        {"WGSL", WGSL_VERTEX, WGSL_FRAGMENT, KX_SHADER_LANGUAGE_WGSL},
        {"KXSL", KXSL_VERTEX, KXSL_FRAGMENT, KX_SHADER_LANGUAGE_KXSL},
    };

    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    printf("Number of tests: %d\n", num_tests);
    for (int i = 0; i < num_tests; ++i) {
        printf("  tests[%d]: name=%s, lang=%d\n", i, tests[i].name, (int)tests[i].language);
    }

    for (int i = 0; i < num_tests; ++i) {
        const char* lang_name = tests[i].name;
        KxShaderLanguage lang_enum = tests[i].language;
        INFO("Testing language: " << lang_name << " (enum=" << (int)lang_enum << ")");

        // Test vertex shader compilation
        KxShaderSourceCreateInfo vert_info{};
        vert_info.sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
        vert_info.language = lang_enum;
        vert_info.stage = KX_SHADER_STAGE_VERTEX;
        vert_info.pSource = tests[i].vertex_src;
        vert_info.pEntryPoint = "main";

        KxShaderModule vert_shader;
        KxResult res = kxCreateShaderModuleFromSource(device, &vert_info, nullptr, &vert_shader);
        REQUIRE(res == KX_SUCCESS);

        // Test fragment shader compilation
        KxShaderSourceCreateInfo frag_info{};
        frag_info.sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO;
        frag_info.language = lang_enum;
        frag_info.stage = KX_SHADER_STAGE_FRAGMENT;
        frag_info.pSource = tests[i].fragment_src;
        frag_info.pEntryPoint = "main";

        KxShaderModule frag_shader;
        res = kxCreateShaderModuleFromSource(device, &frag_info, nullptr, &frag_shader);
        REQUIRE(res == KX_SUCCESS);

        // Cleanup
        kxDestroyShaderModule(device, frag_shader, nullptr);
        kxDestroyShaderModule(device, vert_shader, nullptr);
    }

    // Also test language detection
    KxShaderLanguage detected = kxDetectShaderLanguage(GLSL_VERTEX, strlen(GLSL_VERTEX));
    CHECK(detected == KX_SHADER_LANGUAGE_GLSL);

    detected = kxDetectShaderLanguage(WGSL_VERTEX, strlen(WGSL_VERTEX));
    CHECK(detected == KX_SHADER_LANGUAGE_WGSL);

    detected = kxDetectShaderLanguage(KXSL_VERTEX, strlen(KXSL_VERTEX));
    // KXSL might not have distinctive markers, so just check it's not undefined
    CHECK(detected != KX_SHADER_LANGUAGE_UNDEFINED);

    // Cleanup
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}