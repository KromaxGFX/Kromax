#ifndef KROMAX_SRC_SHADER_PARSERDECLARATIONS_H
#define KROMAX_SRC_SHADER_PARSERDECLARATIONS_H

/*
 * ParserDeclarations.h
 *
 * Declaration parsing: program, structs, uniform buffers, push constants,
 * input/output blocks, variable declarations, and functions.
 */

#include "ParserStatements.h"

#include <string>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * ParserDeclarations - declaration parsing
 * --------------------------------------------------------------------- */
class ParserDeclarations : public ParserStatements {
public:
    using ParserStatements::ParserStatements;

    void parse_program(const std::vector<Token>& tokens, uint32_t source_file);
    ShaderProgram parse(const std::vector<Token>& tokens, uint32_t source_file);

    void parse_declaration();
    void parse_struct();
    void parse_uniform_buffer();
    void parse_block(const std::string& keyword, StorageClass storage);
    void parse_variable_declaration();
    void parse_function();
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERDECLARATIONS_H