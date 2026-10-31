/*
 * ShaderParser.cpp
 *
 * Unified recursive-descent parser for GLSL, HLSL, WGSL and KXSL.
 */

#include "ShaderParser.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

#include "ShaderTypes.h"

namespace kx {
namespace shader {

namespace {

bool is_builtin_function(const std::string& name) {
    static const std::vector<std::string> builtins = {
        "abs", "acos", "acosh", "all", "any", "asin", "asinh", "atan", "atanh",
        "atomicAdd", "atomicAnd", "atomicCompSwap", "atomicExchange", "atomicMax",
        "atomicMin", "atomicOr", "atomicXor",
        "clamp", "cos", "cosh", "cross", "degrees", "determinant", "distance",
        "dot", "dFdx", "dFdy", "equal", "exp", "exp2", "faceforward", "floatBitsToInt",
        "floatBitsToUint", "floor", "fma", "fract", "frexp", "fwidth", "greaterThan",
        "greaterThanEqual", "inverse", "inversesqrt", "interpolateAtCentroid",
        "interpolateAtOffset", "interpolateAtSample", "intBitsToFloat", "isinf",
        "isnan", "ldexp", "length", "lessThan", "lessThanEqual", "log", "log2",
        "matrixCompMult", "max", "min", "mix", "mod", "modf", "noise", "normalize",
        "notEqual", "outerProduct", "packHalf2x16", "packSnorm2x16", "packUnorm2x16",
        "pow", "radians", "reflect", "refract", "round", "roundEven", "sign",
        "sin", "sinh", "smoothstep", "sqrt", "step", "tan", "tanh", "texture",
        "textureGrad", "textureLod", "textureSize", "transpose", "trunc",
        "uintBitsToFloat", "unpackHalf2x16", "unpackSnorm2x16", "unpackUnorm2x16",
    };
    return std::binary_search(builtins.begin(), builtins.end(), name);
}

} /* namespace */

const Token& Parser::peek() const {
    if (position_ < tokens_.size()) return tokens_[position_];
    static const Token end_token;
    return end_token;
}

const Token& Parser::peek(int offset) const {
    std::size_t idx = position_ + offset;
    if (idx < tokens_.size()) return tokens_[idx];
    static const Token end_token;
    return end_token;
}

const Token& Parser::consume() {
    if (position_ < tokens_.size()) return tokens_[position_++];
    static const Token end_token;
    return end_token;
}

bool Parser::match(TokenType type, const std::string& text) {
    if (position_ < tokens_.size() && tokens_[position_].type == type &&
        (text.empty() || tokens_[position_].text == text)) {
        ++position_;
        return true;
    }
    return false;
}

bool Parser::match(TokenType type, const std::string& text1, const std::string& text2) {
    if (position_ < tokens_.size() && tokens_[position_].type == type &&
        (tokens_[position_].text == text1 || tokens_[position_].text == text2)) {
        ++position_;
        return true;
    }
    return false;
}

bool Parser::check(TokenType type, const std::string& text) const {
    if (position_ < tokens_.size() && tokens_[position_].type == type) {
        return text.empty() || tokens_[position_].text == text;
    }
    return false;
}

void Parser::expect(TokenType type, const std::string& text) {
    if (!match(type, text)) {
        const Token& tok = peek();
        error("expected " + (text.empty() ? Parser::token_type_name(type) : text) +
              ", got " + tok.text);
    }
}

void Parser::expect_identifier(std::string& out_name) {
    if (match(TokenType::Identifier)) {
        out_name = tokens_[position_ - 1].text;
    } else {
        const Token& tok = peek();
        error("expected identifier, got " + tok.text);
    }
}

const char* Parser::token_type_name(TokenType t) {
    switch (t) {
        case TokenType::End: return "end of file";
        case TokenType::Identifier: return "identifier";
        case TokenType::Number: return "number";
        case TokenType::String: return "string";
        case TokenType::Punct: return "punctuator";
    }
    return "unknown";
}

uint32_t Parser::make_expr(ExprKind kind, const ShaderType& type, float fconst) {
    Expr expr;
    expr.kind = kind;
    expr.type = type;
    expr.fconst = fconst;
    expr.line = peek().line;
    program_.exprs.push_back(expr);
    return static_cast<uint32_t>(program_.exprs.size() - 1);
}

ShaderProgram Parser::parse(const std::vector<Token>& tokens, uint32_t source_file) {
    tokens_ = tokens;
    position_ = 0;
    program_ = ShaderProgram{};
    program_.language = config_.language;
    struct_names_.clear();
    current_function_ = UINT32_MAX;

    parse_program(tokens_, source_file);

    /* Find entry point */
    for (uint32_t i = 0; i < program_.functions.size(); ++i) {
        if (program_.functions[i].name == program_.entry_name) {
            program_.entry = i;
            program_.functions[i].is_entry = true;
            break;
        }
    }

    return std::move(program_);
}

void Parser::parse_program(const std::vector<Token>&, uint32_t source_file) {
    (void)source_file;
    while (!check(TokenType::End)) {
        parse_declaration();
    }
}

void Parser::parse_declaration() {
    const Token& tok = peek();

    if (tok.type == TokenType::Identifier) {
        const std::string& name = tok.text;

        if (name == "struct") {
            parse_struct();
            return;
        }
        if (name == "uniform" || name == "cbuffer" || name == "ConstantBuffer") {
            parse_uniform_buffer();
            return;
        }
        if (name == "push" || name == "PushConstants") {
            parse_block("push", StorageClass::PushConstant);
            return;
        }
        if (name == "in" || name == "out" || name == "attribute" || name == "varying" ||
            name == "input" || name == "output") {
            parse_block(name, name == "in" || name == "attribute" || name == "input"
                              ? StorageClass::Input
                              : StorageClass::Output);
            return;
        }
        if (name == "texture" || name == "Texture2D" || name == "Texture3D" ||
            name == "TextureCube" || name == "Texture2DArray") {
            parse_variable_declaration();
            return;
        }
        if (name == "sampler" || name == "SamplerState" || name == "SamplerComparisonState") {
            parse_variable_declaration();
            return;
        }
        if (name == "buffer" || name == "StructuredBuffer" || name == "RWStructuredBuffer" ||
            name == "ByteAddressBuffer" || name == "RWByteAddressBuffer") {
            parse_variable_declaration();
            return;
        }

        /* Could be a function, variable, or type */
        std::size_t saved = position_;
        ShaderType type = parse_type();

        if (check(TokenType::Identifier)) {
            /* Look ahead to see if it's a function */
            std::size_t look = position_;
            while (look < tokens_.size() && tokens_[look].type != TokenType::End &&
                   tokens_[look].type != TokenType::Punct) ++look;
            if (look < tokens_.size() && tokens_[look].type == TokenType::Punct &&
                tokens_[look].text == "(") {
                /* Function declaration */
                Function func;
                func.name = tokens_[position_].text;
                func.return_type = type;
                func.line = tokens_[position_].line;
                consume(); /* consume function name */
                /* Add placeholder function first so current_function_ index is valid */
                program_.functions.push_back(std::move(func));
                uint32_t func_index = static_cast<uint32_t>(program_.functions.size() - 1);
                uint32_t saved_function = current_function_;
                current_function_ = func_index;
                /* Parse directly into the function in the vector */
                parse_function();
                current_function_ = saved_function;
                return;
            }
        }

        /* Variable declaration */
        position_ = saved;
        parse_variable_declaration();
    } else if (tok.type == TokenType::Punct && tok.text == "{") {
        /* Block at global scope - treat as error */
        error("unexpected block at global scope");
        consume();
        int depth = 1;
        while (position_ < tokens_.size() && depth > 0) {
            if (tokens_[position_].type == TokenType::Punct) {
                if (tokens_[position_].text == "{") ++depth;
                else if (tokens_[position_].text == "}") --depth;
            }
            ++position_;
        }
    } else {
        error("unexpected token: " + tok.text);
        consume();
    }
}

void Parser::parse_struct() {
    expect(TokenType::Identifier, "struct");
    std::string name;
    expect_identifier(name);

    StructDef struct_def;
    struct_def.name = name;
    struct_def.members.clear();

    expect(TokenType::Punct, "{");

    uint32_t offset = 0;
    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        ShaderType member_type = parse_type();

        std::string member_name;
        expect_identifier(member_name);

        StructMember member;
        member.name = member_name;
        member.type = member_type;
        member.offset = offset;
        offset += member_type.byte_size();

        struct_def.members.push_back(std::move(member));

        expect(TokenType::Punct, ";");
    }

