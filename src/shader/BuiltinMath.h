#ifndef KROMAX_SRC_SHADER_BUILTINMATH_H
#define KROMAX_SRC_SHADER_BUILTINMATH_H

/*
 * BuiltinMath.h
 *
 * Pure mathematical functions for shader builtins.
 * No dependencies on interpreter state - fully testable in isolation.
 * Reusable by Vulkan backend for SPIR-V builtin implementations.
 */

#include "ShaderTypes.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace kx {
namespace shader {
namespace builtin {

using Value = std::vector<float>;

/* -----------------------------------------------------------------------
 * Basic scalar operations
 * --------------------------------------------------------------------- */
inline float clamp_f(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float lerp_f(float a, float b, float t) {
    return a + t * (b - a);
}

inline float smoothstep_f(float edge0, float edge1, float x) {
    float t = clamp_f((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float step_f(float edge, float x) {
    return x < edge ? 0.0f : 1.0f;
}

inline float fract_f(float x) {
    return x - std::floor(x);
}

inline float mix_f(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

/* -----------------------------------------------------------------------
 * Vector operations
 * --------------------------------------------------------------------- */
inline Value swizzle_vec(const Value& v, const uint8_t* indices, uint8_t count) {
    Value result;
    result.reserve(count);
    for (uint8_t i = 0; i < count; ++i) {
        result.push_back(indices[i] < v.size() ? v[indices[i]] : 0.0f);
    }
    return result;
}

inline float dot_vec(const Value& a, const Value& b) {
    float sum = 0.0f;
    std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

inline Value cross_vec(const Value& a, const Value& b) {
    Value result(3);
    if (a.size() >= 3 && b.size() >= 3) {
        result[0] = a[1] * b[2] - a[2] * b[1];
        result[1] = a[2] * b[0] - a[0] * b[2];
        result[2] = a[0] * b[1] - a[1] * b[0];
    }
    return result;
}

inline float length_vec(const Value& v) {
    return std::sqrt(dot_vec(v, v));
}

inline Value normalize_vec(const Value& v) {
    float len = length_vec(v);
    if (len == 0.0f) return v;
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] / len;
    return result;
}

inline Value faceforward_vec(const Value& n, const Value& i, const Value& nref) {
    if (n.size() < 3 || i.size() < 3 || nref.size() < 3) return n;
    float dot = nref[0] * i[0] + nref[1] * i[1] + nref[2] * i[2];
    Value r = n;
    if (dot < 0.0f) for (auto& v : r) v = -v;
    return r;
}

inline Value reflect_vec(const Value& i, const Value& n) {
    if (i.size() != n.size() || i.empty()) return i;
    float dot = dot_vec(n, i);
    Value r(i.size());
    for (std::size_t k = 0; k < i.size(); ++k) r[k] = i[k] - 2.0f * dot * n[k];
    return r;
}

inline Value refract_vec(const Value& i, const Value& n, float eta) {
    // Simplified - return incident vector
    return i;
}

/* -----------------------------------------------------------------------
 * Matrix operations (column-major)
 * --------------------------------------------------------------------- */
inline Value mat_vec_mul(const Value& mat, const Value& vec, uint32_t cols, uint32_t rows) {
    Value result(rows);
    for (uint32_t r = 0; r < rows; ++r) {
        float sum = 0.0f;
        for (uint32_t c = 0; c < cols; ++c) {
            sum += mat[c * rows + r] * (c < vec.size() ? vec[c] : (c == 3 ? 1.0f : 0.0f));
        }
        result[r] = sum;
    }
    return result;
}

inline Value mat_mul(const Value& a, const Value& b, uint32_t a_cols, uint32_t a_rows, uint32_t b_cols) {
    Value result(a_cols * b_cols);
    for (uint32_t col = 0; col < a_cols; ++col) {
        for (uint32_t row = 0; row < b_cols; ++row) {
            float sum = 0.0f;
            for (uint32_t k = 0; k < a_rows; ++k) {
                sum += a[k * a_cols + col] * b[row * b_cols + k];
            }
            result[row * a_cols + col] = sum;
        }
    }
    return result;
}

inline Value transpose_mat(const Value& mat, uint32_t cols, uint32_t rows) {
    Value result(rows * cols);
    for (uint32_t c = 0; c < cols; ++c) {
        for (uint32_t r = 0; r < rows; ++r) {
            result[c * rows + r] = mat[r * cols + c];
        }
    }
    return result;
}

inline float determinant_mat(const Value& mat, uint32_t size) {
    if (size == 2) {
        return mat[0] * mat[3] - mat[1] * mat[2];
    }
    if (size == 3) {
        return mat[0] * (mat[4] * mat[8] - mat[5] * mat[7])
             - mat[1] * (mat[3] * mat[8] - mat[5] * mat[6])
             + mat[2] * (mat[3] * mat[7] - mat[4] * mat[6]);
    }
    if (size == 4) {
        float det = 0;
        for (int c = 0; c < 4; ++c) {
            Value minor(9);
            for (int i = 1; i < 4; ++i) {
                int mi = 0;
                for (int j = 0; j < 4; ++j) {
                    if (j == c) continue;
                    minor[(i - 1) * 3 + mi++] = mat[i * 4 + j];
                }
            }
            float sign = (c % 2 == 0) ? 1.0f : -1.0f;
            det += sign * mat[c] * determinant_mat(minor, 3);
        }
        return det;
    }
    return 0.0f;
}

inline Value inverse_mat(const Value& mat, uint32_t size) {
    Value result(size * size);
    float det = determinant_mat(mat, size);
    if (std::abs(det) < 1e-6f) {
        for (uint32_t i = 0; i < size; ++i) result[i * size + i] = 1.0f;
        return result;
    }
    if (size == 2) {
        float inv_det = 1.0f / det;
        result[0] = mat[3] * inv_det;
        result[1] = -mat[1] * inv_det;
        result[2] = -mat[2] * inv_det;
        result[3] = mat[0] * inv_det;
        return result;
    }
    if (size == 3) {
        float inv_det = 1.0f / det;
        result[0] = (mat[4] * mat[8] - mat[5] * mat[7]) * inv_det;
        result[1] = (mat[2] * mat[7] - mat[1] * mat[8]) * inv_det;
        result[2] = (mat[1] * mat[5] - mat[2] * mat[4]) * inv_det;
        result[3] = (mat[5] * mat[6] - mat[3] * mat[8]) * inv_det;
        result[4] = (mat[0] * mat[8] - mat[2] * mat[6]) * inv_det;
        result[5] = (mat[2] * mat[3] - mat[0] * mat[5]) * inv_det;
        result[6] = (mat[3] * mat[7] - mat[4] * mat[6]) * inv_det;
        result[7] = (mat[1] * mat[6] - mat[0] * mat[7]) * inv_det;
        result[8] = (mat[0] * mat[4] - mat[1] * mat[3]) * inv_det;
        return result;
    }
    if (size == 4) {
        float inv_det = 1.0f / det;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                Value minor(9);
                for (int mi = 0, r = 0; r < 4; ++r) {
                    if (r == i) continue;
                    for (int c = 0; c < 4; ++c) {
                        if (c == j) continue;
                        minor[mi++] = mat[r * 4 + c];
                    }
                }
                float sign = ((i + j) % 2 == 0) ? 1.0f : -1.0f;
                result[j * 4 + i] = sign * determinant_mat(minor, 3) * inv_det;
            }
        }
        return result;
    }
    return result;
}

inline Value outer_product(const Value& a, const Value& b) {
    Value result(a.size() * b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        for (std::size_t j = 0; j < b.size(); ++j) {
            result[i * b.size() + j] = a[i] * b[j];
        }
    }
    return result;
}

inline Value matrix_comp_mult(const Value& a, const Value& b) {
    Value result(std::min(a.size(), b.size()));
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = a[i] * b[i];
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Float/Int bit packing/unpacking
 * --------------------------------------------------------------------- */
inline uint32_t float_to_bits(float f) {
    return *reinterpret_cast<const uint32_t*>(&f);
}

inline float bits_to_float(uint32_t u) {
    return *reinterpret_cast<const float*>(&u);
}

inline float pack_snorm2x16(const Value& v) {
    int16_t x = static_cast<int16_t>(clamp_f(v.size() > 0 ? v[0] : 0.0f, -1.0f, 1.0f) * 32767.0f);
    int16_t y = static_cast<int16_t>(clamp_f(v.size() > 1 ? v[1] : 0.0f, -1.0f, 1.0f) * 32767.0f);
    uint32_t packed = (static_cast<uint32_t>(x) & 0xFFFF) | ((static_cast<uint32_t>(y) & 0xFFFF) << 16);
    return bits_to_float(packed);
}

inline Value unpack_snorm2x16(float packed) {
    uint32_t bits = float_to_bits(packed);
    int16_t x = static_cast<int16_t>(bits & 0xFFFF);
    int16_t y = static_cast<int16_t>((bits >> 16) & 0xFFFF);
    return Value{x / 32767.0f, y / 32767.0f};
}

inline float pack_unorm2x16(const Value& v) {
    uint16_t x = static_cast<uint16_t>(clamp_f(v.size() > 0 ? v[0] : 0.0f, 0.0f, 1.0f) * 65535.0f);
    uint16_t y = static_cast<uint16_t>(clamp_f(v.size() > 1 ? v[1] : 0.0f, 0.0f, 1.0f) * 65535.0f);
    uint32_t packed = (static_cast<uint32_t>(x) & 0xFFFF) | ((static_cast<uint32_t>(y) & 0xFFFF) << 16);
    return bits_to_float(packed);
}

inline Value unpack_unorm2x16(float packed) {
    uint32_t bits = float_to_bits(packed);
    uint16_t x = static_cast<uint16_t>(bits & 0xFFFF);
    uint16_t y = static_cast<uint16_t>((bits >> 16) & 0xFFFF);
    return Value{x / 65535.0f, y / 65535.0f};
}

inline float pack_half2x16(const Value& v) {
    // Simplified - pack as 16-bit normalized (not actual half)
    uint16_t x = static_cast<uint16_t>(clamp_f(v.size() > 0 ? v[0] : 0.0f, 0.0f, 1.0f) * 65535.0f);
    uint16_t y = static_cast<uint16_t>(clamp_f(v.size() > 1 ? v[1] : 0.0f, 0.0f, 1.0f) * 65535.0f);
    uint32_t packed = (static_cast<uint32_t>(x) & 0xFFFF) | ((static_cast<uint32_t>(y) & 0xFFFF) << 16);
    return bits_to_float(packed);
}

inline Value unpack_half2x16(float packed) {
    uint32_t bits = float_to_bits(packed);
    uint16_t x = static_cast<uint16_t>(bits & 0xFFFF);
    uint16_t y = static_cast<uint16_t>((bits >> 16) & 0xFFFF);
    return Value{x / 65535.0f, y / 65535.0f};
}

/* -----------------------------------------------------------------------
 * Noise functions (value noise)
 * --------------------------------------------------------------------- */
inline float noise_1d(float x) {
    int ix = static_cast<int>(std::floor(x));
    float fx = x - ix;
    fx = fx * fx * (3.0f - 2.0f * fx); // smoothstep
    uint32_t h = ix * 1664525u + 1013904223u;
    h = (h ^ (h >> 13)) * 1274126177u;
    float r0 = (h & 0xFFFFFF) / 16777215.0f;
    h = (ix + 1) * 1664525u + 1013904223u;
    h = (h ^ (h >> 13)) * 1274126177u;
    float r1 = (h & 0xFFFFFF) / 16777215.0f;
    return lerp_f(r0, r1, fx);
}

inline float noise_2d(float x, float y) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    auto hash = [](int x, int y) {
        uint32_t h = (x * 1664525u) ^ (y * 1013904223u);
        h = (h ^ (h >> 13)) * 1274126177u;
        return (h & 0xFFFFFF) / 16777215.0f;
    };
    float r00 = hash(ix, iy);
    float r10 = hash(ix + 1, iy);
    float r01 = hash(ix, iy + 1);
    float r11 = hash(ix + 1, iy + 1);
    float x0 = lerp_f(r00, r10, fx);
    float x1 = lerp_f(r01, r11, fx);
    return lerp_f(x0, x1, fy);
}

inline float noise_3d(float x, float y, float z) {
    // Simplified 3D noise
    return noise_2d(x + z * 0.318f, y + z * 0.712f);
}

inline float noise_4d(float x, float y, float z, float w) {
    return noise_3d(x + w * 0.123f, y + w * 0.456f, z + w * 0.789f);
}

/* -----------------------------------------------------------------------
 * Component-wise min/max/clamp for vectors
 * --------------------------------------------------------------------- */
template <typename Op>
inline Value vec_binary_op(const Value& a, const Value& b, Op op) {
    Value result(std::max(a.size(), b.size()));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float av = i < a.size() ? a[i] : 0.0f;
        float bv = i < b.size() ? b[i] : 0.0f;
        result[i] = op(av, bv);
    }
    return result;
}

inline Value vec_min(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return std::min(x, y); });
}

inline Value vec_max(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return std::max(x, y); });
}

