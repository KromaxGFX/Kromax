#ifndef KROMAX_SRC_SHADER_PARSERTYPES_H
#define KROMAX_SRC_SHADER_PARSERTYPES_H

/*
 * ParserTypes.h
 *
 * Type parsing: base types, vectors, matrices, textures, samplers,
 * and user-defined structs.
 */

#include "ParserCore.h"

#include <algorithm>
#include <string>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * ParserTypes - type parsing utilities
 * --------------------------------------------------------------------- */
class ParserTypes : public ParserCore {
public:
    using ParserCore::ParserCore;

    ShaderType parse_type();
    bool is_type_name(const std::string& name) const;

private:
    ShaderType parse_base_type();
    ShaderType parse_vector_type();
    ShaderType parse_matrix_type();
    ShaderType parse_struct_type();
    ShaderType parse_texture_type();
    ShaderType parse_sampler_type();
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERTYPES_H