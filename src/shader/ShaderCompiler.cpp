/*
 * ShaderCompiler.cpp
 *
 * Orchestrates the preprocessor, parser and IR validation.
 */

#include "ShaderCompiler.h"

#include <algorithm>
#include <string>

namespace kx {
namespace shader {

void ShaderCompiler::detect_language(const std::string& source) {
    /* Simple heuristics */
    if (source.find("#version") != std::string::npos ||
        source.find("layout(") != std::string::npos ||
        source.find("in vec3") != std::string::npos ||
        source.find("out vec3") != std::string::npos) {
        parser_config_.language = KX_SHADER_LANGUAGE_GLSL;
        return;
    }
    if (source.find("cbuffer") != std::string::npos ||
        source.find("StructuredBuffer") != std::string::npos ||
        source.find("RWStructuredBuffer") != std::string::npos ||
        source.find("Texture2D") != std::string::npos ||
        source.find("SamplerState") != std::string::npos) {
        parser_config_.language = KX_SHADER_LANGUAGE_HLSL;
        return;
    }
    if (source.find("var<") != std::string::npos ||
        source.find("@group") != std::string::npos ||
        source.find("@binding") != std::string::npos ||
        source.find("fn ") != std::string::npos) {
        parser_config_.language = KX_SHADER_LANGUAGE_WGSL;
        return;
    }
    /* Default to GLSL */
    parser_config_.language = KX_SHADER_LANGUAGE_GLSL;
}

void ShaderCompiler::setup_predefined_macros(KxShaderLanguage language, KxShaderStage stage) {
    preprocessor_.define("__KX_VERSION__", "100");
    preprocessor_.define("__KX_LANGUAGE__", std::to_string(static_cast<int>(language)));

    switch (stage) {
        case KX_SHADER_STAGE_VERTEX:
            preprocessor_.define("VERTEX_SHADER", "1");
            preprocessor_.define("__VERTEX__", "1");
            break;
        case KX_SHADER_STAGE_FRAGMENT:
            preprocessor_.define("FRAGMENT_SHADER", "1");
            preprocessor_.define("__FRAGMENT__", "1");
            break;
        case KX_SHADER_STAGE_COMPUTE:
            preprocessor_.define("COMPUTE_SHADER", "1");
            preprocessor_.define("__COMPUTE__", "1");
            break;
    }

    if (language == KX_SHADER_LANGUAGE_GLSL) {
        preprocessor_.define("__GLSL__", "1");
        preprocessor_.define("GL_ES", "1");
    } else if (language == KX_SHADER_LANGUAGE_HLSL) {
        preprocessor_.define("__HLSL__", "1");
        preprocessor_.define("HLSL", "1");
    } else if (language == KX_SHADER_LANGUAGE_WGSL) {
        preprocessor_.define("__WGSL__", "1");
    } else if (language == KX_SHADER_LANGUAGE_KXSL) {
        preprocessor_.define("__KXSL__", "1");
    }
}

ShaderProgram ShaderCompiler::compile(const std::string& source,
                                       const CompileOptions* options) {
    CompileOptions opts;
    if (options) opts = *options;

    /* Detect language if not specified */
    if (opts.language == KX_SHADER_LANGUAGE_UNDEFINED) {
        detect_language(source);
    } else {
        parser_config_.language = opts.language;
    }

    parser_config_.entry_point_required = !opts.entry_point.empty();

    /* Add source to source manager */
    uint32_t file_id = sources_.add("<source>", source);

    /* Setup predefined macros */
    setup_predefined_macros(parser_config_.language, opts.stage_hint);

    /* Tokenize */
    std::vector<Token> tokens = Lexer::tokenize(source, file_id);

    /* Preprocess */
    std::vector<Token> processed;
    preprocessor_.process(tokens, processed);

    /* Parse */
    Parser parser(diagnostics_, parser_config_);
    ShaderProgram program = parser.parse(processed, file_id);

    program.stage = opts.stage_hint;
    program.language = parser_config_.language;
    program.entry_name = opts.entry_point;

    /* Validate */
    if (opts.dump_ir) {
        program_ir_ = dump_program(program);
    }

    return program;
}

} /* namespace shader */
} /* namespace kx */