#ifndef KROMAX_SRC_SHADER_SHADERTYPES_H
#define KROMAX_SRC_SHADER_SHADERTYPES_H

/*
 * ShaderTypes.h
 *
 * Type system shared by every shader front-end. The front-ends (GLSL, HLSL,
 * WGSL, KXSL) normalise their native types onto these descriptors so the
 * parser, the IR and the interpreter only ever deal with one representation.
 */

#include <cstdint>
#include <string>

#include "kx_core.h"

namespace kx {
namespace shader {

enum class TypeClass : uint8_t {
    Void,
    Bool,
    Int,
    UInt,
    Float,
    Vector,        /* rows = component count (2..4)              */
    Matrix,        /* rows = column count, cols = row count       */
    Struct,
    Texture,       /* sampled image resource                      */
    Sampler,       /* sampler resource                            */
    StorageBuffer, /* read/write buffer resource                  */
    UniformBuffer, /* read only buffer resource                   */
};

struct ShaderType {
    TypeClass cls = TypeClass::Void;
    uint32_t rows = 1;       /* vector components / matrix columns */
    uint32_t cols = 1;       /* matrix rows                       */
    uint32_t struct_id = UINT32_MAX;
    uint32_t elem_size = 4;  /* bytes per scalar element          */

    bool is_scalar() const noexcept {
        return cls == TypeClass::Bool || cls == TypeClass::Int ||
               cls == TypeClass::UInt || cls == TypeClass::Float;
    }

    bool is_vector() const noexcept { return cls == TypeClass::Vector; }
    bool is_matrix() const noexcept { return cls == TypeClass::Matrix; }
    bool is_numeric() const noexcept {
        return cls == TypeClass::Int || cls == TypeClass::UInt ||
               cls == TypeClass::Float || cls == TypeClass::Vector ||
               cls == TypeClass::Matrix;
    }
    bool is_integer() const noexcept {
        return cls == TypeClass::Int || cls == TypeClass::UInt;
    }
    bool is_resource() const noexcept {
        return cls == TypeClass::Texture || cls == TypeClass::Sampler ||
               cls == TypeClass::StorageBuffer || cls == TypeClass::UniformBuffer;
    }

    /* Number of scalar components (matrices are flattened column major). */
    uint32_t component_count() const noexcept {
        if (is_scalar()) return 1;
        if (cls == TypeClass::Vector) return rows;
        if (cls == TypeClass::Matrix) return rows * cols;
        return 1;
    }

    uint32_t byte_size() const noexcept {
        return component_count() * elem_size;
    }

    bool operator==(const ShaderType& other) const noexcept {
        return cls == other.cls && rows == other.rows && cols == other.cols &&
               struct_id == other.struct_id;
    }
    bool operator!=(const ShaderType& other) const noexcept {
        return !(*this == other);
    }
};

/* Constructors for the common types. */
inline ShaderType make_void() { return ShaderType{}; }
inline ShaderType make_bool() {
    ShaderType t{};
    t.cls = TypeClass::Bool;
    t.elem_size = 4;
    return t;
}
inline ShaderType make_int() {
    ShaderType t{};
    t.cls = TypeClass::Int;
    t.elem_size = 4;
    return t;
}
inline ShaderType make_uint() {
    ShaderType t{};
    t.cls = TypeClass::UInt;
    t.elem_size = 4;
    return t;
}
inline ShaderType make_float() {
    ShaderType t{};
    t.cls = TypeClass::Float;
    t.elem_size = 4;
    return t;
}
inline ShaderType make_vector(uint32_t components) {
    ShaderType t{};
    t.cls = TypeClass::Vector;
    t.rows = components < 1u ? 1u : (components > 4u ? 4u : components);
    t.elem_size = 4;
    return t;
}
inline ShaderType make_matrix(uint32_t columns, uint32_t rows) {
    ShaderType t{};
    t.cls = TypeClass::Matrix;
    t.rows = columns < 1u ? 1u : (columns > 4u ? 4u : columns);
    t.cols = rows < 1u ? 1u : (rows > 4u ? 4u : rows);
    t.elem_size = 4;
    return t;
}

const char* type_class_name(TypeClass cls) noexcept;

/* Scalar type promotion rules shared by the parser and the interpreter. */
ShaderType promote(ShaderType a, ShaderType b) noexcept;

/* Convert a value from one scalar type to another (bool / int / uint / float). */
float convert_scalar(float value, TypeClass from, TypeClass to) noexcept;

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERTYPES_H */
