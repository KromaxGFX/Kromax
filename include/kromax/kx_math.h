#ifndef KX_MATH_H
#define KX_MATH_H

/*
 * kx_math.h
 *
 * Public 4x4 column-major matrix math helpers for Kromax.
 * Conventions match examples/03_triangle/main.cpp:
 * - Column-major storage: element at column c, row r is m[c * 4 + r]
 * - Matrices are stored as transposes of standard mathematical matrices
 *   (the mathematical matrix rows become storage columns)
 * - kxMat4Multiply(a, b) computes out = a * b using column-major layout,
 *   i.e. the result applies b first, then a
 * - Right-handed coordinate system
 * - Perspective projection uses [-1, 1] depth range (OpenGL convention)
 * - All functions are header-only, static inline, C99 compatible
 */

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Matrix type
 * ======================================================================= */

typedef struct KxMat4 {
    float m[16]; /* column-major: element at column c, row r is m[c * 4 + r] */
} KxMat4;

/* =======================================================================
 * Matrix operations
 * ======================================================================= */

/* Identity matrix */
static inline void kxMat4Identity(KxMat4* out) {
    if (!out) return;
    for (int i = 0; i < 16; ++i) out->m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

/* Multiply: out = a * b (column-major layout, applies b first then a) */
static inline void kxMat4Multiply(KxMat4* out, const KxMat4* a, const KxMat4* b) {
    if (!out || !a || !b) return;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += a->m[row * 4 + k] * b->m[k * 4 + col];
            }
            out->m[row * 4 + col] = sum;
        }
    }
}

/* Perspective projection (right-handed, [-1, 1] depth range) */
static inline void kxMat4Perspective(KxMat4* out, float fovYRadians, float aspect, float zNear, float zFar) {
    if (!out) return;
    for (int i = 0; i < 16; ++i) out->m[i] = 0.0f;
    float f = 1.0f / tanf(fovYRadians * 0.5f);
    out->m[0] = f / aspect;                    /* column 0, row 0 */
    out->m[5] = f;                             /* column 1, row 1 */
    out->m[10] = (zFar + zNear) / (zNear - zFar); /* column 2, row 2 */
    out->m[11] = -1.0f;                        /* column 2, row 3 */
    out->m[14] = (2.0f * zFar * zNear) / (zNear - zFar); /* column 3, row 2 */
    out->m[15] = 0.0f;                         /* column 3, row 3 */
}

/* Look-at view matrix (right-handed) */
static inline void kxMat4LookAt(KxMat4* out, const float eye[3], const float center[3], const float up[3]) {
    if (!out) return;
    for (int i = 0; i < 16; ++i) out->m[i] = 0.0f;

    float fx = center[0] - eye[0];
    float fy = center[1] - eye[1];
    float fz = center[2] - eye[2];
    float fl = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= fl; fy /= fl; fz /= fl;

    float sx = fy * up[2] - fz * up[1];
    float sy = fz * up[0] - fx * up[2];
    float sz = fx * up[1] - fy * up[0];
    float sl = sqrtf(sx*sx + sy*sy + sz*sz);
    sx /= sl; sy /= sl; sz /= sl;

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    out->m[0] = sx;  out->m[1] = ux;  out->m[2] = -fx; out->m[3] = 0.0f;
    out->m[4] = sy;  out->m[5] = uy;  out->m[6] = -fy; out->m[7] = 0.0f;
    out->m[8] = sz;  out->m[9] = uz;  out->m[10] = -fz; out->m[11] = 0.0f;
    out->m[12] = -(sx*eye[0] + sy*eye[1] + sz*eye[2]);
    out->m[13] = -(ux*eye[0] + uy*eye[1] + uz*eye[2]);
    out->m[14] =  fx*eye[0] + fy*eye[1] + fz*eye[2];
    out->m[15] = 1.0f;
}

/* Rotation around X axis (right-handed) */
static inline void kxMat4RotationX(KxMat4* out, float radians) {
    if (!out) return;
    kxMat4Identity(out);
    float c = cosf(radians);
    float s = sinf(radians);
    out->m[5] = c;   out->m[9] = -s;
    out->m[6] = s;   out->m[10] = c;
}

/* Rotation around Y axis (right-handed) */
static inline void kxMat4RotationY(KxMat4* out, float radians) {
    if (!out) return;
    kxMat4Identity(out);
    float c = cosf(radians);
    float s = sinf(radians);
    out->m[0] = c;  out->m[8] = s;
    out->m[2] = -s; out->m[10] = c;
}

/* Rotation around Z axis (right-handed) */
static inline void kxMat4RotationZ(KxMat4* out, float radians) {
    if (!out) return;
    kxMat4Identity(out);
    float c = cosf(radians);
    float s = sinf(radians);
    out->m[0] = c;  out->m[4] = -s;
    out->m[1] = s;  out->m[5] = c;
}

/* Translation matrix */
static inline void kxMat4Translation(KxMat4* out, float x, float y, float z) {
    if (!out) return;
    kxMat4Identity(out);
    out->m[12] = x;
    out->m[13] = y;
    out->m[14] = z;
}

/* Scale matrix */
static inline void kxMat4Scale(KxMat4* out, float x, float y, float z) {
    if (!out) return;
    for (int i = 0; i < 16; ++i) out->m[i] = 0.0f;
    out->m[0] = x;
    out->m[5] = y;
    out->m[10] = z;
    out->m[15] = 1.0f;
}

/* Transform point by matrix (applies translation, writes w to out[3]) */
static inline void kxMat4TransformPoint(const KxMat4* m, const float in[3], float out[4]) {
    if (!m || !in || !out) return;
    float px = in[0], py = in[1], pz = in[2], pw = 1.0f;
    out[0] = m->m[0] * px + m->m[4] * py + m->m[8] * pz + m->m[12] * pw;
    out[1] = m->m[1] * px + m->m[5] * py + m->m[9] * pz + m->m[13] * pw;
    out[2] = m->m[2] * px + m->m[6] * py + m->m[10] * pz + m->m[14] * pw;
    out[3] = m->m[3] * px + m->m[7] * py + m->m[11] * pz + m->m[15] * pw;
}

/* Transform direction vector by matrix (ignores translation) */
static inline void kxMat4TransformVector(const KxMat4* m, const float in[3], float out[3]) {
    if (!m || !in || !out) return;
    float vx = in[0], vy = in[1], vz = in[2];
    out[0] = m->m[0] * vx + m->m[4] * vy + m->m[8] * vz;
    out[1] = m->m[1] * vx + m->m[5] * vy + m->m[9] * vz;
    out[2] = m->m[2] * vx + m->m[6] * vy + m->m[10] * vz;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_MATH_H */