inline Value vec_clamp(const Value& x, const Value& lo, const Value& hi) {
    Value result(std::max({x.size(), lo.size(), hi.size()}));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float xv = i < x.size() ? x[i] : 0.0f;
        float lov = i < lo.size() ? lo[i] : 0.0f;
        float hiv = i < hi.size() ? hi[i] : 0.0f;
        result[i] = clamp_f(xv, lov, hiv);
    }
    return result;
}

inline Value vec_mix(const Value& a, const Value& b, const Value& t) {
    Value result(std::max({a.size(), b.size(), t.size()}));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float av = i < a.size() ? a[i] : 0.0f;
        float bv = i < b.size() ? b[i] : 0.0f;
        float tv = i < t.size() ? t[i] : (t.size() == 1 ? t[0] : 0.0f);
        result[i] = mix_f(av, bv, tv);
    }
    return result;
}

inline Value vec_step(const Value& edge, const Value& x) {
    return vec_binary_op(edge, x, [](float e, float v) { return v < e ? 0.0f : 1.0f; });
}

inline Value vec_smoothstep(const Value& edge0, const Value& edge1, const Value& x) {
    Value result(std::max({edge0.size(), edge1.size(), x.size()}));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float e0 = i < edge0.size() ? edge0[i] : 0.0f;
        float e1 = i < edge1.size() ? edge1[i] : 0.0f;
        float xv = i < x.size() ? x[i] : 0.0f;
        float t = clamp_f((xv - e0) / (e1 - e0), 0.0f, 1.0f);
        result[i] = t * t * (3.0f - 2.0f * t);
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Comparison operations (return 0.0f or 1.0f per component)
 * --------------------------------------------------------------------- */
inline Value vec_lessThan(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x < y ? 1.0f : 0.0f; });
}

