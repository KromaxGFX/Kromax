/*
 * test_commands.cpp
 *
 * Unit tests for command buffer recording and execution.
 */

#include <kromax/kx_core.h>

#include <cstdio>
#include <cstring>

static void print_result(const char* name, KxResult result) {
    const char* status = (result == KX_SUCCESS) ? "PASS" : "FAIL";
    std::printf("[%s] %s: %s\n", status, name, kxResultToString(result));
}

static void test_command_buffer_lifecycle() {
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

    /* Get queue */
    KxQueue queue = nullptr;
    result = kxGetDeviceQueue(device, 0, 0, &queue);
    print_result("kxGetDeviceQueue", result);

    /* Create command pool */
    KxCommandPool cmd_pool = nullptr;
    KxCommandPoolCreateInfo pool_info = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
    };
    result = kxCreateCommandPool(device, &pool_info, nullptr, &cmd_pool);
    print_result("kxCreateCommandPool", result);

    /* Allocate command buffer */
    KxCommandBuffer cmd_buffer = nullptr;
    KxCommandBufferAllocateInfo alloc_info = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_CREATE_INFO,
        .commandPool = cmd_pool,
        .level = KX_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    result = kxAllocateCommandBuffers(device, &alloc_info, &cmd_buffer);
    print_result("kxAllocateCommandBuffers", result);

    /* Begin command buffer */
    KxCommandBufferBeginInfo begin_info = {
        .sType = KX_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = KX_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    result = kxBeginCommandBuffer(cmd_buffer, &begin_info);
    print_result("kxBeginCommandBuffer", result);

    /* Create test resources */
    KxBuffer src_buffer = nullptr, dst_buffer = nullptr;
    KxImage image = nullptr;
    KxDeviceMemory buffer_mem = nullptr, image_mem = nullptr;

    KxBufferCreateInfo buf_info = {
        .sType = KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 256,
        .usage = KX_BUFFER_USAGE_TRANSFER_SRC_BIT | KX_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingModeExclusive = KX_TRUE,
    };
    result = kxCreateBuffer(device, &buf_info, nullptr, &src_buffer);
    print_result("Create src buffer", result);

    result = kxCreateBuffer(device, &buf_info, nullptr, &dst_buffer);
    print_result("Create dst buffer", result);

    KxImageCreateInfo img_info = {
        .sType = KX_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = KX_IMAGE_TYPE_2D,
        .format = KX_FORMAT_B8G8R8A8_UNORM,
        .width = 32,
        .height = 32,
        .depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = KX_SAMPLE_COUNT_1_BIT,
        .tiling = KX_IMAGE_TILING_LINEAR,
        .usage = KX_IMAGE_USAGE_TRANSFER_DST_BIT | KX_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingModeExclusive = KX_TRUE,
        .initialLayout = KX_IMAGE_LAYOUT_UNDEFINED,
    };
    result = kxCreateImage(device, &img_info, nullptr, &image);
    print_result("Create image", result);

    /* Allocate and bind memory for buffers */
    KxMemoryAllocateInfo mem_info = {
        .sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = 512,
        .memoryTypeIndex = 0,
    };
    result = kxAllocateMemory(device, &mem_info, nullptr, &buffer_mem);
    print_result("Allocate buffer memory", result);

    result = kxBindBufferMemory(device, src_buffer, buffer_mem, 0);
    print_result("Bind src buffer memory", result);

    result = kxBindBufferMemory(device, dst_buffer, buffer_mem, 256);
    print_result("Bind dst buffer memory", result);

    /* Allocate and bind memory for image */
    KxMemoryAllocateInfo img_mem_info = {
        .sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = 32 * 32 * 4,
        .memoryTypeIndex = 0,
    };
    result = kxAllocateMemory(device, &img_mem_info, nullptr, &image_mem);
    print_result("Allocate image memory", result);

    result = kxBindImageMemory(device, image, image_mem, 0);
    print_result("Bind image memory", result);

    /* Map and write to src buffer */
    void* mapped = nullptr;
    result = kxMapMemory(device, buffer_mem, 0, 256, KX_MEMORY_MAP_NONE, &mapped);
    print_result("Map buffer memory", result);
    if (mapped) {
        std::memset(mapped, 0x42, 256);
    }
    kxUnmapMemory(device, buffer_mem);
    print_result("Unmap buffer memory", KX_SUCCESS);

    /* Record commands */
    KxBufferCopy copy_region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = 256
    };
    kxCmdCopyBuffer(cmd_buffer, src_buffer, dst_buffer, 1, &copy_region);
    print_result("kxCmdCopyBuffer", KX_SUCCESS);

    float clear_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    kxCmdClearImage(cmd_buffer, image, KX_IMAGE_LAYOUT_GENERAL,
                    clear_color, nullptr, 0, nullptr);
    print_result("kxCmdClearImage", KX_SUCCESS);

    /* End command buffer */
    result = kxEndCommandBuffer(cmd_buffer);
    print_result("kxEndCommandBuffer", result);

    /* Submit to queue */
    KxSubmitInfo submit_info = {
        .sType = KX_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffer,
    };
    result = kxQueueSubmit(queue, 1, &submit_info, nullptr);
    print_result("kxQueueSubmit", result);

    /* Wait idle */
    result = kxQueueWaitIdle(queue);
    print_result("kxQueueWaitIdle", result);

    /* Verify dst buffer was copied */
    result = kxMapMemory(device, buffer_mem, 256, 256, KX_MEMORY_MAP_NONE, &mapped);
    if (mapped) {
        uint8_t* data = static_cast<uint8_t*>(mapped);
        bool ok = (data[0] == 0x42 && data[255] == 0x42);
        print_result("Buffer copy verification", ok ? KX_SUCCESS : KX_ERROR_INVALID_ARGUMENT);
    }
    kxUnmapMemory(device, buffer_mem);

    /* Verify image was cleared */
    result = kxMapMemory(device, image_mem, 0, 32 * 32 * 4, KX_MEMORY_MAP_NONE, &mapped);
    if (mapped) {
        uint8_t* data = static_cast<uint8_t*>(mapped);
        // Check first pixel is red (1.0, 0.0, 0.0, 1.0) = (255, 0, 0, 255)
        bool ok = (data[0] == 255 && data[1] == 0 && data[2] == 0 && data[3] == 255);
        print_result("Image clear verification", ok ? KX_SUCCESS : KX_ERROR_INVALID_ARGUMENT);
    }
    kxUnmapMemory(device, image_mem);

    /* Cleanup */
    kxFreeMemory(device, buffer_mem, nullptr);
    kxFreeMemory(device, image_mem, nullptr);
    kxDestroyBuffer(device, src_buffer, nullptr);
    kxDestroyBuffer(device, dst_buffer, nullptr);
    kxDestroyImage(device, image, nullptr);
    kxFreeCommandBuffers(device, cmd_pool, 1, &cmd_buffer);
    kxDestroyCommandPool(device, cmd_pool, nullptr);
    kxDestroyDevice(device, nullptr);
    kxDestroyInstance(instance, nullptr);
}

int main() {
    std::printf("=== Command Buffer Tests ===\n\n");

    test_command_buffer_lifecycle();

    std::printf("\n=== All tests completed ===\n");
    return 0;
}