    expect(TokenType::Punct, "}");
    struct_def.size = offset;
    program_.structs.push_back(std::move(struct_def));
    struct_names_.push_back(name);

    /* Optional typedef name */
    if (check(TokenType::Identifier)) {
        std::string typedef_name;
        expect_identifier(typedef_name);
        /* TODO: register typedef */
    }
}

void Parser::parse_uniform_buffer() {
    const Token& tok = peek();
    std::string keyword = tok.text;
    consume();

    std::string block_name;
    if (check(TokenType::Identifier)) {
        expect_identifier(block_name);
    } else {
        block_name = "Uniforms";
    }

    expect(TokenType::Punct, "{");

    UniformBlock block;
    block.name = block_name;
    block.push_constant = (keyword == "push" || keyword == "PushConstants");

    if (!block.push_constant) {
        /* Parse layout qualifier if present */
        if (check(TokenType::Identifier, "layout")) {
            consume();
            expect(TokenType::Punct, "(");
            while (!check(TokenType::Punct, ")") && !check(TokenType::End)) {
                if (check(TokenType::Identifier, "set")) {
                    consume();
                    expect(TokenType::Punct, "=");
                    if (check(TokenType::Number)) {
                        block.set = static_cast<uint32_t>(tokens_[position_ - 1].number);
                    }
                } else if (check(TokenType::Identifier, "binding")) {
                    consume();
                    expect(TokenType::Punct, "=");
                    if (check(TokenType::Number)) {
                        block.binding = static_cast<uint32_t>(tokens_[position_ - 1].number);
                    }
                }
                if (check(TokenType::Punct, ",")) consume();
            }
            expect(TokenType::Punct, ")");
        }
    }

    uint32_t offset = 0;
    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        ShaderType member_type = parse_type();
        std::string member_name;
        expect_identifier(member_name);

        Variable var;
        var.name = member_name;
        var.type = member_type;
        var.storage = block.push_constant ? StorageClass::PushConstant : StorageClass::Uniform;
        var.set = block.set;
        var.binding = block.binding;
        var.offset = offset;
        var.size = member_type.byte_size();
        offset += var.size;

        block.members.push_back(std::move(var));
        expect(TokenType::Punct, ";");
    }

