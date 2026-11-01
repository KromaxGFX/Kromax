#ifndef KROMAX_SRC_RUNTIME_ISEMAPHORE_H
#define KROMAX_SRC_RUNTIME_ISEMAPHORE_H

/*
 * ISemaphore.h
 *
 * Backend-agnostic semaphore and fence interface.
 * Supports both binary and timeline semaphores.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

enum class SemaphoreType : uint32_t {
    BINARY = 0,
    TIMELINE = 1,
};

class ISemaphore {
public:
    virtual ~ISemaphore() = default;

    virtual SemaphoreType getType() const noexcept = 0;
    virtual uint64_t getCounterValue() const noexcept = 0;  // For timeline
    virtual Result signal(uint64_t value = 0) = 0;  // value ignored for binary
    virtual Result wait(uint64_t value, uint64_t timeout) = 0;  // value ignored for binary
};

class IFence {
public:
    virtual ~IFence() = default;

    virtual Result wait(uint64_t timeout) = 0;
    virtual Result reset() = 0;
    virtual bool getStatus() const noexcept = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISEMAPHORE_H