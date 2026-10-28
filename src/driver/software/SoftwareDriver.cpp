#include "SoftwareDriver.h"

#include <cstring>

#include "Instance.h"
#include "PhysicalDevice.h"
#include "SoftwareDevice.h"

namespace kx {

Expected<std::vector<std::unique_ptr<PhysicalDevice>>>
SoftwareDriver::enumerate_physical_devices(Instance* /*instance*/) {
    auto phys = std::make_unique<PhysicalDevice>();

    KxPhysicalDeviceProperties props{};
    props.sType = KX_STRUCTURE_TYPE_APPLICATION_INFO; /* not used here */
    props.apiVersion = KX_API_VERSION_0_1;
    props.driverVersion = KX_MAKE_VERSION(0, 1, 0);
    props.vendorID = 0x1000; /* Kromax vendor ID */
    props.deviceID = 0x0001;
    props.deviceType = KX_PHYSICAL_DEVICE_TYPE_CPU;
    std::snprintf(props.deviceName, sizeof(props.deviceName),
                  "Kromax Software CPU");
    std::snprintf(props.driverName, sizeof(props.driverName),
                  "kromax-software");
    phys->set_properties(props);

    KxPhysicalDeviceFeatures feats{};
    feats.shaderFloat64 = KX_TRUE;
    feats.shaderInt64 = KX_TRUE;
    feats.geometryShader = KX_FALSE;
    feats.tessellationShader = KX_FALSE;
    feats.textureCompressionBC = KX_FALSE;
    feats.textureCompressionETC2 = KX_FALSE;
    feats.textureCompressionASTC_LDR = KX_FALSE;
    feats.pipelineStatisticsQuery = KX_FALSE;
    feats.vertexPipelineStoresAndAtomics = KX_FALSE;
    phys->set_features(feats);

    KxQueueFamilyProperties qf{};
    qf.queueFlags = 0x00000001u | 0x00000200u; /* graphics | compute */
    qf.queueCount = 1;
    qf.timestampValidBits = 64;
    std::vector<KxQueueFamilyProperties> qfs = {qf};
    phys->set_queue_families(qfs);

    KxPhysicalDeviceMemoryProperties mp{};
    mp.memoryHeapCount = 1;
    mp.memoryHeaps[0].size = 1ull << 30; /* 1 GiB */
    mp.memoryHeaps[0].memoryTypeIndex = 0;
    mp.memoryTypeCount = 1;
    mp.memoryTypes[0].heapIndex = 0;
    mp.memoryTypes[0].propertyFlags =
        KX_MEMORY_PROPERTY_HOST_VISIBLE_BIT | KX_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    phys->set_memory_properties(mp);

    std::vector<std::unique_ptr<PhysicalDevice>> out;
    out.push_back(std::move(phys));
    return out;
}

Expected<std::unique_ptr<Device>>
SoftwareDriver::create_device(PhysicalDevice* physical_device,
                              const KxDeviceCreateInfo* /*create_info*/) {
    return std::make_unique<SoftwareDevice>(physical_device);
}

} /* namespace kx */