    expect(TokenType::Punct, "}");
    block.size = offset;

    /* Optional instance name */
    if (check(TokenType::Identifier)) {
        std::string instance_name;
        expect_identifier(instance_name);
    }

    expect(TokenType::Punct, ";");

    if (block.push_constant) {
        program_.blocks.push_back(std::move(block));
    } else {
        program_.blocks.push_back(std::move(block));
    }
}

void Parser::parse_block(const std::string& keyword, StorageClass storage) {
    consume(); /* keyword */

    std::string block_name;
    if (check(TokenType::Identifier)) {
        expect_identifier(block_name);
    } else {
        block_name = keyword + "Block";
    }

    expect(TokenType::Punct, "{");

    uint32_t offset = 0;
    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        ShaderType member_type = parse_type();
        std::string member_name;
        expect_identifier(member_name);

        Variable var;
        var.name = member_name;
        var.type = member_type;
        var.storage = storage;
        var.offset = offset;
        var.size = member_type.byte_size();
        offset += var.size;

        program_.globals.push_back(std::move(var));
        expect(TokenType::Punct, ";");
    }

    expect(TokenType::Punct, "}");
    expect(TokenType::Punct, ";");
}

void Parser::parse_variable_declaration() {
    ShaderType type = parse_type();

    if (check(TokenType::Punct, ";")) {
        /* Forward declaration / type without name - skip */
        consume();
        return;
    }

    std::string name;
    expect_identifier(name);

    Variable var;
    var.name = name;
    var.type = type;
    var.storage = StorageClass::Local;

    if (match(TokenType::Punct, "[")) {
        var.array = true;
        if (check(TokenType::Number)) {
            var.array_size = static_cast<uint32_t>(tokens_[position_ - 1].number);
        }
        expect(TokenType::Punct, "]");
    }

    if (match(TokenType::Punct, "=")) {
        /* Initializer - parse expression but don't store in IR for now */
        parse_expression();
    }

    expect(TokenType::Punct, ";");

    if (current_function_ != UINT32_MAX) {
        program_.functions[current_function_].locals.push_back(var);
    } else {
        program_.globals.push_back(var);
    }
}

void Parser::parse_function() {
    expect(TokenType::Punct, "(");

    /* Parameters */
    while (!check(TokenType::Punct, ")") && !check(TokenType::End)) {
        ShaderType param_type = parse_type();
        std::string param_name;
        expect_identifier(param_name);

        Variable param;
        param.name = param_name;
        param.type = param_type;
        param.storage = StorageClass::Local;

        program_.functions[current_function_].params.push_back(std::move(param));

        if (!check(TokenType::Punct, ")")) expect(TokenType::Punct, ",");
    }
    expect(TokenType::Punct, ")");

    /* Function body */
    expect(TokenType::Punct, "{");

    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        parse_statement(program_.functions[current_function_].body);
    }
    expect(TokenType::Punct, "}");
}

/* -----------------------------------------------------------------------
 * Type parsing
 * --------------------------------------------------------------------- */
ShaderType Parser::parse_type() {
    if (check(TokenType::Identifier)) {
        const std::string& name = peek().text;

        if (name == "bool") return parse_base_type();
        if (name == "int" || name == "int32") return parse_base_type();
        if (name == "uint" || name == "uint32") return parse_base_type();
        if (name == "float" || name == "float32") return parse_base_type();
        if (name == "half" || name == "float16") return parse_base_type();

        if (name == "vec2" || name == "vec3" || name == "vec4" ||
            name == "ivec2" || name == "ivec3" || name == "ivec4" ||
            name == "uvec2" || name == "uvec3" || name == "uvec4" ||
            name == "bvec2" || name == "bvec3" || name == "bvec4" ||
            name == "float2" || name == "float3" || name == "float4" ||
            name == "int2" || name == "int3" || name == "int4" ||
            name == "uint2" || name == "uint3" || name == "uint4") {
            return parse_vector_type();
        }

        if (name == "mat2" || name == "mat3" || name == "mat4" ||
            name == "mat2x2" || name == "mat2x3" || name == "mat2x4" ||
            name == "mat3x2" || name == "mat3x3" || name == "mat3x4" ||
            name == "mat4x2" || name == "mat4x3" || name == "mat4x4" ||
            name == "float2x2" || name == "float3x3" || name == "float4x4") {
            return parse_matrix_type();
        }

        if (name == "texture" || name == "Texture2D" || name == "Texture3D" ||
            name == "TextureCube" || name == "Texture2DArray" ||
            name == "texture_2d" || name == "texture_3d" ||
            name == "texture_cube" || name == "texture_2d_array") {
            return parse_texture_type();
        }

        if (name == "sampler" || name == "SamplerState" ||
            name == "SamplerComparisonState" || name == "sampler_comparison") {
            return parse_sampler_type();
        }

        if (name == "buffer" || name == "StructuredBuffer" || name == "RWStructuredBuffer" ||
            name == "ByteAddressBuffer" || name == "RWByteAddressBuffer") {
            consume();
            return ShaderType{TypeClass::StorageBuffer};
        }

        /* User-defined struct */
        if (std::find(struct_names_.begin(), struct_names_.end(), name) != struct_names_.end()) {
            consume();
            return ShaderType{TypeClass::Struct, 0, 0, 0, 4}; /* struct_id resolved later */
        }
    }

    /* Unknown type - consume and default to float */
    consume();
    return make_float();
}

