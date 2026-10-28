#ifndef KROMAX_SRC_CORE_INSTANCE_H
#define KROMAX_SRC_CORE_INSTANCE_H

/*
 * Instance.h
 *
 * Internal Kromax Instance object. Holds enabled extensions/layers,
 * the dispatch table, and the debug messenger.
 */

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "Allocator.h"
#include "Dispatch.h"
#include "Logging.h"
#include "Object.h"

namespace kx {

class Instance : public Object {
public:
    Instance(const KxInstanceCreateInfo* create_info,
             const KxAllocationCallbacks* allocator) noexcept;

    ~Instance() override = default;

    const KxAllocationCallbacks* allocator() const noexcept {
        return allocator_;
    }

    const AllocatorCallbacks& alloc_callbacks() const noexcept {
        return alloc_callbacks_;
    }

    bool extension_enabled(const char* name) const noexcept;

    const std::vector<std::string>& enabled_extensions() const noexcept {
        return enabled_extensions_;
    }

    const std::vector<std::string>& enabled_layers() const noexcept {
        return enabled_layers_;
    }

    const InstanceDispatch* dispatch() const noexcept { return &dispatch_; }

private:
    const KxAllocationCallbacks* allocator_;
    AllocatorCallbacks alloc_callbacks_;
    std::vector<std::string> enabled_extensions_;
    std::vector<std::string> enabled_layers_;
    InstanceDispatch dispatch_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_INSTANCE_H */