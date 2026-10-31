#ifndef KROMAX_SRC_SHADER_SHADERDIAGNOSTICS_H
#define KROMAX_SRC_SHADER_SHADERDIAGNOSTICS_H

/*
 * ShaderDiagnostics.h
 *
 * Diagnostic collection shared by the lexer, the preprocessor and the parser.
 * Every diagnostic carries enough information to point the user at the exact
 * source location that caused the problem.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "kx_core.h"

namespace kx {
namespace shader {

struct Diagnostic {
    KxShaderSeverity severity = KX_SHADER_SEVERITY_ERROR;
    uint32_t file = 0;
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t offset = 0;
    std::string message;
};

class DiagnosticList {
public:
    void report(KxShaderSeverity severity, uint32_t file, uint32_t line,
                uint32_t column, uint32_t offset, const std::string& message) {
        Diagnostic diagnostic;
        diagnostic.severity = severity;
        diagnostic.file = file;
        diagnostic.line = line;
        diagnostic.column = column;
        diagnostic.offset = offset;
        diagnostic.message = message;
        diagnostics_.push_back(std::move(diagnostic));
    }

    void error(uint32_t file, uint32_t line, uint32_t column, uint32_t offset,
               const std::string& message) {
        report(KX_SHADER_SEVERITY_ERROR, file, line, column, offset, message);
    }

    void warning(uint32_t file, uint32_t line, uint32_t column, uint32_t offset,
                 const std::string& message) {
        report(KX_SHADER_SEVERITY_WARNING, file, line, column, offset, message);
    }

    void info(uint32_t file, uint32_t line, uint32_t column, uint32_t offset,
              const std::string& message) {
        report(KX_SHADER_SEVERITY_INFO, file, line, column, offset, message);
    }

    bool has_errors() const noexcept {
        for (const auto& d : diagnostics_) {
            if (d.severity == KX_SHADER_SEVERITY_ERROR) return true;
        }
        return false;
    }

    const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
    std::vector<Diagnostic>& diagnostics() noexcept { return diagnostics_; }
    void clear() noexcept { diagnostics_.clear(); }

private:
    std::vector<Diagnostic> diagnostics_;
};

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERDIAGNOSTICS_H */
