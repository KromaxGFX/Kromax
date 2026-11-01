/*
 * ArmMath.cpp
 *
 * ARM64/NEON optimized math functions.
 * Falls back to scalar implementation on non-ARM64 platforms.
 */

#include "kx_arm.h"

#include <cmath>
#include <cstring>

/* Detect ARM64 at compile time */
#if defined(__aarch64__) || defined(_M_ARM64)
#define KX_ARM64_COMPILED 1
#else
#define KX_ARM64_COMPILED 0
#endif

#if KX_ARM64_COMPILED && defined(KX_HAVE_NEON)
#include <arm_neon.h>
#define KX_HAVE_NEON_IMPL 1
#else
#define KX_HAVE_NEON_IMPL 0
#endif

namespace kx {

/* ========================================================================
 * CPU feature detection
 * ====================================================================== */

static KxArmCpuFeatures g_armFeatures = KX_ARM_CPU_FEATURE_NONE;
static bool g_armFeaturesDetected = false;

KxArmCpuFeatures detectArmCpuFeatures() {
    KxArmCpuFeatures features = KX_ARM_CPU_FEATURE_NONE;

#if KX_ARM64_COMPILED
#if defined(__linux__)
    /* Read CPU features from /proc/cpuinfo on Linux */
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "Features") || strstr(line, "features")) {
                char* featuresStr = strchr(line, ':');
                if (featuresStr) {
                    featuresStr++;
                    while (*featuresStr == ' ') featuresStr++;

                    if (strstr(featuresStr, "neon")) features |= KX_ARM_CPU_FEATURE_NEON;
                    if (strstr(featuresStr, "fp16")) features |= KX_ARM_CPU_FEATURE_FP16_ARITHMETIC;
                    if (strstr(featuresStr, "dotprod")) features |= KX_ARM_CPU_FEATURE_DOTPROD;
                    if (strstr(featuresStr, "bf16")) features |= KX_ARM_CPU_FEATURE_BF16;
                    if (strstr(featuresStr, "i8mm")) features |= KX_ARM_CPU_FEATURE_I8MM;
                    if (strstr(featuresStr, "sve")) features |= KX_ARM_CPU_FEATURE_SVE;
                    if (strstr(featuresStr, "sve2")) features |= KX_ARM_CPU_FEATURE_SVE2;
                }
            }
        }
        fclose(fp);
    }
#elif defined(__APPLE__)
    /* On macOS, use sysctlbyname */
    int mib[2] = {CTL_HW, HW_CPUSUBTYPE};
    int subtype = 0;
    size_t len = sizeof(subtype);
    if (sysctlbyname("hw.optional.arm64", &subtype, &len, NULL, 0) == 0) {
        features |= KX_ARM_CPU_FEATURE_NEON;  /* All Apple Silicon has NEON */
        features |= KX_ARM_CPU_FEATURE_FP16_ARITHMETIC;
        features |= KX_ARM_CPU_FEATURE_DOTPROD;
        features |= KX_ARM_CPU_FEATURE_BF16;
    }
#elif defined(_WIN32)
    /* On Windows ARM64, use IsProcessorFeaturePresent */
    if (IsProcessorFeaturePresent(PF_ARM_V8_INSTRUCTIONS_AVAILABLE)) {
        features |= KX_ARM_CPU_FEATURE_NEON;
    }
    if (IsProcessorFeaturePresent(PF_ARM_FP16_INSTRUCTIONS_AVAILABLE)) {
        features |= KX_ARM_CPU_FEATURE_FP16_ARITHMETIC;
    }
    if (IsProcessorFeaturePresent(PF_ARM_DOTPROD_INSTRUCTIONS_AVAILABLE)) {
        features |= KX_ARM_CPU_FEATURE_DOTPROD;
    }
#endif
#endif

    return features;
}

/* ========================================================================
 * Public API
 * ====================================================================== */

} /* namespace kx */

extern "C" {

KX_API KxResult KX_CALL kxGetArmCpuFeatures(
    KxArmCpuFeatures* pFeatures) {
    if (!pFeatures) return KX_ERROR_INVALID_POINTER;

    if (!g_armFeaturesDetected) {
        g_armFeatures = kx::detectArmCpuFeatures();
        g_armFeaturesDetected = true;
    }

    *pFeatures = g_armFeatures;
    return KX_SUCCESS;
}

KX_API KxResult KX_CALL kxGetArmCpuInfo(
    KxArmCpuInfo* pInfo) {
    if (!pInfo) return KX_ERROR_INVALID_POINTER;

    KxArmCpuFeatures features;
    KxResult result = kxGetArmCpuFeatures(&features);
    if (result != KX_SUCCESS) return result;

    pInfo->sType = KX_STRUCTURE_TYPE_ARM_CPU_INFO;
    pInfo->pNext = nullptr;
    pInfo->features = features;

#if KX_ARM64_COMPILED
#if defined(__linux__)
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "CPU implementer", 15) == 0) {
                sscanf(line, "CPU implementer\t: %x", &pInfo->implementer);
            } else if (strncmp(line, "CPU variant", 11) == 0) {
                sscanf(line, "CPU variant\t: %x", &pInfo->variant);
            } else if (strncmp(line, "CPU part", 8) == 0) {
                sscanf(line, "CPU part\t: %x", &pInfo->part);
            } else if (strncmp(line, "CPU revision", 12) == 0) {
                sscanf(line, "CPU revision\t: %u", &pInfo->revision);
            } else if (strncmp(line, "model name", 10) == 0) {
                char* colon = strchr(line, ':');
                if (colon) {
                    strncpy(pInfo->name, colon + 2, sizeof(pInfo->name) - 1);
                    pInfo->name[sizeof(pInfo->name) - 1] = '\0';
                    /* Remove trailing newline */
                    char* nl = strchr(pInfo->name, '\n');
                    if (nl) *nl = '\0';
                }
            }
        }
        fclose(fp);
    }
