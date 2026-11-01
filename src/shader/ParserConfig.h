#ifndef KROMAX_SRC_SHADER_PARSERCONFIG_H
#define KROMAX_SRC_SHADER_PARSERCONFIG_H

/*
 * ParserConfig.h
 *
 * Parser configuration structure - shared between parser modules.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {
namespace shader {

struct ParserConfig {
    KxShaderLanguage language = KX_SHADER_LANGUAGE_GLSL;
    bool allow_struct = true;
    bool allow_uniform_buffer = true;
    bool allow_push_constant = true;
    bool require_version = false;
    bool entry_point_required = false;
};

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERCONFIG_H