#ifndef KROMAX_SRC_API_SHADER_SHADERLANGUAGE_H
#define KROMAX_SRC_API_SHADER_SHADERLANGUAGE_H

/*
 * ShaderLanguage.h
 * Shader language detection and name utilities.
 */

#include "kromax.h"
#include "kx_core.h"

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

// Get human-readable name for shader language
const char* KX_CALL kxGetShaderLanguageName(KxShaderLanguage language);

// Auto-detect shader language from source code
KxShaderLanguage KX_CALL kxDetectShaderLanguage(
    const char* pSource,
    size_t sourceSize);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // KROMAX_SRC_API_SHADER_SHADERLANGUAGE_H