#ifndef KROMAX_SRC_SHADER_SHADERPARSER_H
#define KROMAX_SRC_SHADER_SHADERPARSER_H

/*
 * ShaderParser.h
 *
 * Unified recursive-descent parser for GLSL, HLSL, WGSL and KXSL. The parser
 * consumes the preprocessed token stream and produces a ShaderProgram IR.
 * Dialect differences are handled by small virtual methods and configuration
 * flags rather than completely separate parsers.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "ShaderIR.h"
#include "ShaderDiagnostics.h"
#include "ShaderLexer.h"

namespace kx {
namespace shader {

struct ParserConfig {
    KxShaderLanguage language = KX_SHADER_LANGUAGE_GLSL;
    bool allow_struct = true;
    bool allow_uniform_buffer = true;
    bool allow_push_constant = true;
    bool require_version = false;
    bool entry_point_required = false;
};

/* -----------------------------------------------------------------------
 * Parser
 * --------------------------------------------------------------------- */
class Parser {
public:
    explicit Parser(DiagnosticList& diagnostics, const ParserConfig& config = {})
        : diagnostics_(diagnostics), config_(config) {}

    ShaderProgram parse(const std::vector<Token>& tokens, uint32_t source_file = 0);

    const ParserConfig& config() const noexcept { return config_; }

private:
    /* Token access */
    const Token& peek() const;
    const Token& peek(int offset) const;
    const Token& consume();
    bool match(TokenType type, const std::string& text = "");
    bool match(TokenType type, const std::string& text1, const std::string& text2);
    bool check(TokenType type, const std::string& text = "") const;
    void expect(TokenType type, const std::string& text = "");
    void expect_identifier(std::string& out_name);

    /* Top level */
    void parse_program(const std::vector<Token>& tokens, uint32_t source_file);
    void parse_declaration();
    void parse_function();

    /* Types */
    ShaderType parse_type();
    ShaderType parse_base_type();
    ShaderType parse_vector_type();
    ShaderType parse_matrix_type();
    ShaderType parse_struct_type();
    ShaderType parse_texture_type();
    ShaderType parse_sampler_type();

    /* Declarations */
    void parse_variable_declaration();
    void parse_uniform_buffer();
    void parse_struct();
    void parse_block(const std::string& keyword, StorageClass storage);

    /* Statements */
    void parse_statement(std::vector<Stmt>& stmts);
    void parse_block_stmt(std::vector<Stmt>& stmts);
    void parse_declare_stmt(std::vector<Stmt>& stmts);
    void parse_assign_stmt(std::vector<Stmt>& stmts);
    void parse_if_stmt(std::vector<Stmt>& stmts);
    void parse_for_stmt(std::vector<Stmt>& stmts);
    void parse_while_stmt(std::vector<Stmt>& stmts);
    void parse_return_stmt(std::vector<Stmt>& stmts);
    void parse_break_stmt(std::vector<Stmt>& stmts);
    void parse_continue_stmt(std::vector<Stmt>& stmts);
    void parse_discard_stmt(std::vector<Stmt>& stmts);
    void parse_switch_stmt(std::vector<Stmt>& stmts);

    /* Expressions */
    uint32_t parse_expression(int precedence = 0);
    uint32_t parse_primary();
    uint32_t parse_identifier_expr();
    uint32_t parse_literal_expr();
    uint32_t parse_construct_expr();
    uint32_t parse_unary_expr();
    uint32_t parse_binary_expr(int min_precedence);
    uint32_t parse_ternary_expr();
    uint32_t parse_call_expr(uint32_t callee);
    uint32_t parse_member_expr(uint32_t base);
    uint32_t parse_index_expr(uint32_t base);
    uint32_t parse_swizzle_expr(uint32_t base);
    uint32_t parse_assign_expr(uint32_t lhs);
    uint32_t parse_postfix_expr(uint32_t base);

    /* Helpers */
    int get_precedence(const std::string& op) const;
    bool is_assign_op(const std::string& op) const;
    BinaryOp op_to_binary(const std::string& op) const;
    UnaryOp op_to_unary(const std::string& op) const;
    void add_diagnostic(KxShaderSeverity severity, const std::string& message);
    void error(const std::string& message);
    void warning(const std::string& message);

    /* Static helpers */
    static const char* token_type_name(TokenType t);
    bool is_type_name(const std::string& name) const;
    uint32_t make_expr(ExprKind kind, const ShaderType& type, float fconst = 0.0f);

    DiagnosticList& diagnostics_;
    const ParserConfig& config_;
    std::vector<Token> tokens_;
    std::size_t position_ = 0;
    ShaderProgram program_;
    std::vector<std::string> struct_names_;
    uint32_t current_function_ = UINT32_MAX;
};

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERPARSER_H */