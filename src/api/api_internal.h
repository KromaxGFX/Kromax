/*
 * api_internal.h
 *
 * Internal API declarations shared between api.cpp and api_*.cpp modules.
 * Not part of public API.
 */

#ifndef KROMAX_SRC_API_API_INTERNAL_H
#define KROMAX_SRC_API_API_INTERNAL_H

#include "kx_core.h"

namespace kx {

class Instance;
class PhysicalDevice;
class Device;
class Queue;
class SoftwareDevice;
class SoftwareCommandBuffer;

/* Global registry access */
std::vector<std::unique_ptr<Instance>>& g_instances();
std::vector<std::unique_ptr<PhysicalDevice>>& g_physical_devices();
std::vector<std::unique_ptr<Device>>& g_devices();

/* Lookup functions */
Instance* find_instance_locked(KxInstance instance);
Device* find_device_locked(KxDevice device);
PhysicalDevice* find_physical_device_locked(KxPhysicalDevice pd);
Device* find_device_locked_by_command_buffer(KxCommandBuffer cb);
Device* find_device_locked_by_queue(KxQueue queue);

} /* namespace kx */

#endif /* KROMAX_SRC_API_API_INTERNAL_H */