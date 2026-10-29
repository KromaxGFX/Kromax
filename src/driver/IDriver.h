#ifndef KROMAX_SRC_DRIVER_IDRIVER_H
#define KROMAX_SRC_DRIVER_IDRIVER_H

/*
 * IDriver.h
 *
 * Abstract driver interface. Each GPU driver (software, amdgpu, future
 * hardware drivers) implements this. Registered statically in
 * DriverRegistry; a future ADR covers .so plugin (ICD-style) loading.
 */

#include <memory>
#include <string>
#include <vector>

#include "Expected.h"
#include "IDriverDevice.h"

#include "kx_core.h"

namespace kx {

class Instance;
class PhysicalDevice;
class Device;

class IDriver {
public:
    virtual ~IDriver() = default;

    virtual const char* name() const noexcept = 0;
    virtual uint32_t version() const noexcept = 0;

    /*
     * Enumerate physical devices exposed by this driver. The driver fills
     * out properties/features/memory and returns a vector of internal
     * PhysicalDevice objects.
     */
    virtual Expected<std::vector<std::unique_ptr<PhysicalDevice>>>
    enumerate_physical_devices(Instance* instance) = 0;

    /*
     * Create a device from a physical device.
     * Returns the driver's internal device implementation.
     */
    virtual Expected<std::unique_ptr<IDriverDevice>>
    create_device(PhysicalDevice* physical_device,
                  const KxDeviceCreateInfo* create_info) = 0;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_IDRIVER_H */