#include "ParserStatements.h"

#include "ParserExpressions.h"
#include "ShaderIR.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Statement parsing
 * --------------------------------------------------------------------- */
void ParserStatements::parse_statement(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_block_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Punct, "{");
    while (!check(TokenType::Punct, "}") && !check(TokenType::End)) {
        parse_statement(stmts);
    }
    expect(TokenType::Punct, "}");
}

void ParserStatements::parse_declare_stmt(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_assign_stmt(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_if_stmt(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_for_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "for");
    expect(TokenType::Punct, "(");

    Stmt stmt;
    stmt.kind = StmtKind::For;
    stmt.line = peek().line;

    /* Init */
    if (!check(TokenType::Punct, ";")) {
        if (check(TokenType::Identifier) ||
            (check(TokenType::Identifier) && position_ + 1 < tokens_.size() && tokens_[position_ + 1].type == TokenType::Identifier)) {
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

void ParserStatements::parse_while_stmt(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_return_stmt(std::vector<Stmt>& stmts) {
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

void ParserStatements::parse_break_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "break");

    Stmt stmt;
    stmt.kind = StmtKind::Break;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void ParserStatements::parse_continue_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "continue");

    Stmt stmt;
    stmt.kind = StmtKind::Continue;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void ParserStatements::parse_discard_stmt(std::vector<Stmt>& stmts) {
    expect(TokenType::Identifier, "discard");

    Stmt stmt;
    stmt.kind = StmtKind::Discard;
    stmt.line = peek().line;

    stmts.push_back(std::move(stmt));
    expect(TokenType::Punct, ";");
}

void ParserStatements::parse_switch_stmt(std::vector<Stmt>& stmts) {
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

} // namespace shader
} // namespace kx