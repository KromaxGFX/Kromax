#ifndef KROMAX_SRC_CORE_LOGGING_H
#define KROMAX_SRC_CORE_LOGGING_H

/*
 * Logging.h
 *
 * Minimal internal logging. All messages pass through the debug messenger
 * callback when one is registered, otherwise to stderr. No exceptions.
 */

#include <cstdio>
#include <cstdint>

#include "kx_core.h"

namespace kx {

enum class LogLevel : uint32_t {
    Verbose = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
};

/* Log a message. Returns KX_SUCCESS. */
KxResult log_message(LogLevel level, const char* message) noexcept;

/* Convenience printf-style helper (no varargs in the public signature). */
void log_info(const char* message) noexcept;
void log_warning(const char* message) noexcept;
void log_error(const char* message) noexcept;

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_LOGGING_H */