ShaderType Parser::parse_base_type() {
    const std::string& name = peek().text;
    if (name == "bool") { consume(); return make_bool(); }
    if (name == "int" || name == "int32") { consume(); return make_int(); }
    if (name == "uint" || name == "uint32") { consume(); return make_uint(); }
    if (name == "float" || name == "float32") { consume(); return make_float(); }
    if (name == "half" || name == "float16") { consume(); return make_float(); }
    consume();
    return make_float();
}

ShaderType Parser::parse_vector_type() {
    const std::string& name = peek().text;
    uint32_t components = 4;
    TypeClass cls = TypeClass::Float;

    if (name == "vec2" || name == "float2") components = 2;
    else if (name == "vec3" || name == "float3") components = 3;
    else if (name == "vec4" || name == "float4") components = 4;
    else if (name == "ivec2" || name == "int2") { components = 2; cls = TypeClass::Int; }
    else if (name == "ivec3" || name == "int3") { components = 3; cls = TypeClass::Int; }
    else if (name == "ivec4" || name == "int4") { components = 4; cls = TypeClass::Int; }
    else if (name == "uvec2" || name == "uint2") { components = 2; cls = TypeClass::UInt; }
    else if (name == "uvec3" || name == "uint3") { components = 3; cls = TypeClass::UInt; }
    else if (name == "uvec4" || name == "uint4") { components = 4; cls = TypeClass::UInt; }
    else if (name == "bvec2") { components = 2; cls = TypeClass::Bool; }
    else if (name == "bvec3") { components = 3; cls = TypeClass::Bool; }
    else if (name == "bvec4") { components = 4; cls = TypeClass::Bool; }

    consume();
    ShaderType t = make_vector(components);
    t.cls = cls;
    return t;
}

ShaderType Parser::parse_matrix_type() {
    const std::string& name = peek().text;
    uint32_t cols = 4, rows = 4;

    if (name == "mat2" || name == "float2x2") { cols = 2; rows = 2; }
    else if (name == "mat3" || name == "float3x3") { cols = 3; rows = 3; }
    else if (name == "mat4" || name == "float4x4") { cols = 4; rows = 4; }
    else if (name == "mat2x3") { cols = 2; rows = 3; }
    else if (name == "mat2x4") { cols = 2; rows = 4; }
    else if (name == "mat3x2") { cols = 3; rows = 2; }
    else if (name == "mat3x4") { cols = 3; rows = 4; }
    else if (name == "mat4x2") { cols = 4; rows = 2; }
    else if (name == "mat4x3") { cols = 4; rows = 3; }

    consume();
    return make_matrix(cols, rows);
}

ShaderType Parser::parse_struct_type() {
    consume();
    ShaderType t{};
    t.cls = TypeClass::Struct;
    return t;
}

ShaderType Parser::parse_texture_type() {
    consume();
    return ShaderType{TypeClass::Texture};
}

ShaderType Parser::parse_sampler_type() {
    consume();
    return ShaderType{TypeClass::Sampler};
}

/* -----------------------------------------------------------------------
 * Statement parsing
 * --------------------------------------------------------------------- */
void Parser::parse_statement(std::vector<Stmt>& stmts) {
    const Token& tok = peek();

    if (tok.type == TokenType::Punct && tok.text == "{") {
        Stmt block;
        block.kind = StmtKind::Block;
        block.line = tok.line;
        parse_block_stmt(block.body);
        stmts.push_back(std::move(block));
    } else if (tok.type == TokenType::Identifier) {
        const std::string& name = tok.text;

        if (name == "if") {
            parse_if_stmt(stmts);
        } else if (name == "for") {
            parse_for_stmt(stmts);
        } else if (name == "while") {
            parse_while_stmt(stmts);
        } else if (name == "return") {
            parse_return_stmt(stmts);
        } else if (name == "break") {
            parse_break_stmt(stmts);
        } else if (name == "continue") {
            parse_continue_stmt(stmts);
        } else if (name == "discard") {
            parse_discard_stmt(stmts);
        } else if (name == "switch") {
            parse_switch_stmt(stmts);
        } else if (name == "case" || name == "default") {
            /* Handled by switch */
            parse_statement(stmts);
        } else {
            /* Could be declaration or expression */
            std::size_t saved = position_;
            ShaderType type = parse_type();

            if (check(TokenType::Identifier)) {
                std::size_t look = position_;
                while (look < tokens_.size() && tokens_[look].type != TokenType::End &&
                       tokens_[look].type != TokenType::Punct) ++look;
                if (look < tokens_.size() && tokens_[look].type == TokenType::Punct &&
                    tokens_[look].text == "(") {
                    /* This is a function call or constructor - expression statement */
                    position_ = saved;
                } else {
                    /* Declaration */
                    position_ = saved;
                    parse_declare_stmt(stmts);
                    return;
                }
            } else {
                position_ = saved;
            }

            /* Expression statement */
            Stmt expr_stmt;
            expr_stmt.kind = StmtKind::Expression;
            expr_stmt.line = peek().line;
            expr_stmt.expr = parse_expression();
            stmts.push_back(std::move(expr_stmt));
            expect(TokenType::Punct, ";");
        }
    } else {
        error("unexpected token in statement: " + tok.text);
        consume();
    }
}

