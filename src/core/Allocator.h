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
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

#include "kx_core.h"

namespace kx {

/* Portable aligned allocation.
 *
 * Neither std::aligned_alloc nor posix_memalign is available on every
 * supported toolchain, so allocations always go through a small header that
 * records the owning base pointer and the payload size. The header sits
 * immediately before the aligned payload, which keeps the payload pointer
 * suitable for std::vector / std::string and for direct byte access.
 *
 * Layout:  [ magic ][ base ptr ][ payload size ][ ... padding ... ][ payload ]
 */
namespace alloc_detail {

inline constexpr std::uintptr_t kAllocMagic = 0x4B58414C4F434D47ull;

inline void* aligned_allocate(std::size_t size, std::size_t alignment) {
    if (size == 0) return nullptr;
    if (alignment < alignof(void*)) alignment = alignof(void*);

    constexpr std::size_t kHeaderSize = sizeof(std::uintptr_t) * 3;
    const std::size_t total = kHeaderSize + size + alignment;
    void* base = std::malloc(total);
    if (!base) return nullptr;

    auto address = reinterpret_cast<std::uintptr_t>(base) + kHeaderSize;
    address = (address + alignment - 1) & ~static_cast<std::uintptr_t>(alignment - 1);
    auto* aligned = reinterpret_cast<void*>(address);

    auto* magic = reinterpret_cast<std::uintptr_t*>(aligned) - 3;
    auto* owner = reinterpret_cast<void**>(aligned) - 2;
    auto* payload_size = reinterpret_cast<std::size_t*>(aligned) - 1;
    *magic = kAllocMagic;
    *owner = base;
    *payload_size = size;
    return aligned;
}

inline void aligned_free(void* ptr) noexcept {
    if (!ptr) return;
    auto* magic = reinterpret_cast<std::uintptr_t*>(ptr) - 3;
    if (*magic == kAllocMagic) {
        auto* owner = reinterpret_cast<void**>(ptr) - 2;
        std::free(*owner);
        return;
    }
    std::free(ptr);
}

inline std::size_t payload_size(void* ptr) noexcept {
    if (!ptr) return 0;
    auto* magic = reinterpret_cast<std::uintptr_t*>(ptr) - 3;
    if (*magic != kAllocMagic) return 0;
    return *reinterpret_cast<std::size_t*>(ptr) - 1;
}

} /* namespace alloc_detail */

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
        // Fallback: aligned allocation via the portable helper
        return alloc_detail::aligned_allocate(size, alignment);
    }

    void deallocate(void* ptr) noexcept {
        if (!ptr) return;
        if (cb_ && cb_->pfnFree) {
            cb_->pfnFree(cb_->pUserData, ptr);
        } else {
            alloc_detail::aligned_free(ptr);
        }
    }

    void* reallocate(void* original, std::size_t size,
                     std::size_t alignment) {
        if (cb_ && cb_->pfnReallocation) {
            return cb_->pfnReallocation(
                cb_->pUserData, original, size, alignment,
                KX_SYSTEM_ALLOCATION_SCOPE_OBJECT);
        }
        if (!original) return allocate(size, alignment);

        const std::size_t old_size = alloc_detail::payload_size(original);
        void* replacement = alloc_detail::aligned_allocate(size, alignment);
        if (!replacement) return nullptr;
        if (old_size > 0) {
            std::memcpy(replacement, original, old_size < size ? old_size : size);
        }
        alloc_detail::aligned_free(original);
        return replacement;
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