inline Value vec_lessThanEqual(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x <= y ? 1.0f : 0.0f; });
}

inline Value vec_greaterThan(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x > y ? 1.0f : 0.0f; });
}

inline Value vec_greaterThanEqual(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x >= y ? 1.0f : 0.0f; });
}

inline Value vec_equal(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x == y ? 1.0f : 0.0f; });
}

inline Value vec_notEqual(const Value& a, const Value& b) {
    return vec_binary_op(a, b, [](float x, float y) { return x != y ? 1.0f : 0.0f; });
}

inline Value vec_any(const Value& v) {
    Value r(1);
    r[0] = 0.0f;
    for (float f : v) if (f != 0.0f) { r[0] = 1.0f; break; }
    return r;
}

inline Value vec_all(const Value& v) {
    Value r(1);
    r[0] = 1.0f;
    for (float f : v) if (f == 0.0f) { r[0] = 0.0f; break; }
    return r;
}

/* -----------------------------------------------------------------------
 * Sign/abs/fma/frexp/ldexp/mod/modf
 * --------------------------------------------------------------------- */
inline Value vec_sign(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        result[i] = (v[i] > 0) ? 1.0f : ((v[i] < 0) ? -1.0f : 0.0f);
    }
    return result;
}

inline Value vec_abs(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::fabs(v[i]);
    return result;
}

