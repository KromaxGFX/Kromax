/*
 * ShaderLexer.cpp - Preprocessor implementation
 *
 * Handles #include, #define, #undef, #if, #ifdef, #ifndef, #elif, #else, #endif,
 * #version, #extension, #line directives and macro expansion.
 */

#include "ShaderLexer.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Preprocessor
 * --------------------------------------------------------------------- */
void Preprocessor::define(const std::string& name, const std::string& value) {
    Macro macro;
    macro.name = name;
    macro.function_like = false;
    macro.body = Lexer::tokenize(value, 0);
    macros_[name] = std::move(macro);
}

void Preprocessor::process(const std::vector<Token>& input, std::vector<Token>& output) {
    std::size_t index = 0;
    while (index < input.size()) {
        const Token& token = input[index];

        if (token.type == TokenType::End) {
            output.push_back(token);
            break;
        }

        /* Check if we're in an active conditional branch */
        bool in_active_branch = conditionals_.empty() || conditionals_.back().active;

        /* Directive detection: token.hash is true for the token immediately after '#' */
        if (token.hash && token.type == TokenType::Identifier) {
            /* Directive - always process directives regardless of conditional state */
            /* Remove the preceding '#' token from output if present */
            if (!output.empty() && output.back().type == TokenType::Punct && output.back().text == "#") {
                output.pop_back();
            }
            handle_directive(input, index, output);
        } else if (in_active_branch) {
            /* Only output tokens when in an active conditional branch */
            /* Expand any macros in the token stream */
            std::vector<Token> expanded;
            std::size_t saved = index;
            expand_tokens({token}, expanded);
            if (expanded.size() > 1 || (expanded.size() == 1 && expanded[0].type != TokenType::End)) {
                for (const auto& t : expanded) {
                    if (t.type != TokenType::End) output.push_back(t);
                }
            } else {
                output.push_back(token);
            }
            index = saved + 1;
        } else {
            /* In inactive conditional branch - skip token */
            ++index;
        }
    }
}

void Preprocessor::handle_directive(const std::vector<Token>& input, std::size_t& index,
                                    std::vector<Token>& output) {
    /* index points to the directive name token (which has hash=true) */
    const Token& token = input[index];
    const std::string& directive = token.text;

    if (directive == "define") {
        handle_define(input, index);
    } else if (directive == "undef") {
        handle_undef(input, index);
    } else if (directive == "if") {
        handle_if(input, index, false);
    } else if (directive == "ifdef") {
        handle_if(input, index, false); /* special case handled inside */
    } else if (directive == "ifndef") {
        handle_if(input, index, true);
    } else if (directive == "elif") {
        handle_elif(input, index);
    } else if (directive == "else") {
        handle_else(input, index);
    } else if (directive == "endif") {
        handle_endif(input, index);
    } else if (directive == "include") {
        handle_include(input, index, output);
    } else if (directive == "version") {
        ++index;
        if (index < input.size() && input[index].type == TokenType::Number) {
            /* Store version - just ignore for now */
        }
    } else if (directive == "extension") {
        ++index;
        /* Skip extension directive */
        while (index < input.size() && input[index].type != TokenType::End &&
               input[index].line == token.line) {
            ++index;
        }
    } else if (directive == "line") {
        ++index;
        /* Skip #line directive */
        while (index < input.size() && input[index].type != TokenType::End &&
               input[index].line == token.line) {
            ++index;
        }
    } else if (directive == "pragma") {
        ++index;
        /* Skip #pragma directive */
        while (index < input.size() && input[index].type != TokenType::End &&
               input[index].line == token.line) {
            ++index;
        }
    } else {
        diagnostics_.warning(token.file, token.line, token.column, token.offset,
                             "unknown preprocessor directive: #" + directive);
        while (index < input.size() && input[index].type != TokenType::End &&
               input[index].line == token.line) {
            ++index;
        }
    }
}

