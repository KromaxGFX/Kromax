#ifndef KROMAX_SRC_SHADER_SHADERLEXER_H
#define KROMAX_SRC_SHADER_SHADERLEXER_H

/*
 * ShaderLexer.h
 *
 * Shared tokenizer and preprocessor used by every shader front-end. The
 * tokenizer produces a flat token stream; the preprocessor consumes that
 * stream, resolves directives and expands macros, and produces the token
 * stream that the parser consumes.
 */

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "ShaderDiagnostics.h"

namespace kx {
namespace shader {

enum class TokenType : uint8_t {
    End,
    Identifier,
    Number,
    String,
    Punct,
};

struct Token {
    TokenType type = TokenType::End;
    std::string text;
    double number = 0.0;
    bool is_integer = false;
    uint32_t line = 1;
    uint32_t column = 1;
    uint32_t file = 0;
    uint32_t offset = 0;
    bool line_start = false;  /* first token on its source line */
    bool hash = false;        /* directly preceded by '#'            */
};

/* -----------------------------------------------------------------------
 * Source manager: keeps the text of every file involved in a compilation so
 * that diagnostics can name the file a problem came from.
 * --------------------------------------------------------------------- */
class SourceManager {
public:
    uint32_t add(std::string name, std::string text) {
        files_.push_back(File{std::move(name), std::move(text)});
        return static_cast<uint32_t>(files_.size() - 1u);
    }

    const std::string& name(uint32_t id) const {
        static const std::string kUnknown = "<unknown>";
        return id < files_.size() ? files_[id].name : kUnknown;
    }

    const std::string& text(uint32_t id) const {
        static const std::string kEmpty;
        return id < files_.size() ? files_[id].text : kEmpty;
    }

    uint32_t count() const noexcept { return static_cast<uint32_t>(files_.size()); }

private:
    struct File {
        std::string name;
        std::string text;
    };
    std::vector<File> files_;
};

/* -----------------------------------------------------------------------
 * Lexer
 * --------------------------------------------------------------------- */
class Lexer {
public:
    /* Tokenize `text`. The resulting tokens are marked with `line_start` and
     * `hash` so the preprocessor can recognise directives. */
    static std::vector<Token> tokenize(const std::string& text, uint32_t file_id);
};

/* -----------------------------------------------------------------------
 * Preprocessor
 * --------------------------------------------------------------------- */
class Preprocessor {
public:
    using IncludeResolver =
        std::function<bool(const std::string& name, std::string& out_text)>;

    Preprocessor(DiagnosticList& diagnostics, SourceManager& sources)
        : diagnostics_(diagnostics), sources_(sources) {}

    void set_include_resolver(IncludeResolver resolver) {
        include_resolver_ = std::move(resolver);
    }

    /* Predefined macros, e.g. "__VERSION__" -> "1". */
    void define(const std::string& name, const std::string& value);

    /* Process `input` and append the resulting tokens to `output`. */
    void process(const std::vector<Token>& input, std::vector<Token>& output);

private:
    struct Macro {
        std::string name;
        bool function_like = false;
        std::vector<std::string> parameters;
        std::vector<Token> body;
    };

    struct Conditional {
        bool parent_active = true;
        bool branch_taken = false;
        bool active = true;
        uint32_t line = 0;
    };

    /* Directive handling */
    void handle_directive(const std::vector<Token>& input, std::size_t& index,
                          std::vector<Token>& output);
    void handle_define(const std::vector<Token>& input, std::size_t& index);
    void handle_undef(const std::vector<Token>& input, std::size_t& index);
    void handle_if(const std::vector<Token>& input, std::size_t& index, bool negate);
    void handle_else(const std::vector<Token>& input, std::size_t& index);
    void handle_elif(const std::vector<Token>& input, std::size_t& index);
    void handle_endif(const std::vector<Token>& input, std::size_t& index);
    void handle_include(const std::vector<Token>& input, std::size_t& index,
                        std::vector<Token>& output);

    /* Conditional expressions */
    bool evaluate_condition(const std::vector<Token>& input, std::size_t& index);
    long long evaluate_expression(const std::vector<Token>& tokens,
                                  std::size_t& position, int min_precedence,
                                  bool& ok);
    long long evaluate_unary(const std::vector<Token>& tokens,
                             std::size_t& position, bool& ok);
    long long evaluate_primary(const std::vector<Token>& tokens,
                               std::size_t& position, bool& ok);

    /* Macro expansion */
    void expand_tokens(const std::vector<Token>& tokens, std::vector<Token>& output);
    bool try_expand(const std::vector<Token>& tokens, std::size_t index,
                    std::vector<Token>& expansion);
    bool collect_arguments(const std::vector<Token>& tokens, std::size_t& index,
                           std::vector<std::vector<Token>>& arguments);
    void substitute_macro_body(const Macro& macro,
                               const std::vector<std::vector<Token>>& arguments,
                               std::vector<Token>& expansion);

    bool is_defined(const std::string& name) const {
        return macros_.find(name) != macros_.end();
    }

    DiagnosticList& diagnostics_;
    SourceManager& sources_;
    IncludeResolver include_resolver_;
    std::map<std::string, Macro> macros_;
    std::vector<std::string> expansion_stack_;
    std::vector<Conditional> conditionals_;
    bool failed_ = false;
};

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERLEXER_H */
