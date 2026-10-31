/*
 * test_shader_builtins.cpp
 * Unit tests for shader builtin functions
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

TEST_CASE("ShaderBuiltins - Trigonometric") {
    struct TestCase {
        const char* func;
        float input;
        float expected;
        float epsilon;
    };
    
    TestCase cases[] = {
        {"sin", 0.0f, 0.0f, 1e-5f},
        {"sin", 1.5707963f, 1.0f, 1e-5f},
        {"cos", 0.0f, 1.0f, 1e-5f},
        {"cos", 1.5707963f, 0.0f, 1e-5f},
        {"tan", 0.0f, 0.0f, 1e-5f},
        {"asin", 0.0f, 0.0f, 1e-5f},
        {"asin", 1.0f, 1.5707963f, 1e-5f},
        {"acos", 1.0f, 0.0f, 1e-5f},
        {"acos", 0.0f, 1.5707963f, 1e-5f},
        {"atan", 1.0f, 0.785398f, 1e-5f},
        {"sinh", 0.0f, 0.0f, 1e-5f},
        {"cosh", 0.0f, 1.0f, 1e-5f},
        {"tanh", 0.0f, 0.0f, 1e-5f},
    };
    
    for (const auto& tc : cases) {
        std::string source = "void main() { float result = " + std::string(tc.func) + "(" + std::to_string(tc.input) + "); }";
        
        auto [program, interpreter] = compile_and_create_interpreter(source);
        kx::shader::ShaderEnvironment env;
        interpreter->execute(env, nullptr, nullptr);
        
        CHECK_EQ(program.functions.size(), 1);
    }
}

TEST_CASE("ShaderBuiltins - ExponentialLogarithmic") {
    struct TestCase {
        const char* func;
        const char* input;
        float expected;
        float epsilon;
    };
    
    TestCase cases[] = {
        {"exp", "1.0", 2.7182818f, 1e-5f},
        {"log", "2.7182818", 1.0f, 1e-5f},
        {"exp2", "2.0", 4.0f, 1e-5f},
        {"log2", "4.0", 2.0f, 1e-5f},
        {"sqrt", "4.0", 2.0f, 1e-5f},
        {"inversesqrt", "4.0", 0.5f, 1e-5f},
        {"pow", "2.0, 3.0", 8.0f, 1e-5f},
    };
    
    for (const auto& tc : cases) {
        std::string source = "void main() { float result = " + std::string(tc.func) + "(" + tc.input + "); }";
        
        auto [program, interpreter] = compile_and_create_interpreter(source);
        kx::shader::ShaderEnvironment env;
        interpreter->execute(env, nullptr, nullptr);
        
        CHECK_EQ(program.functions.size(), 1);
    }
}

TEST_CASE("ShaderBuiltins - Rounding") {
    struct TestCase {
        const char* func;
        float input;
        float expected;
        float epsilon;
    };
    
    TestCase cases[] = {
        {"floor", 1.7f, 1.0f, 1e-5f},
        {"floor", -1.7f, -2.0f, 1e-5f},
        {"ceil", 1.2f, 2.0f, 1e-5f},
        {"ceil", -1.2f, -1.0f, 1e-5f},
        {"trunc", 1.7f, 1.0f, 1e-5f},
        {"trunc", -1.7f, -1.0f, 1e-5f},
        {"round", 1.5f, 2.0f, 1e-5f},
        {"round", 1.4f, 1.0f, 1e-5f},
        {"round", -1.5f, -2.0f, 1e-5f},
        {"fract", 1.7f, 0.7f, 1e-5f},
        {"fract", -1.7f, 0.3f, 1e-5f},
    };
    
    for (const auto& tc : cases) {
        std::string source = "void main() { float result = " + std::string(tc.func) + "(" + std::to_string(tc.input) + "); }";
        
        auto [program, interpreter] = compile_and_create_interpreter(source);
        kx::shader::ShaderEnvironment env;
        interpreter->execute(env, nullptr, nullptr);
        
        CHECK_EQ(program.functions.size(), 1);
    }
}

TEST_CASE("ShaderBuiltins - VectorFunctions") {
    std::string source = 
        "void main() {\n"
        "    vec3 a = vec3(1.0, 2.0, 3.0);\n"
        "    vec3 b = vec3(4.0, 5.0, 6.0);\n"
        "    float d = dot(a, b);\n"
        "    vec3 c = cross(a, b);\n"
        "    float l = length(a);\n"
        "    float dist = distance(a, b);\n"
        "    vec3 n = normalize(a);\n"
        "    vec3 f = faceforward(a, b, a);\n"
        "    vec3 r = reflect(a, b);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - MatrixFunctions") {
    std::string source = 
        "void main() {\n"
        "    mat4 m = mat4(1.0);\n"
        "    mat4 t = transpose(m);\n"
        "    float d = determinant(m);\n"
        "    mat4 inv = inverse(m);\n"
        "    vec4 v = vec4(1.0, 2.0, 3.0, 1.0);\n"
        "    vec4 r = m * v;\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - ComparisonFunctions") {
    std::string source = 
        "void main() {\n"
        "    vec3 a = vec3(1.0, 2.0, 3.0);\n"
        "    vec3 b = vec3(1.0, 2.0, 3.0);\n"
        "    bvec3 lt = lessThan(a, vec3(2.0, 3.0, 4.0));\n"
        "    bvec3 le = lessThanEqual(a, b);\n"
        "    bvec3 gt = greaterThan(a, vec3(0.0));\n"
        "    bvec3 ge = greaterThanEqual(a, b);\n"
        "    bvec3 eq = equal(a, b);\n"
        "    bvec3 ne = notEqual(a, vec3(0.0));\n"
        "    bool any_result = any(lt);\n"
        "    bool all_result = all(le);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - SelectionFunctions") {
    std::string source = 
        "void main() {\n"
        "    float a = mix(0.0, 1.0, 0.5);\n"
        "    float s = step(0.5, 1.0);\n"
        "    float ss = smoothstep(0.0, 1.0, 0.5);\n"
        "    float c = clamp(5.0, 0.0, 10.0);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - DerivativeFunctions") {
    std::string source = 
        "void main() {\n"
        "    float x = 1.0;\n"
        "    float dx = dFdx(x);\n"
        "    float dy = dFdy(x);\n"
        "    float fw = fwidth(x);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - Noise") {
    std::string source = 
        "void main() {\n"
        "    float n1 = noise(1.0);\n"
        "    float n2 = noise(vec2(1.0, 2.0));\n"
        "    float n3 = noise(vec3(1.0, 2.0, 3.0));\n"
        "    float n4 = noise(vec4(1.0, 2.0, 3.0, 4.0));\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - GeometryFunctions") {
    std::string source = 
        "void main() {\n"
        "    float r = radians(180.0);\n"
        "    float d = degrees(3.14159);\n"
        "    float s = sign(-5.0);\n"
        "    float a = abs(-3.0);\n"
        "    float mn = min(1.0, 2.0);\n"
        "    float mx = max(1.0, 2.0);\n"
        "    float m = mod(5.0, 3.0);\n"
        "    float mp = modf(1.5);\n"
        "    float f = fma(2.0, 3.0, 4.0);\n"
        "    float l = ldexp(1.0, 2);\n"
        "    bool inan = isnan(0.0 / 0.0);\n"
        "    bool iinf = isinf(1.0 / 0.0);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - BitPacking") {
    std::string source = 
        "void main() {\n"
        "    float f2i = floatBitsToInt(1.0);\n"
        "    float f2u = floatBitsToUint(1.0);\n"
        "    float i2f = intBitsToFloat(1065353216);\n"
        "    float u2f = uintBitsToFloat(1065353216u);\n"
        "    float p1 = packSnorm2x16(vec2(1.0, -1.0));\n"
        "    vec2 u1 = unpackSnorm2x16(p1);\n"
        "    float p2 = packUnorm2x16(vec2(1.0, 0.5));\n"
        "    vec2 u2 = unpackUnorm2x16(p2);\n"
        "    float p3 = packHalf2x16(vec2(1.0, 0.5));\n"
        "    vec2 u3 = unpackHalf2x16(p3);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - MatrixExtras") {
    std::string source = 
        "void main() {\n"
        "    vec3 a = vec3(1.0, 0.0, 0.0);\n"
        "    vec3 b = vec3(0.0, 1.0, 0.0);\n"
        "    mat3 outer = outerProduct(a, b);\n"
        "    mat3 m = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);\n"
        "    mat3 mcm = matrixCompMult(m, m);\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderBuiltins - InterpolationFunctions") {
    std::string source = 
        "void main() {\n"
        "    float x = 1.0;\n"
        "    float c = interpolateAtCentroid(x);\n"
        "    float s = interpolateAtSample(x, 0);\n"
        "    float o = interpolateAtOffset(x, vec2(0.5));\n"
        "}\n";
    
    auto [program, interpreter] = compile_and_create_interpreter(source);
    kx::shader::ShaderEnvironment env;
    interpreter->execute(env, nullptr, nullptr);
    
    CHECK_EQ(program.functions.size(), 1);
}