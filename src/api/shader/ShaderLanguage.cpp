/*
 * ShaderLanguage.cpp
 * Shader language detection and name utilities implementation.
 */

#include "ShaderLanguage.h"

#include <cstring>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

const char* KX_CALL kxGetShaderLanguageName(KxShaderLanguage language) {
    switch (language) {
        case KX_SHADER_LANGUAGE_GLSL: return "GLSL";
        case KX_SHADER_LANGUAGE_HLSL: return "HLSL";
        case KX_SHADER_LANGUAGE_WGSL: return "WGSL";
        case KX_SHADER_LANGUAGE_KXSL: return "KXSL";
        default: return "Unknown";
    }
}

KxShaderLanguage KX_CALL kxDetectShaderLanguage(
    const char* pSource,
    size_t sourceSize) {
    if (!pSource || sourceSize == 0) return KX_SHADER_LANGUAGE_UNDEFINED;

    std::string source(pSource, sourceSize);

    if (source.find("#version") != std::string::npos ||
        source.find("layout(") != std::string::npos ||
        source.find("in vec3") != std::string::npos ||
        source.find("out vec3") != std::string::npos) {
        return KX_SHADER_LANGUAGE_GLSL;
    }
    if (source.find("cbuffer") != std::string::npos ||
        source.find("StructuredBuffer") != std::string::npos ||
        source.find("RWStructuredBuffer") != std::string::npos ||
        source.find("Texture2D") != std::string::npos ||
        source.find("SamplerState") != std::string::npos) {
        return KX_SHADER_LANGUAGE_HLSL;
    }
    if (source.find("var<") != std::string::npos ||
        source.find("@group") != std::string::npos ||
        source.find("@binding") != std::string::npos ||
        source.find("fn ") != std::string::npos) {
        return KX_SHADER_LANGUAGE_WGSL;
    }
    return KX_SHADER_LANGUAGE_GLSL;
}

#ifdef __cplusplus
} /* extern "C" */
#endif