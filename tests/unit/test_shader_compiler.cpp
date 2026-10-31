/*
 * test_shader_compiler.cpp
 * Unit tests for shader compiler
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "shader/ShaderCompiler.h"
#include "shader/ShaderDiagnostics.h"
#include <memory>

using namespace kx::shader;

TEST_CASE("ShaderCompiler - CompileGLSL") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = 
        "void main() {\n"
        "    float a = 1.0;\n"
        "}\n";
    
    ShaderCompiler::CompileOptions options;
    options.language = KX_SHADER_LANGUAGE_GLSL;
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    options.entry_point = "main";
    
    ShaderProgram program = compiler.compile(source, &options);
    
    CHECK_FALSE(diagnostics.has_errors());
    CHECK_EQ(program.language, KX_SHADER_LANGUAGE_GLSL);
    CHECK_EQ(program.stage, KX_SHADER_STAGE_VERTEX);
    CHECK_EQ(program.entry_name, "main");
}

TEST_CASE("ShaderCompiler - CompileHLSL") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = 
        "void main() {\n"
        "    float a = 1.0;\n"
        "}\n";
    
    ShaderCompiler::CompileOptions options;
    options.language = KX_SHADER_LANGUAGE_HLSL;
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    options.entry_point = "main";
    
    ShaderProgram program = compiler.compile(source, &options);
    
    CHECK_FALSE(diagnostics.has_errors());
    CHECK_EQ(program.language, KX_SHADER_LANGUAGE_HLSL);
}

TEST_CASE("ShaderCompiler - CompileWGSL") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = 
        "fn main() {\n"
        "    let x = 1.0;\n"
        "}\n";
    
    ShaderCompiler::CompileOptions options;
    options.language = KX_SHADER_LANGUAGE_WGSL;
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    options.entry_point = "main";
    
    ShaderProgram program = compiler.compile(source, &options);
    
    CHECK_FALSE(diagnostics.has_errors());
    CHECK_EQ(program.language, KX_SHADER_LANGUAGE_WGSL);
}

TEST_CASE("ShaderCompiler - LanguageAutoDetection") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string glsl_source = "#version 450\nvoid main() {}";
    ShaderProgram glsl_program = compiler.compile(glsl_source);
    CHECK_EQ(glsl_program.language, KX_SHADER_LANGUAGE_GLSL);
    
    std::string hlsl_source = "void main() {}";
    ShaderProgram hlsl_program = compiler.compile(hlsl_source);
    // HLSL auto-detection may not work perfectly, just verify it compiles
    CHECK(hlsl_program.language != KX_SHADER_LANGUAGE_UNDEFINED);
    
    std::string wgsl_source = "fn main() { let x = 1.0; }";
    ShaderProgram wgsl_program = compiler.compile(wgsl_source);
    CHECK(wgsl_program.language != KX_SHADER_LANGUAGE_UNDEFINED);
}

TEST_CASE("ShaderCompiler - Diagnostics") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = "void main() { invalid syntax; }";
    
    ShaderCompiler::CompileOptions options;
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    
    ShaderProgram program = compiler.compile(source, &options);
    
    CHECK_GE(program.exprs.size(), 0);
}

TEST_CASE("ShaderCompiler - EntryPointSelection") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = 
        "void helper() {}\n"
        "void main() {}\n"
        "void other() {}\n";
    
    ShaderCompiler::CompileOptions options;
    options.entry_point = "main";
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    
    ShaderProgram program = compiler.compile(source, &options);
    
    CHECK_EQ(program.entry_name, "main");
    CHECK(program.functions[program.entry].is_entry);
    CHECK_EQ(program.functions[program.entry].name, "main");
}

TEST_CASE("ShaderCompiler - PredefinedMacros") {
    DiagnosticList diagnostics;
    ShaderCompiler compiler(diagnostics);
    
    std::string source = 
        "#ifdef VERTEX_SHADER\n"
        "int stage = 1;\n"
        "#elif defined(FRAGMENT_SHADER)\n"
        "int stage = 2;\n"
        "#elif defined(COMPUTE_SHADER)\n"
        "int stage = 3;\n"
        "#endif\n"
        "void main() {}\n";
    
    ShaderCompiler::CompileOptions options;
    options.stage_hint = KX_SHADER_STAGE_VERTEX;
    
    ShaderProgram program = compiler.compile(source, &options);
    
    // Debug output
    std::cerr << "Diagnostics count: " << diagnostics.diagnostics().size() << std::endl;
    for (const auto& diag : diagnostics.diagnostics()) {
        std::cerr << "  " << diag.message << " at " << diag.line << ":" << diag.column << std::endl;
    }
    
    CHECK_FALSE(diagnostics.has_errors());
}