inline Value vec_fma(const Value& a, const Value& b, const Value& c) {
    return vec_binary_op(a, b, [&](float x, float y) {
        float cv = c.size() > 0 ? c[0] : 0.0f;
        return x * y + cv;
    });
}

inline Value vec_frexp(const Value& v) {
    Value result(v.size() * 2); // value, exponent
    for (std::size_t i = 0; i < v.size(); ++i) {
        int exp;
        float mantissa = std::frexp(v[i], &exp);
        result[i] = mantissa;
        result[v.size() + i] = static_cast<float>(exp);
    }
    return result;
}

inline Value vec_ldexp(const Value& x, const Value& exp) {
    return vec_binary_op(x, exp, [](float xv, float ev) {
        return std::ldexp(xv, static_cast<int>(ev));
    });
}

inline Value vec_mod(const Value& x, const Value& y) {
    return vec_binary_op(x, y, [](float xv, float yv) { return std::fmod(xv, yv); });
}

inline Value vec_modf(const Value& x) {
    Value result(x.size() * 2); // fractional, integer
    for (std::size_t i = 0; i < x.size(); ++i) {
        float intpart;
        float frac = std::modf(x[i], &intpart);
        result[i] = frac;
        result[x.size() + i] = intpart;
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Bit manipulation
 * --------------------------------------------------------------------- */
inline Value vec_floatBitsToInt(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        result[i] = static_cast<float>(*reinterpret_cast<const int*>(&v[i]));
    }
    return result;
}

inline Value vec_floatBitsToUint(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        result[i] = static_cast<float>(*reinterpret_cast<const uint32_t*>(&v[i]));
    }
    return result;
}

