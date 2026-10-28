#ifndef KROMAX_SRC_CORE_ALLOCATOR_H
#define KROMAX_SRC_CORE_ALLOCATOR_H

/*
 * Allocator.h
 *
 * STL-compatible allocator backed by KxAllocationCallbacks. Falls back to
 * ::operator new / ::operator delete when no callbacks are provided.
 * All internal allocations pass through this allocator.
 */

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

#include "kx_core.h"

namespace kx {

class AllocatorCallbacks {
public:
    AllocatorCallbacks() noexcept = default;

    explicit AllocatorCallbacks(const KxAllocationCallbacks* cb) noexcept
        : cb_(cb) {}

    bool has_callbacks() const noexcept { return cb_ != nullptr; }

    void* allocate(std::size_t size, std::size_t alignment) {
        if (cb_ && cb_->pfnAllocation) {
            return cb_->pfnAllocation(
                cb_->pUserData, size, alignment,
                KX_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        }
        // Fallback: aligned allocation via standard C library
        if (alignment <= __alignof__(std::max_align_t)) {
            return std::malloc(size);
        }
        void* ptr = nullptr;
        if (::posix_memalign(&ptr, alignment, size) != 0) {
            return nullptr;
        }
        return ptr;
    }

    void deallocate(void* ptr) noexcept {
        if (!ptr) return;
        if (cb_ && cb_->pfnFree) {
            cb_->pfnFree(cb_->pUserData, ptr);
        } else {
            std::free(ptr);
        }
    }

    void* reallocate(void* original, std::size_t size,
                     std::size_t alignment) {
        if (cb_ && cb_->pfnReallocation) {
            return cb_->pfnReallocation(
                cb_->pUserData, original, size, alignment,
                KX_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        }
        return std::realloc(original, size);
    }

private:
    const KxAllocationCallbacks* cb_ = nullptr;
};

template <typename T>
class Allocator {
public:
    using value_type = T;

    Allocator() noexcept = default;

    explicit Allocator(const KxAllocationCallbacks* cb) noexcept
        : callbacks_(cb) {}

    template <typename U>
    Allocator(const Allocator<U>& other) noexcept
        : callbacks_(other.callbacks_) {}

    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        if (n > std::size_t(-1) / sizeof(T)) return nullptr;
        auto* ptr = static_cast<T*>(
            callbacks_.allocate(n * sizeof(T), alignof(T)));
        if (!ptr) return nullptr;
        return ptr;
    }

    void deallocate(T* p, std::size_t /*n*/) noexcept {
        callbacks_.deallocate(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        p->~U();
    }

    const AllocatorCallbacks& callbacks() const noexcept { return callbacks_; }

    friend bool operator==(const Allocator&, const Allocator&) noexcept {
        return true;
    }
    friend bool operator!=(const Allocator&, const Allocator&) noexcept {
        return false;
    }

private:
    AllocatorCallbacks callbacks_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_ALLOCATOR_H */