#include "ParserTypes.h"

#include <algorithm>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Type parsing implementations
 * --------------------------------------------------------------------- */
ShaderType ParserTypes::parse_type() {
    if (check(TokenType::Identifier)) {
        const std::string& name = peek().text;

        if (name == "bool") return parse_base_type();
        if (name == "int" || name == "int32") return parse_base_type();
        if (name == "uint" || name == "uint32") return parse_base_type();
        if (name == "float" || name == "float32") return parse_base_type();
        if (name == "half" || name == "float16") return parse_base_type();

        if (name == "vec2" || name == "vec3" || name == "vec4" ||
            name == "ivec2" || name == "ivec3" || name == "ivec4" ||
            name == "uvec2" || name == "uvec3" || name == "uvec4" ||
            name == "bvec2" || name == "bvec3" || name == "bvec4" ||
            name == "float2" || name == "float3" || name == "float4" ||
            name == "int2" || name == "int3" || name == "int4" ||
            name == "uint2" || name == "uint3" || name == "uint4") {
            return parse_vector_type();
        }

        if (name == "mat2" || name == "mat3" || name == "mat4" ||
            name == "mat2x2" || name == "mat2x3" || name == "mat2x4" ||
            name == "mat3x2" || name == "mat3x3" || name == "mat3x4" ||
            name == "mat4x2" || name == "mat4x3" || name == "mat4x4" ||
            name == "float2x2" || name == "float3x3" || name == "float4x4") {
            return parse_matrix_type();
        }

        if (name == "texture" || name == "Texture2D" || name == "Texture3D" ||
            name == "TextureCube" || name == "Texture2DArray" ||
            name == "texture_2d" || name == "texture_3d" ||
            name == "texture_cube" || name == "texture_2d_array") {
            return parse_texture_type();
        }

        if (name == "sampler" || name == "SamplerState" ||
            name == "SamplerComparisonState" || name == "sampler_comparison") {
            return parse_sampler_type();
        }

        if (name == "buffer" || name == "StructuredBuffer" || name == "RWStructuredBuffer" ||
            name == "ByteAddressBuffer" || name == "RWByteAddressBuffer") {
            consume();
            return ShaderType{TypeClass::StorageBuffer};
        }

        /* User-defined struct */
        if (std::find(struct_names_.begin(), struct_names_.end(), name) != struct_names_.end()) {
            consume();
            return ShaderType{TypeClass::Struct, 0, 0, 0, 4};
        }
    }

    /* Unknown type - consume and default to float */
    consume();
    return make_float();
}

ShaderType ParserTypes::parse_base_type() {
    const std::string& name = peek().text;
    if (name == "bool") { consume(); return make_bool(); }
    if (name == "int" || name == "int32") { consume(); return make_int(); }
    if (name == "uint" || name == "uint32") { consume(); return make_uint(); }
    if (name == "float" || name == "float32") { consume(); return make_float(); }
    if (name == "half" || name == "float16") { consume(); return make_float(); }
    consume();
    return make_float();
}

ShaderType ParserTypes::parse_vector_type() {
    const std::string& name = peek().text;
    uint32_t components = 4;
    TypeClass cls = TypeClass::Float;

    if (name == "vec2" || name == "float2") components = 2;
    else if (name == "vec3" || name == "float3") components = 3;
    else if (name == "vec4" || name == "float4") components = 4;
    else if (name == "ivec2" || name == "int2") { components = 2; cls = TypeClass::Int; }
    else if (name == "ivec3" || name == "int3") { components = 3; cls = TypeClass::Int; }
    else if (name == "ivec4" || name == "int4") { components = 4; cls = TypeClass::Int; }
    else if (name == "uvec2" || name == "uint2") { components = 2; cls = TypeClass::UInt; }
    else if (name == "uvec3" || name == "uint3") { components = 3; cls = TypeClass::UInt; }
    else if (name == "uvec4" || name == "uint4") { components = 4; cls = TypeClass::UInt; }
    else if (name == "bvec2") { components = 2; cls = TypeClass::Bool; }
    else if (name == "bvec3") { components = 3; cls = TypeClass::Bool; }
    else if (name == "bvec4") { components = 4; cls = TypeClass::Bool; }

    consume();
    ShaderType t = make_vector(components);
    t.cls = cls;
    return t;
}

ShaderType ParserTypes::parse_matrix_type() {
    const std::string& name = peek().text;
    uint32_t cols = 4, rows = 4;

    if (name == "mat2" || name == "float2x2") { cols = 2; rows = 2; }
    else if (name == "mat3" || name == "float3x3") { cols = 3; rows = 3; }
    else if (name == "mat4" || name == "float4x4") { cols = 4; rows = 4; }
    else if (name == "mat2x3") { cols = 2; rows = 3; }
    else if (name == "mat2x4") { cols = 2; rows = 4; }
    else if (name == "mat3x2") { cols = 3; rows = 2; }
    else if (name == "mat3x4") { cols = 3; rows = 4; }
    else if (name == "mat4x2") { cols = 4; rows = 2; }
    else if (name == "mat4x3") { cols = 4; rows = 3; }

    consume();
    return make_matrix(cols, rows);
}

ShaderType ParserTypes::parse_struct_type() {
    consume();
    ShaderType t{};
    t.cls = TypeClass::Struct;
    return t;
}

ShaderType ParserTypes::parse_texture_type() {
    consume();
    return ShaderType{TypeClass::Texture};
}

ShaderType ParserTypes::parse_sampler_type() {
    consume();
    return ShaderType{TypeClass::Sampler};
}

bool ParserTypes::is_type_name(const std::string& name) const {
    static const std::vector<std::string> types = {
        "bool", "int", "uint", "float", "half",
        "vec2", "vec3", "vec4", "ivec2", "ivec3", "ivec4",
        "uvec2", "uvec3", "uvec4", "bvec2", "bvec3", "bvec4",
        "float2", "float3", "float4", "int2", "int3", "int4",
        "uint2", "uint3", "uint4",
        "mat2", "mat3", "mat4", "mat2x2", "mat2x3", "mat2x4",
        "mat3x2", "mat3x3", "mat3x4", "mat4x2", "mat4x3", "mat4x4",
        "float2x2", "float3x3", "float4x4",
        "texture", "Texture2D", "Texture3D", "TextureCube", "Texture2DArray",
        "sampler", "SamplerState", "SamplerComparisonState",
        "buffer", "StructuredBuffer", "RWStructuredBuffer"
    };
    return std::binary_search(types.begin(), types.end(), name);
}

} // namespace shader
} // namespace kx