inline Value vec_intBitsToFloat(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        int i32 = static_cast<int>(v[i]);
        result[i] = *reinterpret_cast<const float*>(&i32);
    }
    return result;
}

inline Value vec_uintBitsToFloat(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        uint32_t u32 = static_cast<uint32_t>(v[i]);
        result[i] = *reinterpret_cast<const float*>(&u32);
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Packing/Unpacking
 * --------------------------------------------------------------------- */
inline Value vec_packSnorm2x16(const Value& v) {
    Value result((v.size() + 1) / 2);
    for (std::size_t i = 0; i < result.size(); ++i) {
        float x = (i * 2 < v.size()) ? v[i * 2] : 0.0f;
        float y = (i * 2 + 1 < v.size()) ? v[i * 2 + 1] : 0.0f;
        Value tmp{x, y};
        result[i] = pack_snorm2x16(tmp);
    }
    return result;
}

inline Value vec_unpackSnorm2x16(const Value& v) {
    Value result(v.size() * 2);
    for (std::size_t i = 0; i < v.size(); ++i) {
        Value tmp = unpack_snorm2x16(v[i]);
        if (i * 2 < result.size()) result[i * 2] = tmp[0];
        if (i * 2 + 1 < result.size()) result[i * 2 + 1] = tmp[1];
    }
    return result;
}

inline Value vec_packUnorm2x16(const Value& v) {
    Value result((v.size() + 1) / 2);
    for (std::size_t i = 0; i < result.size(); ++i) {
        float x = (i * 2 < v.size()) ? v[i * 2] : 0.0f;
        float y = (i * 2 + 1 < v.size()) ? v[i * 2 + 1] : 0.0f;
        Value tmp{x, y};
        result[i] = pack_unorm2x16(tmp);
    }
    return result;
}

inline Value vec_unpackUnorm2x16(const Value& v) {
    Value result(v.size() * 2);
    for (std::size_t i = 0; i < v.size(); ++i) {
        Value tmp = unpack_unorm2x16(v[i]);
        if (i * 2 < result.size()) result[i * 2] = tmp[0];
        if (i * 2 + 1 < result.size()) result[i * 2 + 1] = tmp[1];
    }
    return result;
}

inline Value vec_packHalf2x16(const Value& v) {
    Value result((v.size() + 1) / 2);
    for (std::size_t i = 0; i < result.size(); ++i) {
        float x = (i * 2 < v.size()) ? v[i * 2] : 0.0f;
        float y = (i * 2 + 1 < v.size()) ? v[i * 2 + 1] : 0.0f;
        Value tmp{x, y};
        result[i] = pack_half2x16(tmp);
    }
    return result;
}

inline Value vec_unpackHalf2x16(const Value& v) {
    Value result(v.size() * 2);
    for (std::size_t i = 0; i < v.size(); ++i) {
        Value tmp = unpack_half2x16(v[i]);
        if (i * 2 < result.size()) result[i * 2] = tmp[0];
        if (i * 2 + 1 < result.size()) result[i * 2 + 1] = tmp[1];
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Radians/degrees
 * --------------------------------------------------------------------- */
inline Value vec_radians(const Value& v) {
    Value result(v.size());
    constexpr float DEG_TO_RAD = 0.017453292519943295f;
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] * DEG_TO_RAD;
    return result;
}

inline Value vec_degrees(const Value& v) {
    Value result(v.size());
    constexpr float RAD_TO_DEG = 57.29577951308232f;
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] * RAD_TO_DEG;
    return result;
}

/* -----------------------------------------------------------------------
 * Trigonometric
 * --------------------------------------------------------------------- */
inline Value vec_sin(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::sin(v[i]);
    return result;
}

inline Value vec_cos(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::cos(v[i]);
    return result;
}

inline Value vec_tan(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::tan(v[i]);
    return result;
}

inline Value vec_asin(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::asin(clamp_f(v[i], -1.0f, 1.0f));
    return result;
}

inline Value vec_acos(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::acos(clamp_f(v[i], -1.0f, 1.0f));
    return result;
}

inline Value vec_atan(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::atan(v[i]);
    return result;
}

inline Value vec_atan2(const Value& y, const Value& x) {
    Value result(std::max(y.size(), x.size()));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float yv = i < y.size() ? y[i] : 0.0f;
        float xv = i < x.size() ? x[i] : 0.0f;
        result[i] = std::atan2(yv, xv);
    }
    return result;
}

