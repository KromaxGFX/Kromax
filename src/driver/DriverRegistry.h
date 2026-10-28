#ifndef KROMAX_SRC_DRIVER_DRIVERREGISTRY_H
#define KROMAX_SRC_DRIVER_DRIVERREGISTRY_H

/*
 * DriverRegistry.h
 *
 * Static registry of available drivers. The first driver that reports at
 * least one physical device is typically selected; the application may
 * later choose explicitly.
 */

#include <memory>
#include <mutex>
#include <vector>

#include "IDriver.h"

namespace kx {

class DriverRegistry {
public:
    DriverRegistry(const DriverRegistry&) = delete;
    DriverRegistry& operator=(const DriverRegistry&) = delete;

    static DriverRegistry& instance() noexcept;

    void register_driver(std::unique_ptr<IDriver> driver) noexcept;

    IDriver* find_driver(const char* name) const noexcept;

    const std::vector<std::unique_ptr<IDriver>>& drivers() const noexcept {
        return drivers_;
    }

private:
    DriverRegistry() = default;

    std::vector<std::unique_ptr<IDriver>> drivers_;
    mutable std::mutex mutex_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_DRIVERREGISTRY_H */