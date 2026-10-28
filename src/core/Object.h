#ifndef KROMAX_SRC_CORE_OBJECT_H
#define KROMAX_SRC_CORE_OBJECT_H

/*
 * Object.h
 *
 * Base class for all internal Kromax objects. Provides intrusive reference
 * counting, object-type tagging, and a virtual destructor.
 */

#include <atomic>
#include <cstdint>

#include "kx_core.h"

namespace kx {

enum class ObjectType : uint32_t {
    Instance = 1,
    PhysicalDevice = 2,
    Device = 3,
    Queue = 4,
    DeviceMemory = 5,
    Buffer = 6,
    Image = 7,
    ImageView = 8,
    ShaderModule = 9,
    PipelineLayout = 10,
    Pipeline = 11,
    CommandPool = 12,
    CommandBuffer = 13,
    Fence = 14,
    Semaphore = 15,
    DebugMessenger = 16,
};

class Object {
public:
    explicit Object(ObjectType type) noexcept
        : type_(type), ref_count_(1) {}

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    virtual ~Object() = default;

    ObjectType type() const noexcept { return type_; }

    void ref() noexcept { ref_count_.fetch_add(1, std::memory_order_relaxed); }

    void unref() noexcept {
        if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
        }
    }

    uint32_t ref_count() const noexcept {
        return ref_count_.load(std::memory_order_relaxed);
    }

private:
    ObjectType type_;
    std::atomic<uint32_t> ref_count_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_OBJECT_H */