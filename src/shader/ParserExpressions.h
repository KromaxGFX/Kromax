#ifndef KROMAX_SRC_SHADER_PARSEREXPRESSIONS_H
#define KROMAX_SRC_SHADER_PARSEREXPRESSIONS_H

/*
 * ParserExpressions.h
 *
 * Expression parsing: primary, literal, identifier, postfix, constructor,
 * unary, binary, ternary, and function call expressions.
 */

#include "ParserTypes.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * ParserExpressions - expression parsing
 * --------------------------------------------------------------------- */
class ParserExpressions : public ParserTypes {
public:
    using ParserTypes::ParserTypes;

    uint32_t parse_expression(int precedence = 0);

protected:
    uint32_t parse_primary();
    uint32_t parse_literal_expr();
    uint32_t parse_identifier_expr();
    uint32_t parse_postfix_expr(uint32_t base);
    uint32_t parse_construct_expr();
    uint32_t parse_unary_expr();
    uint32_t parse_binary_expr(int min_precedence);
    uint32_t parse_call_expr(uint32_t callee);
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSEREXPRESSIONS_H