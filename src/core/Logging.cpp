#include "Logging.h"

#include <cstdio>

namespace kx {

KxResult log_message(LogLevel level, const char* message) noexcept {
    if (!message) return KX_ERROR_INVALID_ARGUMENT;
    const char* level_str = "INFO";
    switch (level) {
    case LogLevel::Verbose: level_str = "VERBOSE"; break;
    case LogLevel::Info:    level_str = "INFO"; break;
    case LogLevel::Warning: level_str = "WARNING"; break;
    case LogLevel::Error:   level_str = "ERROR"; break;
    default: break;
    }
    std::fprintf(stderr, "[kromax][%s] %s\n", level_str, message);
    return KX_SUCCESS;
}

void log_info(const char* message) noexcept {
    log_message(LogLevel::Info, message);
}

void log_warning(const char* message) noexcept {
    log_message(LogLevel::Warning, message);
}

void log_error(const char* message) noexcept {
    log_message(LogLevel::Error, message);
}

} /* namespace kx */