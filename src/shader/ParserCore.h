#ifndef KROMAX_SRC_SHADER_PARSERCORE_H
#define KROMAX_SRC_SHADER_PARSERCORE_H

/*
 * ParserCore.h
 *
 * Core parser infrastructure: token handling, error reporting,
 * expression/statement creation utilities, and parser state.
 */

#include "ShaderTypes.h"
#include "ShaderIR.h"
#include "ShaderLexer.h"
#include "ShaderDiagnostics.h"
#include "ParserConfig.h"
#include "ParserBuiltins.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * ParserCore - core parsing utilities and state
 * --------------------------------------------------------------------- */
class ParserCore {
protected:
    std::vector<Token> tokens_;
    std::size_t position_ = 0;
    ShaderProgram program_;
    ParserConfig config_;
    std::vector<std::string> struct_names_;
    uint32_t current_function_ = UINT32_MAX;
    DiagnosticList& diagnostics_;

public:
    explicit ParserCore(DiagnosticList& diagnostics, const ParserConfig& config = {})
        : config_(config), diagnostics_(diagnostics) {}
    virtual ~ParserCore() = default;

    const std::vector<Token>& tokens() const { return tokens_; }
    std::size_t position() const { return position_; }
    const ShaderProgram& program() const { return program_; }
    ShaderProgram& program() { return program_; }
    const ParserConfig& config() const { return config_; }
    ParserConfig& config() { return config_; }
    const std::vector<std::string>& struct_names() const { return struct_names_; }
    uint32_t current_function() const { return current_function_; }
    DiagnosticList& diagnostics() { return diagnostics_; }

    void set_tokens(const std::vector<Token>& tokens) { tokens_ = tokens; }
    void set_position(std::size_t pos) { position_ = pos; }
    void set_current_function(uint32_t func) { current_function_ = func; }

    /* Token lookahead and consumption */
    const Token& peek() const;
    const Token& peek(int offset) const;
    const Token& consume();
    bool match(TokenType type, const std::string& text = "");
    bool match(TokenType type, const std::string& text1, const std::string& text2);
    bool check(TokenType type, const std::string& text = "") const;
    void expect(TokenType type, const std::string& text = "");
    void expect_identifier(std::string& out_name);

    static const char* token_type_name(TokenType t);

    /* Expression creation */
    uint32_t make_expr(ExprKind kind, const ShaderType& type, float fconst = 0.0f);
    ShaderType make_bool() const { return ShaderType{TypeClass::Bool}; }
    ShaderType make_int() const { return ShaderType{TypeClass::Int}; }
    ShaderType make_uint() const { return ShaderType{TypeClass::UInt}; }
    ShaderType make_float() const { return ShaderType{TypeClass::Float}; }
    ShaderType make_vector(uint32_t components, TypeClass cls = TypeClass::Float) const {
        return ShaderType{cls, components, 1, 0, 4u * components};
    }
    ShaderType make_matrix(uint32_t cols, uint32_t rows, TypeClass cls = TypeClass::Float) const {
        return ShaderType{cls, cols, rows, 0, 4u * cols * rows};
    }

    /* Type promotion and binary operator utilities */
    ShaderType promote(const ShaderType& a, const ShaderType& b) const;

    /* Operator precedence and conversion */
    int get_precedence(const std::string& op) const;
    bool is_assign_op(const std::string& op) const;
    BinaryOp op_to_binary(const std::string& op) const;
    UnaryOp op_to_unary(const std::string& op) const;

    /* Error reporting */
    void add_diagnostic(KxShaderSeverity severity, const std::string& message);
    void error(const std::string& message);
    void warning(const std::string& message);

    /* Builtin check */
    bool is_builtin(const std::string& name) const { return is_builtin_function(name); }
};

/* -----------------------------------------------------------------------
 * Inline implementations
 * --------------------------------------------------------------------- */
inline const Token& ParserCore::peek() const {
    if (position_ < tokens_.size()) return tokens_[position_];
    static const Token end_token;
    return end_token;
}