void Parser::parse_block_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Punct, "{");
    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        parse_statement(stmts);
    }
    expect(TokenType::Punct, "}");
}

void Parser::parse_declare_stmt(std::vector<Stmt>& stmts) {
    ShaderType type = parse_type();
    std::string name;
    expect_identifier(name);

    Stmt stmt;
    stmt.kind = StmtKind::Declare;
    stmt.line = peek().line;

    Variable var;
    var.name = name;
    var.type = type;
    var.storage = StorageClass::Local;

    if (match(TokenType::Punct, "[")) {
        var.array = true;
        if (check(TokenType::Number)) {
            var.array_size = static_cast<uint32_t>(tokens_[position_ - 1].number);
        }
        expect(TokenType::Punct, "]");
    }

    /* Store variable in function locals or globals */
    if (current_function_ != UINT32_MAX) {
        program_.functions[current_function_].locals.push_back(var);
        stmt.var = static_cast<uint32_t>(program_.functions[current_function_].locals.size() - 1);
    } else {
        program_.globals.push_back(var);
        stmt.var = static_cast<uint32_t>(program_.globals.size() - 1) + 10000; /* offset */
    }

    if (match(TokenType::Punct, "=")) {
        stmt.expr = parse_expression();
    }

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_assign_stmt(std::vector<Stmt>& stmts) {
    uint32_t lhs = parse_expression(0);
    Stmt stmt;
    stmt.kind = StmtKind::Assign;
    stmt.line = peek().line;
    stmt.expr = lhs;
    if (match(TokenType::Punct, "=")) {
        stmt.expr2 = parse_expression();
    }
    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_if_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "if");
    expect(TokenType::Punct, "(");
    uint32_t cond = parse_expression();
    expect(TokenType::Punct, ")");

    Stmt stmt;
    stmt.kind = StmtKind::If;
    stmt.line = peek().line;
    stmt.expr = cond;

    parse_statement(stmt.body);

    if (match(TokenType::Identifier, "else")) {
        parse_statement(stmt.else_body);
    }

    stmts.push_back(std::move(stmt));
}

void Parser::parse_for_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "for");
    expect(TokenType::Punct, "(");

    Stmt stmt;
    stmt.kind = StmtKind::For;
    stmt.line = peek().line;

    /* Init */
    if (!check(TokenType::Punct, ";")) {
        if (check(TokenType::Identifier) ||
            (check(TokenType::Identifier) && tokens_[position_ + 1].type == TokenType::Identifier)) {
            std::size_t saved = position_;
            ShaderType type = parse_type();
            if (check(TokenType::Identifier) && position_ < tokens_.size()) {
                parse_declare_stmt(stmt.init);
            } else {
                position_ = saved;
                stmt.init.push_back(Stmt{StmtKind::Expression, 0, 0, 0, UINT32_MAX,
                                        peek().line, {}, {}, {}, {}});
                stmt.init.back().expr = parse_expression();
            }
        } else {
            stmt.init.push_back(Stmt{StmtKind::Expression, 0, 0, 0, UINT32_MAX,
                                    peek().line, {}, {}, {}, {}});
            stmt.init.back().expr = parse_expression();
        }
    }
    expect(TokenType::Punct, ";");

    /* Condition */
    if (!check(TokenType::Punct, ";")) {
        stmt.expr = parse_expression();
    }
    expect(TokenType::Punct, ";");

    /* Increment */
    if (!check(TokenType::Punct, ")")) {
        Stmt inc;
        inc.kind = StmtKind::Expression;
        inc.expr = parse_expression();
        stmt.increment.push_back(std::move(inc));
    }
    expect(TokenType::Punct, ")");

    parse_statement(stmt.body);
    stmts.push_back(std::move(stmt));
}

void Parser::parse_while_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "while");
    expect(TokenType::Punct, "(");
    uint32_t cond = parse_expression();
    expect(TokenType::Punct, ")");

    Stmt stmt;
    stmt.kind = StmtKind::While;
    stmt.line = peek().line;
    stmt.expr = cond;

    parse_statement(stmt.body);
    stmts.push_back(std::move(stmt));
}