inline Value vec_sinh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::sinh(v[i]);
    return result;
}

inline Value vec_cosh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::cosh(v[i]);
    return result;
}

inline Value vec_tanh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::tanh(v[i]);
    return result;
}

inline Value vec_asinh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::asinh(v[i]);
    return result;
}

inline Value vec_acosh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::acosh(v[i]);
    return result;
}

inline Value vec_atanh(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::atanh(clamp_f(v[i], -1.0f + 1e-6f, 1.0f - 1e-6f));
    return result;
}

/* -----------------------------------------------------------------------
 * Exponential/Logarithmic
 * --------------------------------------------------------------------- */
inline Value vec_exp(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::exp(v[i]);
    return result;
}

inline Value vec_log(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::log(std::max(v[i], 1e-10f));
    return result;
}

inline Value vec_exp2(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::exp2(v[i]);
    return result;
}

inline Value vec_log2(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::log2(std::max(v[i], 1e-10f));
    return result;
}

inline Value vec_sqrt(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::sqrt(std::max(v[i], 0.0f));
    return result;
}

inline Value vec_inversesqrt(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = 1.0f / std::sqrt(std::max(v[i], 1e-10f));
    return result;
}

inline Value vec_pow(const Value& x, const Value& y) {
    Value result(std::max(x.size(), y.size()));
    for (std::size_t i = 0; i < result.size(); ++i) {
        float xv = i < x.size() ? x[i] : 0.0f;
        float yv = i < y.size() ? y[i] : 0.0f;
        result[i] = std::pow(std::max(xv, 0.0f), yv);
    }
    return result;
}

/* -----------------------------------------------------------------------
 * Rounding
 * --------------------------------------------------------------------- */
inline Value vec_floor(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::floor(v[i]);
    return result;
}

inline Value vec_ceil(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::ceil(v[i]);
    return result;
}

inline Value vec_fract(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] - std::floor(v[i]);
    return result;
}

inline Value vec_trunc(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::trunc(v[i]);
    return result;
}

inline Value vec_round(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::round(v[i]);
    return result;
}

inline Value vec_roundEven(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::nearbyint(v[i]);
    return result;
}

/* -----------------------------------------------------------------------
 * NaN/Inf checks
 * --------------------------------------------------------------------- */
inline Value vec_isnan(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::isnan(v[i]) ? 1.0f : 0.0f;
    return result;
}

inline Value vec_isinf(const Value& v) {
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = std::isinf(v[i]) ? 1.0f : 0.0f;
    return result;
}

} // namespace builtin
} // namespace shader
} // namespace kx

#endif // KROMAX_SRC_SHADER_BUILTINMATH_H