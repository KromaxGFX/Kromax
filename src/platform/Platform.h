#ifndef KROMAX_SRC_PLATFORM_PLATFORM_H
#define KROMAX_SRC_PLATFORM_PLATFORM_H

/*
 * Platform.h
 *
 * Platform abstraction layer for OS-specific functionality:
 * - Dynamic library loading (dlopen/LoadLibrary)
 * - Vulkan loader discovery
 * - Surface/window system integration (stubs for now)
 *
 * This is internal C++17 API; not exposed in public C headers.
 */

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace kx {

/* -------------------------------------------------------------------------- */
/* Platform Library Handle                                                    */
/* -------------------------------------------------------------------------- */

class PlatformLibrary {
public:
    virtual ~PlatformLibrary() = default;

    // Returns null on failure; error message via get_last_error()
    virtual void* get_symbol(const char* name) noexcept = 0;

    // Platform-specific last error (dlerror/GetLastError)
    virtual const char* get_last_error() const noexcept = 0;
};

/* -------------------------------------------------------------------------- */
/* Platform Interface                                                         */
/* -------------------------------------------------------------------------- */

class Platform {
public:
    virtual ~Platform() = default;

    // Dynamic library loading
    virtual std::unique_ptr<PlatformLibrary> load_library(const char* path) noexcept = 0;
    virtual std::unique_ptr<PlatformLibrary> load_system_library(const char* name) noexcept = 0;

    // Vulkan loader discovery
    // Returns list of candidate library names/paths to try (e.g., "vulkan-1", "libvulkan.so.1")
    virtual std::vector<std::string> get_vulkan_library_names() const noexcept = 0;

    // Surface creation stubs (to be implemented when swapchain support lands)
    // Returns empty optional on unsupported platform
    virtual bool create_xcb_surface(void* connection, void* window, void** out_surface) noexcept { return false; }
    virtual bool create_wayland_surface(void* display, void* surface, void** out_surface) noexcept { return false; }
    virtual bool create_win32_surface(void* hinstance, void* hwnd, void** out_surface) noexcept { return false; }

    // Platform name for logging/debugging
    virtual const char* name() const noexcept = 0;
};

/* -------------------------------------------------------------------------- */
/* Global accessor                                                            */
/* -------------------------------------------------------------------------- */

// Returns the platform singleton (initialized at startup)
Platform& get_platform() noexcept;

} // namespace kx

#endif // KROMAX_SRC_PLATFORM_PLATFORM_H