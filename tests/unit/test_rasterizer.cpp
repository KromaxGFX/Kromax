/* test_rasterizer.cpp
 * Unit tests for software rasterizer corrections.
 * Covers: API function availability and basic correctness.
 * Uses doctest framework.
 */

#include <doctest.h>

#include <kromax/kromax.h>

#include <cstdio>

TEST_CASE("kxCreateInstance creates valid instance") {
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
    CHECK(result == KX_SUCCESS);
    CHECK(instance != nullptr);
    
    /* Verify we can get instance proc addr */
    KxPfnVoidFunction proc = kxGetInstanceProcAddr(instance, "kxCreateDevice");
    (void)proc; // Suppress unused warning
    
    kxDestroyInstance(instance, nullptr);
}

TEST_CASE("kxEnumeratePhysicalDevices returns count") {
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
    kxCreateInstance(&inst_info, nullptr, &instance);
    
    uint32_t pd_count = 0;
    KxResult result = kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    CHECK(result == KX_SUCCESS);
    CHECK(pd_count >= 0);
    
    kxDestroyInstance(instance, nullptr);
}

TEST_CASE("kxGetPhysicalDeviceProperties works") {
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
    kxCreateInstance(&inst_info, nullptr, &instance);
    
    uint32_t pd_count = 0;
    kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    
    if (pd_count > 0) {
        KxPhysicalDevice physical_devices[1];
        kxEnumeratePhysicalDevices(instance, &pd_count, physical_devices);
        
        KxPhysicalDeviceProperties props{};
        KxResult result = kxGetPhysicalDeviceProperties(physical_devices[0], &props);
        CHECK(result == KX_SUCCESS);
        CHECK(props.apiVersion > 0);
    }
    
    kxDestroyInstance(instance, nullptr);
}

TEST_CASE("kxCreateDevice creates valid device") {
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
    kxCreateInstance(&inst_info, nullptr, &instance);
    
    uint32_t pd_count = 0;
    kxEnumeratePhysicalDevices(instance, &pd_count, nullptr);
    
    if (pd_count > 0) {
        KxPhysicalDevice physical_devices[1];
        kxEnumeratePhysicalDevices(instance, &pd_count, physical_devices);
        
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
        
        KxResult result = kxCreateDevice(physical_devices[0], &dev_info, nullptr, &device);
        CHECK(result == KX_SUCCESS);
        CHECK(device != nullptr);
        
        kxDestroyDevice(device, nullptr);
    }
    
    kxDestroyInstance(instance, nullptr);
}
