#ifndef KROMAX_SRC_DRIVER_AMDGPU_AMDGPUDRIVER_H
#define KROMAX_SRC_DRIVER_AMDGPU_AMDGPUDRIVER_H

/*
 * AmdGpuDriver.h
 *
 * AMD GPU hardware driver skeleton. Disabled by default
 * (KX_BUILD_DRIVER_AMDGPU=OFF). This file is intentionally minimal: the
 * real driver remains a future implementation (see ADR 0002 / ROADMAP).
 */

#ifdef KX_DRIVER_AMDGPU_ENABLED
#error "amdgpu driver is not yet implemented"
#endif

namespace kx {

/* Placeholder: no symbols compiled when the driver is disabled. */
struct AmdGpuDriverPlaceholder {};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_AMDGPU_AMDGPUDRIVER_H */