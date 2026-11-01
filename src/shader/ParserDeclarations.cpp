#include "ParserDeclarations.h"

#include <algorithm>
#include "ShaderIR.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Main entry point
 * --------------------------------------------------------------------- */
ShaderProgram ParserDeclarations::parse(const std::vector<Token>& tokens, uint32_t source_file) {
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

void ParserDeclarations::parse_program(const std::vector<Token>&, uint32_t source_file) {
    (void)source_file;
    while (!check(TokenType::End)) {
        parse_declaration();
    }
}

/* -----------------------------------------------------------------------
 * Top-level declarations
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_declaration() {
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

/* -----------------------------------------------------------------------
 * Struct definitions
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_struct() {
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

/* -----------------------------------------------------------------------
 * Uniform buffers / cbuffers / push constants
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_uniform_buffer() {
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

/* -----------------------------------------------------------------------
 * Input/Output/Push constant blocks
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_block(const std::string& keyword, StorageClass storage) {
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

/* -----------------------------------------------------------------------
 * Variable declarations
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_variable_declaration() {
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

/* -----------------------------------------------------------------------
 * Function definitions
 * --------------------------------------------------------------------- */
void ParserDeclarations::parse_function() {
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

} // namespace shader
} // namespace kx