void Parser::parse_return_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "return");

    Stmt stmt;
    stmt.kind = StmtKind::Return;
    stmt.line = peek().line;

    if (!check(TokenType::Punct, ";")) {
        stmt.expr = parse_expression();
    }

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_break_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "break");

    Stmt stmt;
    stmt.kind = StmtKind::Break;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_continue_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "continue");

    Stmt stmt;
    stmt.kind = StmtKind::Continue;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_discard_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "discard");

    Stmt stmt;
    stmt.kind = StmtKind::Discard;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void Parser::parse_switch_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "switch");
    expect(TokenType::Punct, "(");
    uint32_t value = parse_expression();
    expect(TokenType::Punct, ")");
    expect(TokenType::Punct, "{");

    Stmt stmt;
    stmt.kind = StmtKind::Switch;
    stmt.line = peek().line;
    stmt.expr = value;

    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        if (match(TokenType::Identifier, "case")) {
            uint32_t case_val = parse_expression();
            stmt.case_values.push_back(case_val);
            expect(TokenType::Punct, ":");
            std::vector<Stmt> case_body;
            while (!check(TokenType::Punct, "}") &&
                   !check(TokenType::Identifier, "case") &&
                   !check(TokenType::Identifier, "default") &&
                   !check(TokenType::End)) {
                parse_statement(case_body);
            }
            stmt.cases.push_back(std::move(case_body));
        } else if (match(TokenType::Identifier, "default")) {
            stmt.case_values.push_back(UINT32_MAX);
            expect(TokenType::Punct, ":");
            std::vector<Stmt> case_body;
            while (!check(TokenType::Punct, "}") &&
                   !check(TokenType::Identifier, "case") &&
                   !check(TokenType::Identifier, "default") &&
                   !check(TokenType::End)) {
                parse_statement(case_body);
            }
            stmt.cases.push_back(std::move(case_body));
        } else {
            error("expected 'case' or 'default' in switch");
            consume();
        }
    }
    expect(TokenType::Punct, "}");
    stmts.push_back(std::move(stmt));
}

/* -----------------------------------------------------------------------
 * Expression parsing
 * --------------------------------------------------------------------- */
uint32_t Parser::parse_expression(int precedence) {
    return parse_binary_expr(precedence);
}

uint32_t Parser::parse_primary() {
    const Token& tok = peek();

    if (tok.type == TokenType::Number) {
        return parse_literal_expr();
    }
    if (tok.type == TokenType::Identifier) {
        return parse_identifier_expr();
    }
    if (tok.type == TokenType::Punct && tok.text == "(") {
        consume(); /* '(' */
        uint32_t expr = parse_expression();
        expect(TokenType::Punct, ")");
        return expr;
    }

    error("unexpected token in expression: " + tok.text);
    consume();
    return make_expr(ExprKind::Literal, make_float(), 0.0f);
}

uint32_t Parser::parse_literal_expr() {
    const Token& tok = consume();

    Expr expr;
    expr.kind = ExprKind::Literal;
    expr.line = tok.line;

    if (tok.text == "true" || tok.text == "false") {
        expr.type = make_bool();
        expr.bconst = (tok.text == "true");
    } else if (tok.is_integer) {
        if (tok.text.size() > 0 && (tok.text.back() == 'u' || tok.text.back() == 'U')) {
            expr.type = make_uint();
            expr.uconst = static_cast<uint32_t>(tok.number);
        } else {
            expr.type = make_int();
            expr.iconst = static_cast<int32_t>(tok.number);
        }
    } else {
        expr.type = make_float();
        expr.fconst = static_cast<float>(tok.number);
    }

    program_.exprs.push_back(expr);
    return static_cast<uint32_t>(program_.exprs.size() - 1);
}

uint32_t Parser::parse_identifier_expr() {
    const Token& tok = consume();
    const std::string name = tok.text;

    /* Check for type constructor */
    if (is_type_name(name)) {
        return parse_construct_expr();
    }

    /* Variable reference */
    Expr expr;
    expr.kind = ExprKind::VarRef;
    expr.line = tok.line;

    /* Find variable */
    if (current_function_ != UINT32_MAX) {
        const auto& locals = program_.functions[current_function_].locals;
        for (uint32_t i = 0; i < locals.size(); ++i) {
            if (locals[i].name == name) {
                expr.local = i;
                expr.type = locals[i].type;
                break;
            }
        }
    }
    if (expr.local == UINT32_MAX) {
        for (uint32_t i = 0; i < program_.globals.size(); ++i) {
            if (program_.globals[i].name == name) {
                expr.var = i;
                expr.type = program_.globals[i].type;
                break;
            }
        }
    }

    program_.exprs.push_back(expr);
    uint32_t result = static_cast<uint32_t>(program_.exprs.size() - 1);

    /* Postfix: member access, swizzle, index, call */
    return parse_postfix_expr(result);
}

