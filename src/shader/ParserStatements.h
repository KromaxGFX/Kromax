#ifndef KROMAX_SRC_SHADER_PARSERSTATEMENTS_H
#define KROMAX_SRC_SHADER_PARSERSTATEMENTS_H

/*
 * ParserStatements.h
 *
 * Statement parsing: if, for, while, return, break, continue, discard,
 * switch, block, declaration, assignment, expression statements.
 */

#include "ParserExpressions.h"

#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * ParserStatements - statement parsing
 * --------------------------------------------------------------------- */
class ParserStatements : public ParserExpressions {
public:
    using ParserExpressions::ParserExpressions;

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
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERSTATEMENTS_H