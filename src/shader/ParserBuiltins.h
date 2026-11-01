#ifndef KROMAX_SRC_SHADER_PARSERBUILTINS_H
#define KROMAX_SRC_SHADER_PARSERBUILTINS_H

/*
 * ParserBuiltins.h
 *
 * Builtin function names for GLSL/HLSL/WGSL/KXSL.
 * Shared between parser modules.
 */

#include <algorithm>
#include <string>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Builtin function list (sorted for binary_search)
 * --------------------------------------------------------------------- */
inline bool is_builtin_function(const std::string& name) {
    static const std::vector<std::string> builtins = {
        "abs", "acos", "acosh", "all", "any", "asin", "asinh", "atan", "atanh",
        "atomicAdd", "atomicAnd", "atomicCompSwap", "atomicExchange", "atomicMax",
        "atomicMin", "atomicOr", "atomicXor",
        "clamp", "cos", "cosh", "cross", "degrees", "determinant", "distance",
        "dot", "dFdx", "dFdy", "equal", "exp", "exp2", "faceforward", "floatBitsToInt",
        "floatBitsToUint", "floor", "fma", "fract", "frexp", "fwidth", "greaterThan",
        "greaterThanEqual", "inverse", "inversesqrt", "interpolateAtCentroid",
        "interpolateAtOffset", "interpolateAtSample", "intBitsToFloat", "isinf",
        "isnan", "ldexp", "length", "lessThan", "lessThanEqual", "log", "log2",
        "matrixCompMult", "max", "min", "mix", "mod", "modf", "noise", "normalize",
        "notEqual", "outerProduct", "packHalf2x16", "packSnorm2x16", "packUnorm2x16",
        "pow", "radians", "reflect", "refract", "round", "roundEven", "sign",
        "sin", "sinh", "smoothstep", "sqrt", "step", "tan", "tanh", "texture",
        "textureGrad", "textureLod", "textureSize", "transpose", "trunc",
        "uintBitsToFloat", "unpackHalf2x16", "unpackSnorm2x16", "unpackUnorm2x16",
    };
    return std::binary_search(builtins.begin(), builtins.end(), name);
}

} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_PARSERBUILTINS_H