bool Parser::is_type_name(const std::string& name) const {
    static const std::vector<std::string> types = {
        "bool", "int", "uint", "float", "half",
        "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4",
        "uvec2", "uvec3", "uvec4", "bvec2", "bvec3", "bvec4",
        "float2", "float3", "float4", "int2", "int3", "int4",
        "uint2", "uint3", "uint4",
        "mat2", "mat3", "mat4", "mat2x2", "mat2x3", "mat2x4",
        "mat3x2", "mat3x3", "mat3x4", "mat4x2", "mat4x3", "mat4x4",
        "float2x2", "float3x3", "float4x4",
        "texture", "Texture2D", "Texture3D", "TextureCube", "Texture2DArray",
        "sampler", "SamplerState", "SamplerComparisonState",
        "buffer", "StructuredBuffer", "RWStructuredBuffer"
    };
    return std::binary_search(types.begin(), types.end(), name);
}

uint32_t Parser::parse_postfix_expr(uint32_t base) {
    while (true) {
        if (check(TokenType::Punct, ".")) {
            consume();
            if (check(TokenType::Identifier)) {
                const Token& tok = consume();
                /* Member access or swizzle */
                const std::string& member = tok.text;

                if (member.size() <= 4 &&
                    member.find_first_not_of("xyzwrgba") == std::string::npos) {
                    /* Swizzle */
                    Expr expr;
                    expr.kind = ExprKind::Swizzle;
                    expr.line = tok.line;
                    expr.args.push_back(base);
                    for (char c : member) {
                        uint8_t idx = (c == 'x' || c == 'r') ? 0 :
                                      (c == 'y' || c == 'g') ? 1 :
                                      (c == 'z' || c == 'b') ? 2 : 3;
                        if (expr.swizzle_count < 4) {
                            expr.swizzle[expr.swizzle_count++] = idx;
                        }
                    }
                    if (base < program_.exprs.size()) {
                        const ShaderType& base_type = program_.exprs[base].type;
                        if (base_type.is_vector()) {
                            expr.type = make_vector(expr.swizzle_count);
                        } else {
                            expr.type = base_type;
                        }
                    }
                    program_.exprs.push_back(expr);
                    base = static_cast<uint32_t>(program_.exprs.size() - 1);
                } else {
                    /* Struct member */
                    Expr expr;
                    expr.kind = ExprKind::Member;
                    expr.line = tok.line;
                    expr.args.push_back(base);
                    /* member name -> index lookup deferred */
                    if (base < program_.exprs.size()) {
                        expr.type = program_.exprs[base].type;
                    }
                    program_.exprs.push_back(expr);
                    base = static_cast<uint32_t>(program_.exprs.size() - 1);
                }
            }
        } else if (check(TokenType::Punct, "[")) {
            consume();
            uint32_t index = parse_expression();
            expect(TokenType::Punct, "]");

            Expr expr;
            expr.kind = ExprKind::Index;
            expr.line = peek().line;
            expr.args.push_back(base);
            expr.args.push_back(index);
            if (base < program_.exprs.size()) {
                expr.type = program_.exprs[base].type;
            }
            program_.exprs.push_back(expr);
            base = static_cast<uint32_t>(program_.exprs.size() - 1);
        } else if (check(TokenType::Punct, "(")) {
            /* Function call */
            base = parse_call_expr(base);
        } else {
            break;
        }
    }
    return base;
}

uint32_t Parser::parse_construct_expr() {
    const Token& tok = consume();
    ShaderType type = parse_type(); /* consumes the type name */

    expect(TokenType::Punct, "(");

    Expr expr;
    expr.kind = ExprKind::Construct;
    expr.type = type;
    expr.line = tok.line;

    if (!check(TokenType::Punct, ")")) {
        while (true) {
            uint32_t arg = parse_expression();
            expr.args.push_back(arg);
            if (!match(TokenType::Punct, ",")) break;
        }
    }
    expect(TokenType::Punct, ")");

    program_.exprs.push_back(expr);
    return static_cast<uint32_t>(program_.exprs.size() - 1);
}

uint32_t Parser::parse_unary_expr() {
    if (check(TokenType::Punct)) {
        const std::string& op = peek().text;
        if (op == "+" || op == "-" || op == "!" || op == "~" ||
            op == "++" || op == "--") {
            const Token& t = consume();
            uint32_t operand = parse_unary_expr();

            Expr expr;
            expr.kind = ExprKind::Unary;
            expr.unop = op_to_unary(op);
            expr.line = t.line;
            expr.args.push_back(operand);

            /* Type inference */
            if (operand < program_.exprs.size()) {
                expr.type = program_.exprs[operand].type;
            }

            program_.exprs.push_back(expr);
            return static_cast<uint32_t>(program_.exprs.size() - 1);
        }
    }
    return parse_primary();
}

