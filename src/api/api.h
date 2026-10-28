#ifndef KROMAX_SRC_API_API_H
#define KROMAX_SRC_API_API_H

/*
 * api.h
 *
 * Internal declarations for the C API layer.
 */

#include <memory>
#include <vector>

#include "kx_core.h"

namespace kx {

class Instance;
class Device;
class PhysicalDevice;

/* Global registries */
std::vector<std::unique_ptr<Instance>>& g_instances();
std::vector<std::unique_ptr<PhysicalDevice>>& g_physical_devices();
std::vector<std::unique_ptr<Device>>& g_devices();

Instance* find_instance(KxInstance instance);
PhysicalDevice* find_physical_device(KxPhysicalDevice physical_device);
Device* find_device(KxDevice device);

} /* namespace kx */

#endif /* KROMAX_SRC_API_API_H */