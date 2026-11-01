#ifndef KROMAX_SRC_RUNTIME_IBACKEND_H
#define KROMAX_SRC_RUNTIME_IBACKEND_H

/*
 * IBackend.h
 *
 * Backend plugin interface. Each backend (cpu_backend, vk_backend, etc.)
 * implements this interface for dynamic loading.
 *
 * SOLID: Open/Closed Principle - new backends can be added without
 * modifying existing code. Factory pattern via BackendRegistry.
 */

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "IRuntime.h"
#include "Expected.h"

namespace kx {

struct BackendInfo {
    BackendType type = BackendType::UNKNOWN;
    std::string name;
    uint32_t version = 0;
    std::string description;
    std::vector<std::string> supportedExtensions;
    std::vector<std::string> requiredSystemLibs;  // e.g., "vulkan-1", "dl"
};

using BackendFactoryFn = std::unique_ptr<IRuntime>(*)();

class IBackend {
public:
    virtual ~IBackend() = default;

    // Returns backend metadata
    virtual BackendInfo getInfo() const noexcept = 0;

    // Creates the runtime instance
    // Called once per backend load
    virtual std::unique_ptr<IRuntime> createRuntime() = 0;

    // Optional: backend-specific initialization
    // Called after createRuntime(), before any other operations
    virtual KxResult initialize(const char* configJson = nullptr) {
        (void)configJson;
        return KX_SUCCESS;
    }

    // Optional: backend-specific shutdown
    virtual void shutdown() {}
};

// Backend registration macros
#define KX_DECLARE_BACKEND_FACTORY(BackendClass) \
    extern "C" KX_API std::unique_ptr<kx::IRuntime> create_##BackendClass##_runtime() { \
        return std::make_unique<BackendClass>(); \
    }

// For static linking (when not using dynamic plugins)
#define KX_REGISTER_BACKEND_STATIC(BackendClass) \
    namespace { \
        struct BackendClass##_Registrar { \
            BackendClass##_Registrar() { \
                kx::BackendRegistry::instance().registerBackend( \
                    BackendClass::getBackendTypeStatic(), \
                    #BackendClass, \
                    &create_##BackendClass##_runtime \
                ); \
            } \
        }; \
        static BackendClass##_Registrar g_##BackendClass##_registrar; \
    }

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IBACKEND_H