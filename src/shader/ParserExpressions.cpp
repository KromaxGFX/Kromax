#include "ParserExpressions.h"

#include <algorithm>
#include "ShaderIR.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Expression parsing
 * --------------------------------------------------------------------- */
uint32_t ParserExpressions::parse_expression(int precedence) {
    return parse_binary_expr(precedence);
}

uint32_t ParserExpressions::parse_primary() {
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

uint32_t ParserExpressions::parse_literal_expr() {
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

uint32_t ParserExpressions::parse_identifier_expr() {
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

uint32_t ParserExpressions::parse_postfix_expr(uint32_t base) {
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

uint32_t ParserExpressions::parse_construct_expr() {
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

uint32_t ParserExpressions::parse_unary_expr() {
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

uint32_t ParserExpressions::parse_binary_expr(int min_precedence) {
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

uint32_t ParserExpressions::parse_call_expr(uint32_t callee) {
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
        /* Find the function name token - it's at position_ - args.size() - 2 (for '(' and name) */
        std::size_t name_pos = position_ - expr.args.size() - 1;
        if (name_pos < tokens_.size()) {
            const std::string& func_name = tokens_[name_pos].text;
            if (is_builtin_function(func_name)) {
                expr.builtin = static_cast<uint32_t>(program_.builtin_names.size());
                program_.builtin_names.push_back(func_name);
                expr.func = UINT32_MAX;
            }
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

} // namespace shader
} // namespace kx