void Preprocessor::handle_define(const std::vector<Token>& input, std::size_t& index) {
    ++index; /* skip 'define' */

    if (index >= input.size() || input[index].type != TokenType::Identifier) {
        diagnostics_.error(input[index].file, input[index].line, input[index].column,
                           input[index].offset, "expected macro name after #define");
        return;
    }

    Token name_token = input[index];
    ++index;

    Macro macro;
    macro.name = name_token.text;

    if (index < input.size() && input[index].type == TokenType::Punct &&
        input[index].text == "(" && !input[index].line_start) {
        macro.function_like = true;
        ++index; /* skip '(' */

        while (index < input.size() && !(input[index].type == TokenType::Punct &&
                                          input[index].text == ")")) {
            if (input[index].type == TokenType::Identifier) {
                macro.parameters.push_back(input[index].text);
                ++index;
                if (index < input.size() && input[index].type == TokenType::Punct &&
                    input[index].text == ",") {
                    ++index;
                }
            } else {
                ++index;
            }
        }
        if (index < input.size() && input[index].type == TokenType::Punct &&
            input[index].text == ")") {
            ++index; /* skip ')' */
        }
    }

    /* Collect body tokens until end of line */
    while (index < input.size() && input[index].type != TokenType::End &&
           input[index].line == name_token.line) {
        macro.body.push_back(input[index]);
        ++index;
    }

    macros_[name_token.text] = std::move(macro);
}

void Preprocessor::handle_undef(const std::vector<Token>& input, std::size_t& index) {
    ++index; /* skip 'undef' */

    if (index < input.size() && input[index].type == TokenType::Identifier) {
        macros_.erase(input[index].text);
        ++index;
    }
}

void Preprocessor::handle_if(const std::vector<Token>& input, std::size_t& index, bool negate) {
    ++index; /* skip if/ifdef/ifndef */

    Conditional cond;
    cond.line = index < input.size() ? input[index].line : 0;
    cond.parent_active = conditionals_.empty() ? true : conditionals_.back().active;

    bool value = false;
    if (negate) {
        /* ifndef: check if defined */
        if (index < input.size() && input[index].type == TokenType::Identifier) {
            value = !is_defined(input[index].text);
            ++index;
        }
    } else if (index < input.size() && input[index].type == TokenType::Identifier &&
               input[index].text == "defined") {
        /* #if defined(NAME) */
        ++index;
        if (index < input.size() && input[index].type == TokenType::Punct &&
            input[index].text == "(") {
            ++index;
            if (index < input.size() && input[index].type == TokenType::Identifier) {
                value = is_defined(input[index].text);
                ++index;
            }
            if (index < input.size() && input[index].type == TokenType::Punct &&
                input[index].text == ")") {
                ++index;
            }
        }
    } else if (input[index].type == TokenType::Identifier &&
               (input[index].text == "true" || input[index].text == "false")) {
        value = (input[index].text == "true");
        ++index;
    } else {
        value = evaluate_condition(input, index) != 0;
    }

    cond.active = cond.parent_active && value;
    cond.branch_taken = cond.active;
    conditionals_.push_back(std::move(cond));
}

void Preprocessor::handle_elif(const std::vector<Token>& input, std::size_t& index) {
    ++index; /* skip 'elif' */

    if (conditionals_.empty()) {
        diagnostics_.error(input[index].file, input[index].line, input[index].column,
                           input[index].offset, "#elif without #if");
        return;
    }

    Conditional& cond = conditionals_.back();
    if (cond.branch_taken) {
        cond.active = false;
    } else {
        bool value = evaluate_condition(input, index);
        cond.active = cond.parent_active && (value != 0);
        cond.branch_taken = cond.branch_taken || cond.active;
    }
}

void Preprocessor::handle_else(const std::vector<Token>& input, std::size_t& index) {
    ++index; /* skip 'else' */

    if (conditionals_.empty()) {
        diagnostics_.error(input[index].file, input[index].line, input[index].column,
                           input[index].offset, "#else without #if");
        return;
    }

    Conditional& cond = conditionals_.back();
    cond.active = cond.parent_active && !cond.branch_taken;
    cond.branch_taken = true;
}

void Preprocessor::handle_endif(const std::vector<Token>& input, std::size_t& index) {
    ++index; /* skip 'endif' */

    if (conditionals_.empty()) {
        diagnostics_.error(input[index].file, input[index].line, input[index].column,
                           input[index].offset, "#endif without #if");
        return;
    }

    conditionals_.pop_back();
}

bool Preprocessor::evaluate_condition(const std::vector<Token>& input, std::size_t& index) {
    bool ok = true;
    long long result = evaluate_expression(input, index, 0, ok);
    return ok && result != 0;
}

