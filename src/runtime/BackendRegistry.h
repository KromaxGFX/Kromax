#ifndef KROMAX_SRC_RUNTIME_BACKENDREGISTRY_H
#define KROMAX_SRC_RUNTIME_BACKENDREGISTRY_H

/*
 * BackendRegistry.h
 *
 * Registry for backend plugins. Supports both static registration
 * and dynamic loading via dlopen/LoadLibrary.
 */

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <type_traits>

#include "IRuntime.h"
#include "IBackend.h"
#include "Expected.h"
#include "../platform/Platform.h"

namespace kx {

class BackendRegistry {
public:
    BackendRegistry(const BackendRegistry&) = delete;
    BackendRegistry& operator=(const BackendRegistry&) = delete;

    static BackendRegistry& instance() noexcept {
        static BackendRegistry registry;
        return registry;
    }

    using BackendFactoryFn = std::unique_ptr<IRuntime>(*)();

    struct BackendEntry {
        BackendType type;
        std::string libraryName;
        BackendFactoryFn factory;
        bool isStatic = true;
    };

    void registerBackend(BackendType type, const char* libraryName, BackendFactoryFn factory) noexcept;

    template <BackendType T, typename TBackend>
    void registerBackend() noexcept {
        static_assert(std::is_base_of_v<IRuntime, TBackend>, "TBackend must derive from IRuntime");
        registerBackend(T, nullptr, []() -> std::unique_ptr<IRuntime> {
            return std::make_unique<TBackend>();
        });
    }

    Expected<std::unique_ptr<IRuntime>> loadBackend(const char* name);
    Expected<std::unique_ptr<IRuntime>> loadPreferredBackend();

    const std::vector<BackendEntry>& getStaticBackends() const noexcept;
    std::vector<std::string> getAvailableBackends() const noexcept;

private:
    BackendRegistry() = default;

    std::vector<BackendEntry> staticBackends_;
    std::unordered_map<std::string, std::unique_ptr<PlatformLibrary>> loadedLibraries_;
    mutable std::mutex mutex_;

    std::unique_ptr<PlatformLibrary> platformLoadLibrary(const char* path);
    void* platformGetSymbol(PlatformLibrary* lib, const char* name);
    std::string getLibraryPath(const char* name) const;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_BACKENDREGISTRY_H