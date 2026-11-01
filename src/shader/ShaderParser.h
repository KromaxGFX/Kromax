#ifndef KROMAX_SRC_SHADER_SHADERPARSER_H
#define KROMAX_SRC_SHADER_SHADERPARSER_H

/*
 * ShaderParser.h
 *
 * Unified recursive-descent parser for GLSL, HLSL, WGSL and KXSL. The parser
 * consumes the preprocessed token stream and produces a ShaderProgram IR.
 * Dialect differences are handled by small virtual methods and configuration
 * flags rather than completely separate parsers.
 *
 * Modularized into linear inheritance chain:
 * - ParserCore: token handling, error reporting, expression creation, operator utilities
 * - ParserTypes: type parsing (base, vector, matrix, texture, sampler, struct)
 * - ParserExpressions: primary, literal, identifier, postfix, unary, binary, ternary, call
 * - ParserStatements: if, for, while, return, break, continue, switch, blocks
 * - ParserDeclarations: program, structs, uniform buffers, functions, variables
 * - ParserBuiltins: builtin function name list
 */

#include <cstdint>
#include <string>
#include <vector>

#include "ShaderIR.h"
#include "ShaderDiagnostics.h"
#include "ShaderLexer.h"
#include "ParserConfig.h"
#include "ParserCore.h"
#include "ParserTypes.h"
#include "ParserExpressions.h"
#include "ParserStatements.h"
#include "ParserDeclarations.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Parser - main parser class using linear inheritance of modular components
 * --------------------------------------------------------------------- */
class Parser : public ParserDeclarations {
public:
    explicit Parser(DiagnosticList& diagnostics, const ParserConfig& config = {})
        : ParserDeclarations(diagnostics, config) {}

    using ParserDeclarations::parse;

    const ParserConfig& config() const noexcept { return ParserCore::config(); }

    /* Expose core parser state access */
    using ParserCore::tokens;
    using ParserCore::position;
    using ParserCore::program;
    using ParserCore::struct_names;
    using ParserCore::current_function;
    using ParserCore::diagnostics;

    /* Expose token operations */
    using ParserCore::peek;
    using ParserCore::consume;
    using ParserCore::match;
    using ParserCore::check;
    using ParserCore::expect;
    using ParserCore::expect_identifier;
    using ParserCore::token_type_name;

    /* Expose expression creation */
    using ParserCore::make_expr;
    using ParserCore::make_bool;
    using ParserCore::make_int;
    using ParserCore::make_uint;
    using ParserCore::make_float;
    using ParserCore::make_vector;
    using ParserCore::make_matrix;

    /* Expose type promotion and operators */
    using ParserCore::promote;
    using ParserCore::get_precedence;
    using ParserCore::is_assign_op;
    using ParserCore::op_to_binary;
    using ParserCore::op_to_unary;

    /* Expose error reporting */
    using ParserCore::add_diagnostic;
    using ParserCore::error;
    using ParserCore::warning;

    /* Expose builtin check */
    using ParserCore::is_builtin;

    /* Expose types */
    using ParserTypes::parse_type;
    using ParserTypes::is_type_name;

    /* Expose declarations */
    using ParserDeclarations::parse_program;
    using ParserDeclarations::parse_declaration;
    using ParserDeclarations::parse_struct;
    using ParserDeclarations::parse_uniform_buffer;
    using ParserDeclarations::parse_block;
    using ParserDeclarations::parse_variable_declaration;
    using ParserDeclarations::parse_function;

    /* Expose statements */
    using ParserStatements::parse_statement;
    using ParserStatements::parse_block_stmt;
    using ParserStatements::parse_declare_stmt;
    using ParserStatements::parse_assign_stmt;
    using ParserStatements::parse_if_stmt;
    using ParserStatements::parse_for_stmt;
    using ParserStatements::parse_while_stmt;
    using ParserStatements::parse_return_stmt;
    using ParserStatements::parse_break_stmt;
    using ParserStatements::parse_continue_stmt;
    using ParserStatements::parse_discard_stmt;
    using ParserStatements::parse_switch_stmt;

    /* Expose expressions - public interface only */
    using ParserExpressions::parse_expression;

private:
    /* All functionality inherited via linear inheritance chain */
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_SHADERPARSER_H