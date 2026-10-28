/*
 * api_dispatch.cpp
 *
 * Global dispatch table definitions. The public API layer calls through
 * these tables; layers (validation) can wrap them.
 */

#include "Dispatch.h"

namespace kx {

InstanceDispatch g_instance_dispatch{};
DeviceDispatch g_device_dispatch{};

} /* namespace kx */