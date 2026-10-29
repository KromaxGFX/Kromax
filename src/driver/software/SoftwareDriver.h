#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDRIVER_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDRIVER_H

/*
 * SoftwareDriver.h
 *
 * CPU software driver implementing IDriver. Exposes a single physical
 * device ("Kromax Software CPU") backed by system RAM. Reference backend
 * used for CI, development, and offline rendering.
 */

#include <memory>
#include <vector>

#include "IDriver.h"

namespace kx {

class SoftwareDriver : public IDriver {
public:
    SoftwareDriver() = default;
    ~SoftwareDriver() override = default;

    const char* name() const noexcept override { return "kromax-software"; }
    uint32_t version() const noexcept override { return KX_MAKE_VERSION(0, 1, 0); }

    Expected<std::vector<std::unique_ptr<PhysicalDevice>>>
    enumerate_physical_devices(Instance* instance) override;

    Expected<std::unique_ptr<IDriverDevice>>
    create_device(PhysicalDevice* physical_device,
                  const KxDeviceCreateInfo* create_info) override;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDRIVER_H */