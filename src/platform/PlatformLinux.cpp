/*
 * PlatformLinux.cpp
 *
 * Linux platform implementation using dlopen, libvulkan, xcb, wayland stubs.
 */

#include "Platform.h"

#include <dlfcn.h>
#include <vector>
#include <string>
#include <cstring>

namespace kx {

/* -------------------------------------------------------------------------- */
/* LinuxLibrary                                                               */
/* -------------------------------------------------------------------------- */

class LinuxLibrary final : public PlatformLibrary {
public:
    explicit LinuxLibrary(void* handle) noexcept : handle_(handle) {}

    ~LinuxLibrary() override {
        if (handle_) {
            dlclose(handle_);
        }
    }

    LinuxLibrary(const LinuxLibrary&) = delete;
    LinuxLibrary& operator=(const LinuxLibrary&) = delete;

    LinuxLibrary(LinuxLibrary&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    LinuxLibrary& operator=(LinuxLibrary&& other) noexcept {
        if (this != &other) {
            if (handle_) dlclose(handle_);
            handle_ = other.handle_;
            other.handle_ = nullptr;
        }
        return *this;
    }

    void* get_symbol(const char* name) noexcept override {
        if (!handle_) return nullptr;
        // dlsym returns null on failure; dlerror() gives the message
        void* sym = dlsym(handle_, name);
        return sym;
    }

    const char* get_last_error() const noexcept override {
        return dlerror();
    }

private:
    void* handle_ = nullptr;
};

/* -------------------------------------------------------------------------- */
/* LinuxPlatform                                                              */
/* -------------------------------------------------------------------------- */

class LinuxPlatform final : public Platform {
public:
    std::unique_ptr<PlatformLibrary> load_library(const char* path) noexcept override {
        void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            // dlerror() will be captured by get_last_error()
            return nullptr;
        }
        return std::make_unique<LinuxLibrary>(handle);
    }

    std::unique_ptr<PlatformLibrary> load_system_library(const char* name) noexcept override {
        // On Linux, system libraries are found via ld.so cache; try dlopen with just the name
        void* handle = dlopen(name, RTLD_NOW | RTLD_LOCAL | RTLD_GLOBAL);
        if (!handle) {
            return nullptr;
        }
        return std::make_unique<LinuxLibrary>(handle);
    }

    std::vector<std::string> get_vulkan_library_names() const noexcept override {
        // Order matters: try standard names first, then distro-specific
        return {
            "vulkan-1",           // Standard (Vulkan SDK, most distros)
            "libvulkan.so.1",     // Explicit soname
            "libvulkan.so",       // Unversioned (dev packages)
        };
    }

    bool create_xcb_surface(void* connection, void* window, void** out_surface) noexcept override {
        // TODO: Implement when swapchain support lands (Phase D)
        // Requires: xcb, vkCreateXcbSurfaceKHR
        (void)connection;
        (void)window;
        (void)out_surface;
        return false;
    }

    bool create_wayland_surface(void* display, void* surface, void** out_surface) noexcept override {
        // TODO: Implement when swapchain support lands (Phase D)
        // Requires: wayland-client, vkCreateWaylandSurfaceKHR
        (void)display;
        (void)surface;
        (void)out_surface;
        return false;
    }

    bool create_win32_surface(void* hinstance, void* hwnd, void** out_surface) noexcept override {
        // Not applicable on Linux
        (void)hinstance;
        (void)hwnd;
        (void)out_surface;
        return false;
    }

    const char* name() const noexcept override {
        return "linux";
    }
};

/* -------------------------------------------------------------------------- */
/* Platform singleton                                                         */
/* -------------------------------------------------------------------------- */

static LinuxPlatform g_linux_platform;

Platform& get_platform() noexcept {
    return g_linux_platform;
}

} // namespace kx