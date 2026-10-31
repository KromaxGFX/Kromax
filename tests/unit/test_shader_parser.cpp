/*
 * test_shader_parser.cpp
 * Unit tests for shader parser
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "shader/ShaderParser.h"
#include "shader/ShaderLexer.h"
#include "shader/ShaderDiagnostics.h"
#include "shader/ShaderIR.h"

using namespace kx::shader;

static ShaderProgram parse_source(const std::string& source) {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    Parser parser(diagnostics);
    
    uint32_t file_id = sources.add("test", source);
    std::vector<Token> tokens = Lexer::tokenize(source, file_id);
    std::vector<Token> processed;
    pp.process(tokens, processed);
    
    return parser.parse(processed, file_id);
}

TEST_CASE("ShaderParser - SimpleFunction") {
    std::string source = 
        "void main() {\n"
        "    float x = 1.0;\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_EQ(program.functions.size(), 1);
    CHECK_EQ(program.functions[0].name, "main");
    CHECK_EQ(program.entry, 0);
    CHECK(program.functions[0].is_entry);
}

TEST_CASE("ShaderParser - FunctionWithParameters") {
    std::string source = 
        "vec4 myFunc(vec3 pos, float time) {\n"
        "    return vec4(pos, time);\n"
        "}\n"
        "void main() {\n"
        "    vec4 result = myFunc(vec3(1.0), 0.5);\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_EQ(program.functions.size(), 2);
    
    bool found_myfunc = false;
    for (const auto& f : program.functions) {
        if (f.name == "myFunc") {
            found_myfunc = true;
            CHECK_EQ(f.params.size(), 2);
            CHECK_EQ(f.params[0].name, "pos");
            CHECK_EQ(f.params[1].name, "time");
        }
    }
    CHECK(found_myfunc);
}

TEST_CASE("ShaderParser - StructDeclaration") {
    std::string source = 
        "struct MyStruct {\n"
        "    vec3 position;\n"
        "    float value;\n"
        "};\n"
        "void main() {\n"
        "    MyStruct s;\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_EQ(program.structs.size(), 1);
    CHECK_EQ(program.structs[0].name, "MyStruct");
    CHECK_EQ(program.structs[0].members.size(), 2);
}

TEST_CASE("ShaderParser - UniformBlock") {
    std::string source = 
        "layout(set = 0, binding = 0) uniform CameraData {\n"
        "    mat4 view;\n"
        "    mat4 proj;\n"
        "    vec3 camera_pos;\n"
        "};\n"
        "void main() {}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_GE(program.blocks.size(), 1);
    bool found_camera = false;
    for (const auto& b : program.blocks) {
        if (b.name == "CameraData") {
            found_camera = true;
            CHECK_EQ(b.set, 0);
            CHECK_EQ(b.binding, 0);
            CHECK_EQ(b.members.size(), 3);
        }
    }
    CHECK(found_camera);
}

TEST_CASE("ShaderParser - VectorOperations") {
    std::string source = 
        "void main() {\n"
        "    vec3 a = vec3(1.0, 2.0, 3.0);\n"
        "    vec3 b = vec3(4.0, 5.0, 6.0);\n"
        "    vec3 c = a + b;\n"
        "    float d = dot(a, b);\n"
        "    vec3 e = cross(a, b);\n"
        "    float len = length(a);\n"
        "    vec3 n = normalize(a);\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_GT(program.exprs.size(), 0);
    CHECK_EQ(program.functions.size(), 1);
}

TEST_CASE("ShaderParser - MatrixOperations") {
    std::string source = 
        "void main() {\n"
        "    mat4 m = mat4(1.0);\n"
        "    mat4 n = transpose(m);\n"
        "    float det = determinant(m);\n"
        "    mat4 inv = inverse(m);\n"
        "    vec4 v = vec4(1.0, 2.0, 3.0, 1.0);\n"
        "    vec4 r = m * v;\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_GT(program.exprs.size(), 0);
}

TEST_CASE("ShaderParser - ControlFlow") {
    std::string source = 
        "void main() {\n"
        "    float x = 1.0;\n"
        "    if (x > 0.5) {\n"
        "        x = 2.0;\n"
        "    } else {\n"
        "        x = 3.0;\n"
        "    }\n"
        "    for (int i = 0; i < 10; i++) {\n"
        "        x += 0.1;\n"
        "    }\n"
        "    while (x < 5.0) {\n"
        "        x += 1.0;\n"
        "    }\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_EQ(program.functions.size(), 1);
    CHECK_GT(program.functions[0].body.size(), 0);
}

TEST_CASE("ShaderParser - Swizzle") {
    std::string source = 
        "void main() {\n"
        "    vec4 v = vec4(1.0, 2.0, 3.0, 4.0);\n"
        "    vec3 xyz = v.xyz;\n"
        "    vec2 rg = v.rg;\n"
        "    float x = v.x;\n"
        "    v.xy = vec2(0.0);\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_GT(program.exprs.size(), 0);
    int swizzle_count = 0;
    for (const auto& e : program.exprs) {
        if (e.kind == ExprKind::Swizzle) swizzle_count++;
    }
    CHECK_GT(swizzle_count, 0);
}

TEST_CASE("ShaderParser - TextureAndSampler") {
    std::string source = 
        "texture tex;\n"
        "sampler samp;\n"
        "void main() {\n"
        "    vec4 color = texture(tex, vec2(0.5));\n"
        "}\n";
    
    ShaderProgram program = parse_source(source);
    
    CHECK_EQ(program.globals.size(), 2);
    int texture_count = 0, sampler_count = 0;
    for (const auto& g : program.globals) {
        if (g.type.cls == TypeClass::Texture) texture_count++;
        if (g.type.cls == TypeClass::Sampler) sampler_count++;
    }
    CHECK_EQ(texture_count, 1);
    CHECK_EQ(sampler_count, 1);
}