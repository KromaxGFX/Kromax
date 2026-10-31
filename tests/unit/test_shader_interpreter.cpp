/*
 * test_shader_interpreter.cpp
 * Unit tests for shader interpreter
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "shader/ShaderInterpreter.h"
#include "shader/ShaderParser.h"
#include "shader/ShaderLexer.h"
#include "shader/ShaderDiagnostics.h"
#include <memory>

using namespace kx::shader;

static std::pair<ShaderProgram, std::unique_ptr<Interpreter>> compile_and_create_interpreter(const std::string& source) {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    Parser parser(diagnostics);
    
    uint32_t file_id = sources.add("test", source);
    std::vector<Token> tokens = Lexer::tokenize(source, file_id);
    std::vector<Token> processed;
    pp.process(tokens, processed);
    
    ShaderProgram program = parser.parse(processed, file_id);
    auto interpreter = std::make_unique<Interpreter>(program);
    
    return {std::move(program), std::move(interpreter)};
}

TEST_CASE("ShaderInterpreter - SimpleArithmetic") {
    std::string source = 
        "void main() {\n"
        "    float a = 2.0 + 3.0;\n"
        "    float b = 10.0 - 4.0;\n"
        "    float c = 3.0 * 4.0;\n"
        "    float d = 20.0 / 5.0;\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - VectorOperations") {
    std::string source = 
        "void main() {\n"
        "    vec3 v1 = vec3(1.0, 2.0, 3.0);\n"
        "    vec3 v2 = vec3(4.0, 5.0, 6.0);\n"
        "    vec3 sum = v1 + v2;\n"
        "    vec3 diff = v1 - v2;\n"
        "    vec3 prod = v1 * v2;\n"
        "    float dot_result = dot(v1, v2);\n"
        "    float len = length(v1);\n"
        "    vec3 norm = normalize(v1);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - MatrixOperations") {
    std::string source = 
        "void main() {\n"
        "    mat4 m = mat4(1.0);\n"
        "    mat4 transposed = transpose(m);\n"
        "    float det = determinant(m);\n"
        "    mat4 inv = inverse(m);\n"
        "    vec4 v = vec4(1.0, 2.0, 3.0, 1.0);\n"
        "    vec4 result = m * v;\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - MathFunctions") {
    std::string source = 
        "void main() {\n"
        "    float a = sin(1.0);\n"
        "    float b = cos(1.0);\n"
        "    float c = sqrt(4.0);\n"
        "    float d = pow(2.0, 3.0);\n"
        "    float e = floor(1.5);\n"
        "    float f = ceil(1.5);\n"
        "    float g = abs(-5.0);\n"
        "    float h = min(1.0, 2.0);\n"
        "    float i = max(1.0, 2.0);\n"
        "    float j = clamp(5.0, 0.0, 10.0);\n"
        "    float k = mix(0.0, 1.0, 0.5);\n"
        "    float l = step(0.5, 1.0);\n"
        "    float m = smoothstep(0.0, 1.0, 0.5);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - ControlFlow") {
    std::string source = 
        "void main() {\n"
        "    float x = 0.0;\n"
        "    if (true) {\n"
        "        x = 1.0;\n"
        "    } else {\n"
        "        x = 2.0;\n"
        "    }\n"
        "    for (int i = 0; i < 5; i++) {\n"
        "        x += 1.0;\n"
        "    }\n"
        "    while (x < 10.0) {\n"
        "        x += 2.0;\n"
        "    }\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - VectorSwizzle") {
    std::string source = 
        "void main() {\n"
        "    vec4 v = vec4(1.0, 2.0, 3.0, 4.0);\n"
        "    vec3 xyz = v.xyz;\n"
        "    vec2 rg = v.rg;\n"
        "    float x = v.x;\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderInterpreter - UniformsAndPushConstants") {
    std::string source = 
        "layout(set = 0, binding = 0) uniform CameraData {\n"
        "    mat4 view_proj;\n"
        "    vec3 camera_pos;\n"
        "};\n"
        "void main() {\n"
        "    vec4 pos = vec4(1.0, 2.0, 3.0, 1.0);\n"
        "    vec4 clip = view_proj * pos;\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}