/*
 * test_buffer_image.cpp
 *
 * Unit tests for buffer and image creation, binding, and mapping.
 */

#include <kromax/kx_core.h>

#include <cstdio>
#include <cstring>

static void print_result(const char* name, KxResult result) {
    const char* status = (result == KX_SUCCESS) ? "PASS" : "FAIL";
    std::printf("[%s] %s: %s\n", status, name, kxResultToString(result));
}

static void test_buffer_lifecycle() {
    KxInstance instance = nullptr;
    KxApplicationInfo app_info = {
        .sType = KX_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "test",
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
    print_result("kxCreateInstance", result);
    if (result != KX_SUCCESS) return;

    uint32_t pd_count = 0;
    KxPhysicalDevice pd = nullptr;
    result = kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    print_result("kxEnumeratePhysicalDevices (count)", result);
    if (result != KX_SUCCESS || pd_count == 0) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    KxPhysicalDevice* pds = new KxPhysicalDevice[pd_count];
    result = kxEnumeratePhysicalDevices(instance, &pd_count, pds);
    print_result("kxEnumeratePhysicalDevices (get)", result);
    if (result != KX_SUCCESS) {
        delete[] pds;
        kxDestroyInstance(instance, nullptr);
        return;
    }
    pd = pds[0];
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
    print_result("kxCreateDevice", result);
    if (result != KX_SUCCESS) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Test buffer creation */
    KxBuffer buffer = nullptr;
    KxBufferCreateInfo buf_info = {
        .sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 256,
        .usage = KX_BUFFER_USAGE_TRANSFER_SRC_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingModeExclusive = KX_TRUE,
    };
    result = kxCreateBuffer(device, &buf_info, nullptr, &buffer);
    print_result("kxCreateBuffer", result);

    /* Test memory allocation */
    KxDeviceMemory memory = nullptr;
    KxMemoryAllocateInfo mem_info = {
        .sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = 256,
        .memoryTypeIndex = 0,
    };
    result = kxAllocateMemory(device, &mem_info, nullptr, &memory);
    print_result("kxAllocateMemory", result);

    /* Test bind buffer memory */
    result = kxBindBufferMemory(device, buffer, memory, 0);
    print_result("kxBindBufferMemory", result);

    /* Test map memory */
    void* mapped_ptr = nullptr;
    result = kxMapMemory(device, memory, 0, 256, KX_MEMORY_MAP_NONE, &mapped_ptr);
    print_result("kxMapMemory", result);

    if (mapped_ptr) {
        std::memset(mapped_ptr, 0xAA, 256);
        uint8_t* data = static_cast<uint8_t*>(mapped_ptr);
        bool ok = (data[0] == 0xAA && data[255] == 0xAA);
        print_result("Memory write/verify", ok ? KX_SUCCESS : KX_ERROR_INVALID_ARGUMENT);
    }

    /* Test unmap memory */
    kxUnmapMemory(device, memory);
    print_result("kxUnmapMemory", KX_SUCCESS);

    /* Cleanup */
    kxFreeMemory(device, memory, nullptr);
    kxDestroyBuffer(device, buffer, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

static void test_image_lifecycle() {
    KxInstance instance = nullptr;
    KxApplicationInfo app_info = {
        .sType = KX_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "test",
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
    if (result != KX_SUCCESS) return;

    uint32_t pd_count = 0;
    result = kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    if (result != KX_SUCCESS || pd_count == 0) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    KxPhysicalDevice* pds = new KxPhysicalDevice[pd_count];
    result = kxEnumeratePhysicalDevices(instance, &pd_count, pds);
    if (result != KX_SUCCESS) {
        delete[] pds;
        kxDestroyInstance(instance, nullptr);
        return;
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
    if (result != KX_SUCCESS) {
        kxDestroyInstance(instance, nullptr);
        return;
    }

    /* Test image creation */
    KxImage image = nullptr;
    KxImageCreateInfo img_info = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_B8G8R8A8_UNORM,
        .width = 64,
        .height = 64,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_LINEAR,
        .usage = KX_IMAGE_USAGE_TRANSFER_SRC_BIT | KX_IMAGE_USAGE_TRANSFER_DST_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    result = kxCreateImage(device, &img_info, nullptr, &image);
    print_result("kxCreateImage", result);

    /* Test memory allocation for image */
    KxDeviceMemory memory = nullptr;
    KxMemoryAllocateInfo mem_info = {
        .sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = 64 * 64 * 4,
        .memoryTypeIndex = 0,
    };
    result = kxAllocateMemory(device, &mem_info, nullptr, &memory);
    print_result("kxAllocateMemory (image)", result);

    /* Test bind image memory */
    result = kxBindImageMemory(device, image, memory, 0);
    print_result("kxBindImageMemory", result);

    /* Cleanup */
    kxFreeMemory(device, memory, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

int main() {
    std::printf("=== Buffer/Image Lifecycle Tests ===\n\n");

    test_buffer_lifecycle();
    std::printf("\n");
    test_image_lifecycle();

    std::printf("\n=== All tests completed ===\n");
    return 0;
}