long long Preprocessor::evaluate_expression(const std::vector<Token>& tokens,
                                            std::size_t& position, int min_precedence,
                                            bool& ok) {
    long long left = evaluate_unary(tokens, position, ok);
    if (!ok) return 0;

    while (position < tokens.size()) {
        const Token& tok = tokens[position];
        if (tok.type != TokenType::Punct) break;

        int prec = 0;
        const char* op = tok.text.c_str();

        if (strcmp(op, "||") == 0) prec = 1;
        else if (strcmp(op, "&&") == 0) prec = 2;
        else if (strcmp(op, "|") == 0) prec = 3;
        else if (strcmp(op, "^") == 0) prec = 4;
        else if (strcmp(op, "&") == 0) prec = 5;
        else if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) prec = 6;
        else if (strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
                 strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0) prec = 7;
        else if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0) prec = 8;
        else if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) prec = 9;
        else if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) prec = 10;
        else break;

        if (prec < min_precedence) break;

        ++position;
        long long right = evaluate_expression(tokens, position, prec + 1, ok);
        if (!ok) return 0;

        if (strcmp(op, "||") == 0) left = left || right;
        else if (strcmp(op, "&&") == 0) left = left && right;
        else if (strcmp(op, "|") == 0) left = left | right;
        else if (strcmp(op, "^") == 0) left = left ^ right;
        else if (strcmp(op, "&") == 0) left = left & right;
        else if (strcmp(op, "==") == 0) left = left == right;
        else if (strcmp(op, "!=") == 0) left = left != right;
        else if (strcmp(op, "<") == 0) left = left < right;
        else if (strcmp(op, ">") == 0) left = left > right;
        else if (strcmp(op, "<=") == 0) left = left <= right;
        else if (strcmp(op, ">=") == 0) left = left >= right;
        else if (strcmp(op, "<<") == 0) left = left << right;
        else if (strcmp(op, ">>") == 0) left = left >> right;
        else if (strcmp(op, "+") == 0) left = left + right;
        else if (strcmp(op, "-") == 0) left = left - right;
        else if (strcmp(op, "*") == 0) left = left * right;
        else if (strcmp(op, "/") == 0) left = right ? left / right : 0;
        else if (strcmp(op, "%") == 0) left = right ? left % right : 0;
    }
    return left;
}

long long Preprocessor::evaluate_unary(const std::vector<Token>& tokens,
                                       std::size_t& position, bool& ok) {
    if (position >= tokens.size()) {
        ok = false;
        return 0;
    }
    const Token& tok = tokens[position];
    if (tok.type == TokenType::Punct) {
        if (tok.text == "+") {
            ++position;
            return evaluate_unary(tokens, position, ok);
        }
        if (tok.text == "-") {
            ++position;
            return -evaluate_unary(tokens, position, ok);
        }
        if (tok.text == "!") {
            ++position;
            return !evaluate_unary(tokens, position, ok);
        }
        if (tok.text == "~") {
            ++position;
            return ~evaluate_unary(tokens, position, ok);
        }
    }
    return evaluate_primary(tokens, position, ok);
}

long long Preprocessor::evaluate_primary(const std::vector<Token>& tokens,
                                         std::size_t& position, bool& ok) {
    if (position >= tokens.size()) {
        ok = false;
        return 0;
    }
    const Token& tok = tokens[position];

    if (tok.type == TokenType::Number) {
        long long val = tok.is_integer ? static_cast<long long>(tok.number)
                                        : static_cast<long long>(tok.number);
        ++position;
        return val;
    }
    if (tok.type == TokenType::Identifier) {
        if (tok.text == "defined") {
            ++position;
            if (position < tokens.size() && tokens[position].type == TokenType::Punct &&
                tokens[position].text == "(") {
                ++position;
                if (position < tokens.size() && tokens[position].type == TokenType::Identifier) {
                    ++position;
                }
                if (position < tokens.size() && tokens[position].type == TokenType::Punct &&
                    tokens[position].text == ")") {
                    ++position;
                }
                return is_defined(tokens[position - 1].text) ? 1 : 0;
            }
        }
        if (tok.text == "true") { ++position; return 1; }
        if (tok.text == "false") { ++position; return 0; }
        if (is_defined(tok.text)) {
            /* Defined macro with value - for now just return 1 */
            ++position;
            return 1;
        }
    }
    if (tok.type == TokenType::Punct && tok.text == "(") {
        ++position;
        bool inner_ok = true;
        long long val = evaluate_expression(tokens, position, 0, inner_ok);
        if (position < tokens.size() && tokens[position].type == TokenType::Punct &&
            tokens[position].text == ")") {
            ++position;
        }
        ok = inner_ok;
        return val;
    }
    ok = false;
    return 0;
}

