// Integration test: enumerate physical devices
#include <kromax/kromax.h>
#include <doctest/doctest.h>

TEST_CASE("enumerate_physical_devices") {
    KxInstance instance = nullptr;
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "test";
    app_info.apiVersion = KX_API_VERSION_0_1;

    KxInstanceCreateInfo ici{};
    ici.sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app_info;

    KxResult res = kxCreateInstance(&ici, nullptr, &instance);
    CHECK(res == KX_SUCCESS);

    uint32_t count = 0;
    res = kxEnumeratePhysicalDevices(instance, &count, nullptr);
    CHECK(res == KX_SUCCESS);
    CHECK(count >= 1);

    kxDestroyInstance(instance, nullptr);
}