#ifndef KROMAX_SRC_SHADER_SHADERCOMPILER_H
#define KROMAX_SRC_SHADER_SHADERCOMPILER_H

/*
 * ShaderCompiler.h
 *
 * High level compiler orchestrating the preprocessor, parser and IR validation.
 * Accepts source in GLSL, HLSL, WGSL or KXSL and produces a ShaderProgram.
 */

#include <cstdint>
#include <string>
#include <vector>
#include <functional>

#include "ShaderIR.h"
#include "ShaderDiagnostics.h"
#include "ShaderLexer.h"
#include "ShaderParser.h"
#include "kx_core.h"

namespace kx {
namespace shader {

class ShaderCompiler {
public:
    struct CompileOptions {
        KxShaderLanguage language = KX_SHADER_LANGUAGE_GLSL;
        KxShaderStage stage_hint = KX_SHADER_STAGE_VERTEX;
        std::string entry_point = "main";
        bool dump_ir = false;
        bool optimize = false;
    };

    explicit ShaderCompiler(DiagnosticList& diagnostics) : diagnostics_(diagnostics) {}

    /* Set include resolver for #include directives */
    void set_include_resolver(std::function<bool(const std::string&, std::string&)> resolver) {
        preprocessor_.set_include_resolver(std::move(resolver));
    }

    /* Add predefined macros */
    void define(const std::string& name, const std::string& value) {
        preprocessor_.define(name, value);
    }

    /* Compile source to IR */
    ShaderProgram compile(const std::string& source,
                          const CompileOptions* options = nullptr);

    /* Access diagnostics */
    const DiagnosticList& diagnostics() const noexcept { return diagnostics_; }

    /* Access the source manager (for file names in diagnostics) */
    const SourceManager& sources() const noexcept { return sources_; }
    SourceManager& sources() noexcept { return sources_; }

private:
    void detect_language(const std::string& source);
    void setup_predefined_macros(KxShaderLanguage language, KxShaderStage stage);

    DiagnosticList& diagnostics_;
    SourceManager sources_;
    Preprocessor preprocessor_{diagnostics_, sources_};
    ParserConfig parser_config_;
    std::string program_ir_;
};

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERCOMPILER_H */