void Preprocessor::expand_tokens(const std::vector<Token>& tokens, std::vector<Token>& output) {
    std::size_t index = 0;
    while (index < tokens.size()) {
        const Token& tok = tokens[index];
        if (tok.type == TokenType::Identifier && is_defined(tok.text)) {
            auto it = macros_.find(tok.text);
            if (it != macros_.end()) {
                const Macro& macro = it->second;
                if (macro.function_like) {
                    std::vector<std::vector<Token>> arguments;
                    if (!collect_arguments(tokens, index, arguments)) {
                        /* Not a macro call or failed - output original */
                        output.push_back(tok);
                        ++index;
                        continue;
                    }
                    std::vector<Token> expansion;
                    substitute_macro_body(macro, arguments, expansion);
                    for (const auto& t : expansion) {
                        if (t.type != TokenType::End) output.push_back(t);
                    }
                    continue;
                } else {
                    /* Object-like macro */
                    for (const auto& t : macro.body) {
                        if (t.type != TokenType::End) output.push_back(t);
                    }
                    ++index;
                    continue;
                }
            }
        }
        output.push_back(tok);
        ++index;
    }
}

bool Preprocessor::collect_arguments(const std::vector<Token>& tokens, std::size_t& index,
                                     std::vector<std::vector<Token>>& arguments) {
    std::size_t start = index;
    if (index >= tokens.size() || tokens[index].type != TokenType::Punct ||
        tokens[index].text != "(") {
        index = start;
        return false;
    }
    ++index; /* skip '(' */

    std::vector<Token> current;
    int paren_depth = 0;

    while (index < tokens.size()) {
        const Token& tok = tokens[index];
        if (tok.type == TokenType::Punct) {
            if (tok.text == "(") {
                ++paren_depth;
                current.push_back(tok);
            } else if (tok.text == ")") {
                if (paren_depth == 0) {
                    ++index; /* skip ')' */
                    arguments.push_back(std::move(current));
                    return true;
                } else {
                    --paren_depth;
                    current.push_back(tok);
                }
            } else if (tok.text == "," && paren_depth == 0) {
                arguments.push_back(std::move(current));
                current.clear();
            } else {
                current.push_back(tok);
            }
        } else {
            current.push_back(tok);
        }
        ++index;
    }
    index = start;
    return false;
}

void Preprocessor::substitute_macro_body(const Macro& macro,
                                         const std::vector<std::vector<Token>>& arguments,
                                         std::vector<Token>& expansion) {
    if (!macro.function_like) {
        expansion = macro.body;
        return;
    }

    std::vector<Token> body = macro.body;
    std::vector<Token> result;

    for (std::size_t i = 0; i < body.size(); ++i) {
        const Token& tok = body[i];
        if (tok.type == TokenType::Identifier) {
            auto it = std::find(macro.parameters.begin(), macro.parameters.end(), tok.text);
            if (it != macro.parameters.end()) {
                std::size_t param_index = std::distance(macro.parameters.begin(), it);
                if (param_index < arguments.size()) {
                    for (const auto& arg_tok : arguments[param_index]) {
                        result.push_back(arg_tok);
                    }
                    continue;
                }
            }
        }
        result.push_back(tok);
    }

    expansion = std::move(result);
}

void Preprocessor::handle_include(const std::vector<Token>& input, std::size_t& index,
                                  std::vector<Token>& output) {
    ++index; /* skip 'include' */

    if (index >= input.size()) return;

    std::string include_name;
    if (input[index].type == TokenType::String) {
        include_name = input[index].text;
        ++index;
    } else if (input[index].type == TokenType::Punct && input[index].text == "<") {
        /* System include - not supported */
        while (index < input.size() && input[index].type != TokenType::End &&
               input[index].line == input[index - 1].line) {
            ++index;
        }
        return;
    }

    if (include_name.empty()) return;

    if (include_resolver_) {
        std::string included_text;
        if (include_resolver_(include_name, included_text)) {
            uint32_t file_id = sources_.add(include_name, included_text);
            std::vector<Token> included_tokens = Lexer::tokenize(included_text, file_id);
            std::vector<Token> processed;
            process(included_tokens, processed);
            for (const auto& t : processed) {
                if (t.type != TokenType::End) output.push_back(t);
            }
            return;
        }
    }

    diagnostics_.warning(input[index].file, input[index].line, input[index].column,
                         input[index].offset,
                         "include not found: " + include_name);
}

} /* namespace shader */
} /* namespace kx */