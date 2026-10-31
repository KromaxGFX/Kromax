/*
 * ShaderLexer.cpp
 *
 * Tokenizer shared by all shader front-ends.
 */

#include "ShaderLexer.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

namespace kx {
namespace shader {

namespace {

bool is_identifier_start(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool is_identifier_part(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

/* Longest-match punctuation table. */
const char* const kPunctuators[] = {
    "<<=", ">>=", "...",
    "->", "++", "--", "&&", "||", "==", "!=", "<=", ">=",
    "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "::",
    "+", "-", "*", "/", "%", "=", "<", ">", "!", "~", "&", "|", "^",
    "?", ":", ";", ",", ".", "(", ")", "[", "]", "{", "}", "#",
};

} /* namespace */

std::vector<Token> Lexer::tokenize(const std::string& text, uint32_t file_id) {
    std::vector<Token> tokens;
    const std::size_t size = text.size();
    std::size_t i = 0;
    uint32_t line = 1;
    uint32_t column = 1;
    bool line_start = true;
    bool pending_hash = false;

    auto advance = [&](std::size_t count) {
        for (std::size_t k = 0; k < count && i < size; ++k) {
            if (text[i] == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
            ++i;
        }
    };

    while (i < size) {
        const char c = text[i];

        /* Whitespace */
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance(1);
            if (c == '\n') {
                line_start = true;
                pending_hash = false;
            }
            continue;
        }

        /* Comments */
        if (c == '/' && i + 1 < size && text[i + 1] == '/') {
            while (i < size && text[i] != '\n') advance(1);
            continue;
        }
        if (c == '/' && i + 1 < size && text[i + 1] == '*') {
            advance(2);
            while (i + 1 < size && !(text[i] == '*' && text[i + 1] == '/')) {
                advance(1);
            }
            if (i + 1 < size) advance(2);
            continue;
        }

        Token token;
        token.file = file_id;
        token.line = line;
        token.column = column;
        token.offset = static_cast<uint32_t>(i);
        token.line_start = line_start;
        token.hash = pending_hash;

        /* Identifiers */
        if (is_identifier_start(c)) {
            const std::size_t start = i;
            while (i < size && is_identifier_part(text[i])) advance(1);
            token.type = TokenType::Identifier;
            token.text.assign(text, start, i - start);
            tokens.push_back(std::move(token));
            line_start = false;
            pending_hash = false;
            continue;
        }

        /* Numbers */
        if (std::isdigit(static_cast<unsigned char>(c)) != 0 ||
            (c == '.' && i + 1 < size &&
             std::isdigit(static_cast<unsigned char>(text[i + 1])) != 0)) {
            const std::size_t start = i;
            bool is_float = false;

            if (c == '0' && i + 1 < size && (text[i + 1] == 'x' || text[i + 1] == 'X')) {
                advance(2);
                while (i < size && std::isxdigit(static_cast<unsigned char>(text[i])) != 0) {
                    advance(1);
                }
                token.number = static_cast<double>(
                    std::strtoull(text.substr(start + 2, i - start - 2).c_str(), nullptr, 16));
            } else {
                while (i < size && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
                    advance(1);
                }
                if (i < size && text[i] == '.') {
                    is_float = true;
                    advance(1);
                    while (i < size && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
                        advance(1);
                    }
                }
                if (i < size && (text[i] == 'e' || text[i] == 'E')) {
                    is_float = true;
                    advance(1);
                    if (i < size && (text[i] == '+' || text[i] == '-')) advance(1);
                    while (i < size && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
                        advance(1);
                    }
                }
                const std::string digits(text.substr(start, i - start));
                token.number = is_float ? std::strtod(digits.c_str(), nullptr)
                                        : static_cast<double>(
                                              std::strtoll(digits.c_str(), nullptr, 10));
            }

            /* Optional suffix (u, U, f, F, h, H, l, L) */
            if (i < size && is_identifier_part(text[i])) {
                const char suffix = text[i];
                if (suffix == 'f' || suffix == 'F' || suffix == 'h' || suffix == 'H') {
                    is_float = true;
                }
                advance(1);
            }

            token.type = TokenType::Number;
            token.is_integer = !is_float;
            token.text.assign(text, start, i - start);
            tokens.push_back(std::move(token));
            line_start = false;
            pending_hash = false;
            continue;
        }

        /* Strings */
        if (c == '"' || c == '\'') {
            const char quote = c;
            advance(1);
            std::string value;
            while (i < size && text[i] != quote) {
                if (text[i] == '\\' && i + 1 < size) {
                    advance(1);
                    char escaped = text[i];
                    switch (escaped) {
                        case 'n': value.push_back('\n'); break;
                        case 't': value.push_back('\t'); break;
                        case 'r': value.push_back('\r'); break;
                        case '\\': value.push_back('\\'); break;
                        case '"': value.push_back('"'); break;
                        case '\'': value.push_back('\''); break;
                        default: value.push_back(escaped); break;
                    }
                    advance(1);
                    continue;
                }
                value.push_back(text[i]);
                advance(1);
            }
            if (i < size) advance(1); /* closing quote */
            token.type = TokenType::String;
            token.text = std::move(value);
            tokens.push_back(std::move(token));
            line_start = false;
            pending_hash = false;
            continue;
        }

        /* Punctuators */
        bool matched = false;
        const char* matched_candidate = nullptr;
        for (const char* candidate : kPunctuators) {
            const std::size_t length = std::strlen(candidate);
            if (i + length <= size && text.compare(i, length, candidate) == 0) {
                token.type = TokenType::Punct;
                token.text = candidate;
                advance(length);
                tokens.push_back(std::move(token));
                matched = true;
                matched_candidate = candidate;
                break;
            }
        }
        if (matched) {
            line_start = false;
            pending_hash = (matched_candidate != nullptr && std::strcmp(matched_candidate, "#") == 0);
            continue;
        }

        /* Unknown character: emit it as a punctuation token so the parser can
         * report a meaningful error instead of silently dropping input. */
        token.type = TokenType::Punct;
        token.text.assign(1, c);
        advance(1);
        tokens.push_back(std::move(token));
        line_start = false;
        pending_hash = false;
    }

    Token end;
    end.type = TokenType::End;
    end.file = file_id;
    end.line = line;
    end.column = column;
    end.offset = static_cast<uint32_t>(i);
    tokens.push_back(std::move(end));

    return tokens;
}

} /* namespace shader */
} /* namespace kx */
