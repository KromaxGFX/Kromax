/*
 * ShaderTypes.cpp
 *
 * Implementation of type system utilities.
 */

#include "ShaderTypes.h"

#include <string>

namespace kx {
namespace shader {

const char* type_class_name(TypeClass cls) noexcept {
    switch (cls) {
        case TypeClass::Void: return "void";
        case TypeClass::Bool: return "bool";
        case TypeClass::Int: return "int";
        case TypeClass::UInt: return "uint";
        case TypeClass::Float: return "float";
        case TypeClass::Vector: return "vector";
        case TypeClass::Matrix: return "matrix";
        case TypeClass::Struct: return "struct";
        case TypeClass::Texture: return "texture";
        case TypeClass::Sampler: return "sampler";
        case TypeClass::StorageBuffer: return "storage_buffer";
        case TypeClass::UniformBuffer: return "uniform_buffer";
        default: return "unknown";
    }
}

ShaderType promote(ShaderType a, ShaderType b) noexcept {
    /* If either is not numeric, return the first */
    if (!a.is_numeric() || !b.is_numeric()) return a;

    /* Promote integer types to float if either is float */
    bool a_is_float = a.cls == TypeClass::Float || a.cls == TypeClass::Vector || a.cls == TypeClass::Matrix;
    bool b_is_float = b.cls == TypeClass::Float || b.cls == TypeClass::Vector || b.cls == TypeClass::Matrix;

    TypeClass result_cls = (a_is_float || b_is_float) ? TypeClass::Float : TypeClass::Int;

    /* For vectors/matrices, preserve the shape and class */
    bool a_is_vec = a.cls == TypeClass::Vector || a.cls == TypeClass::Matrix || (a.is_scalar() && a.rows > 1);
    bool b_is_vec = b.cls == TypeClass::Vector || b.cls == TypeClass::Matrix || (b.is_scalar() && b.rows > 1);

    if (a_is_vec) {
        ShaderType result = a;
        if (a.cls == TypeClass::Vector || a.cls == TypeClass::Matrix) {
            result.cls = a.cls;
        } else {
            /* Legacy vector representation: scalar cls with rows > 1 -> convert to proper Vector */
            result.cls = result_cls == TypeClass::Int ? TypeClass::Vector : TypeClass::Vector;
            /* rows already set correctly */
        }
        return result;
    }
    if (b_is_vec) {
        ShaderType result = b;
        if (b.cls == TypeClass::Vector || b.cls == TypeClass::Matrix) {
            result.cls = b.cls;
        } else {
            result.cls = result_cls == TypeClass::Int ? TypeClass::Vector : TypeClass::Vector;
        }
        return result;
    }

    /* Both scalars */
    ShaderType result = make_float();
    if (result_cls == TypeClass::Int) result = make_int();
    return result;
}

} /* namespace shader */
} /* namespace kx */