uint32_t Parser::parse_binary_expr(int min_precedence) {
    uint32_t lhs = parse_unary_expr();

    while (true) {
        if (position_ >= tokens_.size()) break;
        const Token& tok = peek();
        if (tok.type != TokenType::Punct) break;

        int prec = get_precedence(tok.text);
        if (prec < min_precedence) break;

        const std::string op = tok.text;
        consume();

        uint32_t rhs = parse_binary_expr(prec + 1);

        Expr expr;
        expr.kind = ExprKind::Binary;
        expr.binop = op_to_binary(op);
        expr.line = tok.line;
        expr.args.push_back(lhs);
        expr.args.push_back(rhs);

        /* Type inference - promote */
        if (lhs < program_.exprs.size() && rhs < program_.exprs.size()) {
            expr.type = promote(program_.exprs[lhs].type, program_.exprs[rhs].type);
        }

        program_.exprs.push_back(expr);
        lhs = static_cast<uint32_t>(program_.exprs.size() - 1);
    }

    /* Ternary */
    if (check(TokenType::Punct, "?")) {
        consume();
        uint32_t true_expr = parse_expression();
        expect(TokenType::Punct, ":");
        uint32_t false_expr = parse_expression();

        Expr expr;
        expr.kind = ExprKind::Ternary;
        expr.line = peek().line;
        expr.args.push_back(lhs);
        expr.args.push_back(true_expr);
        expr.args.push_back(false_expr);

        if (true_expr < program_.exprs.size() && false_expr < program_.exprs.size()) {
            expr.type = promote(program_.exprs[true_expr].type, program_.exprs[false_expr].type);
        }

        program_.exprs.push_back(expr);
        lhs = static_cast<uint32_t>(program_.exprs.size() - 1);
    }

    /* Postfix operators */
    return parse_postfix_expr(lhs);
}

uint32_t Parser::parse_call_expr(uint32_t callee) {
    expect(TokenType::Punct, "(");

    Expr expr;
    expr.kind = ExprKind::Call;
    expr.line = peek().line;
    expr.args.push_back(callee);

    if (!check(TokenType::Punct, ")")) {
        while (true) {
            uint32_t arg = parse_expression();
            expr.args.push_back(arg);
            if (!match(TokenType::Punct, ",")) break;
        }
    }
    expect(TokenType::Punct, ")");

    /* Is it a builtin? */
    if (callee < program_.exprs.size() &&
        program_.exprs[callee].kind == ExprKind::VarRef) {
        const Token& tok = tokens_[position_ - expr.args.size() - 2];
        if (is_builtin_function(tok.text)) {
            expr.builtin = static_cast<uint32_t>(program_.builtin_names.size());
            program_.builtin_names.push_back(tok.text);
            expr.func = UINT32_MAX;
        }
    }

    /* Return type inference - for builtins use first arg type, otherwise void */
    if (expr.builtin != UINT32_MAX) {
        if (expr.args.size() > 1) {
            expr.type = program_.exprs[expr.args[1]].type;
        }
    }

    program_.exprs.push_back(expr);
    return static_cast<uint32_t>(program_.exprs.size() - 1);
}

int Parser::get_precedence(const std::string& op) const {
    if (op == "||") return 1;
    if (op == "&&") return 2;
    if (op == "|") return 3;
    if (op == "^") return 4;
    if (op == "&") return 5;
    if (op == "==" || op == "!=") return 6;
    if (op == "<" || op == ">" || op == "<=" || op == ">=") return 7;
    if (op == "<<" || op == ">>") return 8;
    if (op == "+" || op == "-") return 9;
    if (op == "*" || op == "/" || op == "%") return 10;
    return -1;
}

bool Parser::is_assign_op(const std::string& op) const {
    return op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" ||
           op == "%=" || op == "&=" || op == "|=" || op == "^=" || op == "<<=" || op == ">>=";
}

BinaryOp Parser::op_to_binary(const std::string& op) const {
    if (op == "+") return BinaryOp::Add;
    if (op == "-") return BinaryOp::Subtract;
    if (op == "*") return BinaryOp::Multiply;
    if (op == "/") return BinaryOp::Divide;
    if (op == "%") return BinaryOp::Modulo;
    if (op == "<") return BinaryOp::Less;
    if (op == "<=") return BinaryOp::LessEqual;
    if (op == ">") return BinaryOp::Greater;
    if (op == ">=") return BinaryOp::GreaterEqual;
    if (op == "==") return BinaryOp::Equal;
    if (op == "!=") return BinaryOp::NotEqual;
    if (op == "&&") return BinaryOp::LogicalAnd;
    if (op == "||") return BinaryOp::LogicalOr;
    if (op == "&") return BinaryOp::BitAnd;
    if (op == "|") return BinaryOp::BitOr;
    if (op == "^") return BinaryOp::BitXor;
    if (op == "<<") return BinaryOp::ShiftLeft;
    if (op == ">>") return BinaryOp::ShiftRight;
    return BinaryOp::Add;
}

UnaryOp Parser::op_to_unary(const std::string& op) const {
    if (op == "-") return UnaryOp::Negate;
    if (op == "!") return UnaryOp::LogicalNot;
    if (op == "~") return UnaryOp::BitNot;
    if (op == "+") return UnaryOp::Plus;
    if (op == "++") return UnaryOp::PreIncrement;
    if (op == "--") return UnaryOp::PreDecrement;
    return UnaryOp::Negate;
}

void Parser::add_diagnostic(KxShaderSeverity severity, const std::string& message) {
    const Token& tok = peek();
    diagnostics_.report(severity, tok.file, tok.line, tok.column, tok.offset, message);
}

void Parser::error(const std::string& message) {
    add_diagnostic(KX_SHADER_SEVERITY_ERROR, message);
}

void Parser::warning(const std::string& message) {
    add_diagnostic(KX_SHADER_SEVERITY_WARNING, message);
}

} /* namespace shader */
} /* namespace kx */