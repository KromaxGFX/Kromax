#include "DriverRegistry.h"

namespace kx {

DriverRegistry& DriverRegistry::instance() noexcept {
    static DriverRegistry registry;
    return registry;
}

void DriverRegistry::register_driver(std::unique_ptr<IDriver> driver) noexcept {
    if (!driver) return;
    std::lock_guard<std::mutex> lock(mutex_);
    drivers_.push_back(std::move(driver));
}

IDriver* DriverRegistry::find_driver(const char* name) const noexcept {
    if (!name) return nullptr;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& driver : drivers_) {
        if (driver && std::string(driver->name()) == name) {
            return driver.get();
        }
    }
    return nullptr;
}

} /* namespace kx */