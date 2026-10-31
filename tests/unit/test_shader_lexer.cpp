/*
 * test_shader_lexer.cpp
 * Unit tests for shader lexer and preprocessor
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "shader/ShaderLexer.h"
#include "shader/ShaderDiagnostics.h"

using namespace kx::shader;

TEST_CASE("ShaderLexer - BasicTokenization") {
    DiagnosticList diagnostics;
    SourceManager sources;
    uint32_t file_id = sources.add("test", "float x = 1.0;");
    
    std::vector<Token> tokens = Lexer::tokenize("float x = 1.0;", file_id);
    
    CHECK(tokens.size() > 0);
    // Should have: float, x, =, 1.0, ;, End
    CHECK(tokens[0].type == TokenType::Identifier);
    CHECK(tokens[0].text == "float");
    CHECK(tokens[1].type == TokenType::Identifier);
    CHECK(tokens[1].text == "x");
    CHECK(tokens[2].type == TokenType::Punct);
    CHECK(tokens[2].text == "=");
    CHECK(tokens[3].type == TokenType::Number);
    CHECK(tokens[4].type == TokenType::Punct);
    CHECK(tokens[4].text == ";");
}

TEST_CASE("ShaderLexer - VectorTypes") {
    DiagnosticList diagnostics;
    SourceManager sources;
    uint32_t file_id = sources.add("test", "vec3 pos = vec3(1.0, 2.0, 3.0);");
    
    std::vector<Token> tokens = Lexer::tokenize("vec3 pos = vec3(1.0, 2.0, 3.0);", file_id);
    
    bool found_vec3 = false;
    for (const auto& t : tokens) {
        if (t.text == "vec3") found_vec3 = true;
    }
    CHECK(found_vec3);
}

TEST_CASE("ShaderPreprocessor - SimpleDefine") {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    
    pp.define("MY_VALUE", "42");
    
    std::vector<Token> input = Lexer::tokenize("int x = MY_VALUE;", 0);
    std::vector<Token> output;
    pp.process(input, output);
    
    // Should have expanded MY_VALUE to 42
    bool found_42 = false;
    for (const auto& t : output) {
        if (t.type == TokenType::Number && t.text == "42") {
            found_42 = true;
            break;
        }
    }
    CHECK(found_42);
}

TEST_CASE("ShaderPreprocessor - ConditionalCompilation") {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    
    pp.define("ENABLE_FEATURE", "1");
    
    std::string source = 
        "#if ENABLE_FEATURE\n"
        "int x = 1;\n"
        "#else\n"
        "int x = 2;\n"
        "#endif\n";
    
    std::vector<Token> input = Lexer::tokenize(source, 0);
    std::vector<Token> output;
    pp.process(input, output);
    
    // Should have x = 1, not x = 2
    bool found_1 = false, found_2 = false;
    for (size_t i = 0; i < output.size(); ++i) {
        if (output[i].type == TokenType::Number) {
            if (output[i].text == "1") found_1 = true;
            if (output[i].text == "2") found_2 = true;
        }
    }
    CHECK(found_1);
    CHECK(!found_2);
}

TEST_CASE("ShaderPreprocessor - IfDef") {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    
    pp.define("DEBUG", "1");
    
    std::string source = 
        "#ifdef DEBUG\n"
        "int debug_val = 1;\n"
        "#endif\n";
    
    std::vector<Token> input = Lexer::tokenize(source, 0);
    std::vector<Token> output;
    pp.process(input, output);
    
    bool found_debug = false;
    for (const auto& t : output) {
        if (t.text == "debug_val") found_debug = true;
    }
    CHECK(found_debug);
}

TEST_CASE("ShaderPreprocessor - IfNotDef") {
    DiagnosticList diagnostics;
    SourceManager sources;
    Preprocessor pp(diagnostics, sources);
    
    // Don't define RELEASE
    
    std::string source = 
        "#ifndef RELEASE\n"
        "int debug_val = 1;\n"
        "#endif\n";
    
    std::vector<Token> input = Lexer::tokenize(source, 0);
    std::vector<Token> output;
    pp.process(input, output);
    
    bool found_debug = false;
    for (const auto& t : output) {
        if (t.text == "debug_val") found_debug = true;
    }
    CHECK(found_debug);
}