#elif defined(__APPLE__)
    strncpy(pInfo->name, "Apple Silicon", sizeof(pInfo->name) - 1);
    size_t len = sizeof(pInfo->cpuid);
    sysctlbyname("hw.cpufrequency", &pInfo->cpuid, &len, NULL, 0);
#elif defined(_WIN32)
    strncpy(pInfo->name, "Windows ARM64", sizeof(pInfo->name) - 1);
#endif
#else
    pInfo->features = KX_ARM_CPU_FEATURE_NONE;
#endif

    return KX_SUCCESS;
}

/* ========================================================================
 * NEON-accelerated math (with scalar fallback)
 * ====================================================================== */

KX_API void KX_CALL kxNeonVectorAdd(
    KxNeonVector4* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count) {
    if (!dst || !a || !b || count == 0) return;

#if KX_HAVE_NEON_IMPL
    uint32_t i = 0;
    for (; i + 3 < count; i += 4) {
        float32x4x4_t va = vld4q_f32(reinterpret_cast<const float*>(&a[i]));
        float32x4x4_t vb = vld4q_f32(reinterpret_cast<const float*>(&b[i]));
        float32x4x4_t vc;
        vc.val[0] = vaddq_f32(va.val[0], vb.val[0]);
        vc.val[1] = vaddq_f32(va.val[1], vb.val[1]);
        vc.val[2] = vaddq_f32(va.val[2], vb.val[2]);
        vc.val[3] = vaddq_f32(va.val[3], vb.val[3]);
        vst4q_f32(reinterpret_cast<float*>(&dst[i]), vc);
    }
    for (; i < count; ++i) {
        dst[i].x = a[i].x + b[i].x;
        dst[i].y = a[i].y + b[i].y;
        dst[i].z = a[i].z + b[i].z;
        dst[i].w = a[i].w + b[i].w;
    }
#else
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].x = a[i].x + b[i].x;
        dst[i].y = a[i].y + b[i].y;
        dst[i].z = a[i].z + b[i].z;
        dst[i].w = a[i].w + b[i].w;
    }
#endif
}

KX_API void KX_CALL kxNeonVectorMul(
    KxNeonVector4* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count) {
    if (!dst || !a || !b || count == 0) return;

#if KX_HAVE_NEON_IMPL
    uint32_t i = 0;
    for (; i + 3 < count; i += 4) {
        float32x4x4_t va = vld4q_f32(reinterpret_cast<const float*>(&a[i]));
        float32x4x4_t vb = vld4q_f32(reinterpret_cast<const float*>(&b[i]));
        float32x4x4_t vc;
        vc.val[0] = vmulq_f32(va.val[0], vb.val[0]);
        vc.val[1] = vmulq_f32(va.val[1], vb.val[1]);
        vc.val[2] = vmulq_f32(va.val[2], vb.val[2]);
        vc.val[3] = vmulq_f32(va.val[3], vb.val[3]);
        vst4q_f32(reinterpret_cast<float*>(&dst[i]), vc);
    }
    for (; i < count; ++i) {
        dst[i].x = a[i].x * b[i].x;
        dst[i].y = a[i].y * b[i].y;
        dst[i].z = a[i].z * b[i].z;
        dst[i].w = a[i].w * b[i].w;
    }
#else
    for (uint32_t i = 0; i < count; ++i) {
        dst[i].x = a[i].x * b[i].x;
        dst[i].y = a[i].y * b[i].y;
        dst[i].z = a[i].z * b[i].z;
        dst[i].w = a[i].w * b[i].w;
    }
#endif
}

KX_API void KX_CALL kxNeonVectorDot(
    float* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count) {
    if (!dst || !a || !b || count == 0) return;

#if KX_HAVE_NEON_IMPL
    uint32_t i = 0;
    for (; i + 3 < count; i += 4) {
        float32x4x4_t va = vld4q_f32(reinterpret_cast<const float*>(&a[i]));
        float32x4x4_t vb = vld4q_f32(reinterpret_cast<const float*>(&b[i]));
        float32x4_t dot = vmulq_f32(va.val[0], vb.val[0]);
        dot = vfmaq_f32(dot, va.val[1], vb.val[1]);
        dot = vfmaq_f32(dot, va.val[2], vb.val[2]);
        dot = vfmaq_f32(dot, va.val[3], vb.val[3]);

        float32x2_t sum = vpadd_f32(vget_low_f32(dot), vget_high_f32(dot));
        sum = vpadd_f32(sum, sum);
        vst1_f32(&dst[i], sum);
        vst1_f32(&dst[i+1], sum);
        vst1_f32(&dst[i+2], sum);
        vst1_f32(&dst[i+3], sum);
    }
    for (; i < count; ++i) {
        dst[i] = a[i].x * b[i].x + a[i].y * b[i].y + a[i].z * b[i].z + a[i].w * b[i].w;
    }
#else
    for (uint32_t i = 0; i < count; ++i) {
        dst[i] = a[i].x * b[i].x + a[i].y * b[i].y + a[i].z * b[i].z + a[i].w * b[i].w;
    }
#endif
}

