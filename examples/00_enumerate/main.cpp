// Example 00: enumerate physical devices
#include <kromax/kromax.h>
#include <cstdio>

int main() {
    KxApplicationInfo app_info{};
    app_info.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "00_enumerate";
    app_info.apiVersion = KX_API_VERSION_0_1;

    KxInstanceCreateInfo ici{};
    ici.sType = KX_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app_info;

    KxInstance instance = nullptr;
    KxResult res = kxCreateInstance(&ici, nullptr, &instance);
    if (res != KX_SUCCESS) {
        std::printf("kxCreateInstance failed: %s\n", kxResultToString(res));
        return 1;
    }

    uint32_t count = 0;
    res = kxEnumeratePhysicalDevices(instance, &count, nullptr);
    std::printf("Physical devices: %u\n", count);

    kxDestroyInstance(instance, nullptr);
    return 0;
}