inline const Token& ParserCore::peek(int offset) const {
    std::size_t idx = position_ + offset;
    if (idx < tokens_.size()) return tokens_[idx];
    static const Token end_token;
    return end_token;
}

inline const Token& ParserCore::consume() {
    if (position_ < tokens_.size()) return tokens_[position_++];
    static const Token end_token;
    return end_token;
}

inline bool ParserCore::match(TokenType type, const std::string& text) {
    if (position_ < tokens_.size() && tokens_[position_].type == type &&
        (text.empty() || tokens_[position_].text == text)) {
        ++position_;
        return true;
    }
    return false;
}

inline bool ParserCore::match(TokenType type, const std::string& text1, const std::string& text2) {
    if (position_ < tokens_.size() && tokens_[position_].type == type &&
        (tokens_[position_].text == text1 || tokens_[position_].text == text2)) {
        ++position_;
        return true;
    }
    return false;
}

inline bool ParserCore::check(TokenType type, const std::string& text) const {
    if (position_ < tokens_.size() && tokens_[position_].type == type) {
        return text.empty() || tokens_[position_].text == text;
    }
    return false;
}

inline void ParserCore::expect(TokenType type, const std::string& text) {
    if (!match(type, text)) {
        const Token& tok = peek();
        error("expected " + (text.empty() ? token_type_name(type) : text) +
              ", got " + tok.text);
    }
}

inline void ParserCore::expect_identifier(std::string& out_name) {
    if (match(TokenType::Identifier)) {
        out_name = tokens_[position_ - 1].text;
    } else {
        const Token& tok = peek();
        error("expected identifier, got " + tok.text);
    }
}

inline const char* ParserCore::token_type_name(TokenType t) {
    switch (t) {
        case TokenType::End: return "end of file";
        case TokenType::Identifier: return "identifier";
        case TokenType::Number: return "number";
        case TokenType::String: return "string";
        case TokenType::Punct: return "punctuator";
    }
    return "unknown";
}

inline uint32_t ParserCore::make_expr(ExprKind kind, const ShaderType& type, float fconst) {
    Expr expr;
    expr.kind = kind;
    expr.type = type;
    expr.fconst = fconst;
    expr.line = peek().line;
    program_.exprs.push_back(expr);
    return static_cast<uint32_t>(program_.exprs.size() - 1);
}

inline ShaderType ParserCore::promote(const ShaderType& a, const ShaderType& b) const {
    if (a.cls != b.cls) return make_float();
    if (a.cols != b.cols || a.rows != b.rows) return make_float();
    return a;
}

inline int ParserCore::get_precedence(const std::string& op) const {
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

inline bool ParserCore::is_assign_op(const std::string& op) const {
    return op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" ||
           op == "%=" || op == "&=" || op == "|=" || op == "^=" || op == "<<=" || op == ">>=";
}

inline BinaryOp ParserCore::op_to_binary(const std::string& op) const {
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

inline UnaryOp ParserCore::op_to_unary(const std::string& op) const {
    if (op == "-") return UnaryOp::Negate;
    if (op == "!") return UnaryOp::LogicalNot;
    if (op == "~") return UnaryOp::BitNot;
    if (op == "+") return UnaryOp::Plus;
    if (op == "++") return UnaryOp::PreIncrement;
    if (op == "--") return UnaryOp::PreDecrement;
    return UnaryOp::Negate;
}

inline void ParserCore::add_diagnostic(KxShaderSeverity severity, const std::string& message) {
    const Token& tok = peek();
    diagnostics_.report(severity, tok.file, tok.line, tok.column, tok.offset, message);
}

inline void ParserCore::error(const std::string& message) {
    add_diagnostic(KX_SHADER_SEVERITY_ERROR, message);
}

inline void ParserCore::warning(const std::string& message) {
    add_diagnostic(KX_SHADER_SEVERITY_WARNING, message);
}

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERCORE_H