KX_API void KX_CALL kxNeonMatrixMul4x4(
    KxNeonMatrix4x4* dst,
    const KxNeonMatrix4x4* a,
    const KxNeonMatrix4x4* b,
    uint32_t count) {
    if (!dst || !a || !b || count == 0) return;

#if KX_HAVE_NEON_IMPL
    for (uint32_t m = 0; m < count; ++m) {
        float32x4_t rowsA[4];
        for (int r = 0; r < 4; ++r) {
            rowsA[r] = vld1q_f32(a[m].m[r]);
        }

        float32x4_t colsB[4];
        for (int c = 0; c < 4; ++c) {
            colsB[c] = vld1q_f32(&b[m].m[0][c]);
        }

        for (int r = 0; r < 4; ++r) {
            float32x4_t row = vdupq_n_f32(0.0f);
            for (int c = 0; c < 4; ++c) {
                row = vfmaq_f32(row, rowsA[c], vdupq_n_f32(a[m].m[r][c]));
            }
            vst1q_f32(dst[m].m[r], row);
        }
    }
#else
    for (uint32_t m = 0; m < count; ++m) {
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a[m].m[r][k] * b[m].m[k][c];
                }
                dst[m].m[r][c] = sum;
            }
        }
    }
#endif
}

KX_API void KX_CALL kxNeonSampleTextureBilinear(
    float* output,
    const float* texture,
    const float* uvs,
    uint32_t textureWidth,
    uint32_t textureHeight,
    uint32_t count) {
    (void)texture;
    (void)uvs;
    (void)textureWidth;
    (void)textureHeight;
    (void)count;
    /* Placeholder - full implementation requires more complex texture access patterns */
    if (output) {
        for (uint32_t i = 0; i < count * 4; ++i) output[i] = 0.0f;
    }
}

KX_API void KX_CALL kxNeonSampleTextureTrilinear(
    float* output,
    const float* texture,
    const float* uvs,
    uint32_t textureWidth,
    uint32_t textureHeight,
    uint32_t mipLevels,
    uint32_t count) {
    (void)texture;
    (void)uvs;
    (void)textureWidth;
    (void)textureHeight;
    (void)mipLevels;
    (void)count;
    if (output) {
        for (uint32_t i = 0; i < count * 4; ++i) output[i] = 0.0f;
    }
}

KX_API void KX_CALL kxNeonTransformVertices(
    KxNeonVector4* output,
    const KxNeonVector4* input,
    const KxNeonMatrix4x4* matrix,
    uint32_t count) {
    if (!output || !input || !matrix || count == 0) return;

#if KX_HAVE_NEON_IMPL
    for (uint32_t i = 0; i < count; ++i) {
        float32x4_t vec = vld1q_f32(reinterpret_cast<const float*>(&input[i]));
        float32x4_t mat0 = vld1q_f32(matrix->m[0]);
        float32x4_t mat1 = vld1q_f32(matrix->m[1]);
        float32x4_t mat2 = vld1q_f32(matrix->m[2]);
        float32x4_t mat3 = vld1q_f32(matrix->m[3]);

        float32x4_t res = vmulq_f32(mat0, vdupq_n_f32(input[i].x));
        res = vfmaq_f32(res, mat1, vdupq_n_f32(input[i].y));
        res = vfmaq_f32(res, mat2, vdupq_n_f32(input[i].z));
        res = vfmaq_f32(res, mat3, vdupq_n_f32(input[i].w));

        vst1q_f32(reinterpret_cast<float*>(&output[i]), res);
    }
#else
    for (uint32_t i = 0; i < count; ++i) {
        output[i].x = input[i].x * matrix->m[0][0] + input[i].y * matrix->m[1][0] + input[i].z * matrix->m[2][0] + input[i].w * matrix->m[3][0];
        output[i].y = input[i].x * matrix->m[0][1] + input[i].y * matrix->m[1][1] + input[i].z * matrix->m[2][1] + input[i].w * matrix->m[3][1];
        output[i].z = input[i].x * matrix->m[0][2] + input[i].y * matrix->m[1][2] + input[i].z * matrix->m[2][2] + input[i].w * matrix->m[3][2];
        output[i].w = input[i].x * matrix->m[0][3] + input[i].y * matrix->m[1][3] + input[i].z * matrix->m[2][3] + input[i].w * matrix->m[3][3];
    }
#endif
}

} /* extern "C" */