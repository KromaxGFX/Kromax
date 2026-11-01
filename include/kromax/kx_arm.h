#ifndef KX_ARM_H
#define KX_ARM_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * ARM64/NEON Support Extension (KX_EXT_arm64)
 * ======================================================================= */

#define KX_EXT_ARM64_EXTENSION_NAME "KX_EXT_arm64"
#define KX_EXT_ARM64_SPEC_VERSION 1

/* ARM64 CPU features */
typedef uint32_t KxArmCpuFeatures;
#define KX_ARM_CPU_FEATURE_NONE              0u
#define KX_ARM_CPU_FEATURE_NEON              0x00000001u
#define KX_ARM_CPU_FEATURE_FP16_ARITHMETIC   0x00000002u
#define KX_ARM_CPU_FEATURE_DOTPROD           0x00000004u
#define KX_ARM_CPU_FEATURE_BF16              0x00000008u
#define KX_ARM_CPU_FEATURE_I8MM              0x00000010u
#define KX_ARM_CPU_FEATURE_SVE               0x00000020u
#define KX_ARM_CPU_FEATURE_SVE2              0x00000040u
#define KX_ARM_CPU_FEATURE_MAX_ENUM          0x7FFFFFFFu

/* ARM64 CPU info - defined in kx_structs.h */
/* typedef struct KxArmCpuInfo {
    KxStructureType sType;
    void* pNext;
    KxArmCpuFeatures features;
    uint32_t implementer;
    uint32_t variant;
    uint32_t part;
    uint32_t revision;
    uint32_t cpuid;
    char name[64];
} KxArmCpuInfo; */

/* NEON vector types for shader compatibility */
typedef struct KxNeonVector4 {
    float x, y, z, w;
} KxNeonVector4;

typedef struct KxNeonVector3 {
    float x, y, z;
} KxNeonVector3;

typedef struct KxNeonVector2 {
    float x, y;
} KxNeonVector2;

typedef struct KxNeonMatrix4x4 {
    float m[4][4];
} KxNeonMatrix4x4;

/* =======================================================================
 * ARM64 functions
 * ======================================================================= */

/* Get ARM64 CPU features */
KX_API KxResult KX_CALL kxGetArmCpuFeatures(
    KxArmCpuFeatures* pFeatures);

/* Get detailed ARM64 CPU info */
KX_API KxResult KX_CALL kxGetArmCpuInfo(
    KxArmCpuInfo* pInfo);

/* NEON-accelerated math functions */
KX_API void KX_CALL kxNeonVectorAdd(
    KxNeonVector4* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count);

KX_API void KX_CALL kxNeonVectorMul(
    KxNeonVector4* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count);

KX_API void KX_CALL kxNeonVectorDot(
    float* dst,
    const KxNeonVector4* a,
    const KxNeonVector4* b,
    uint32_t count);

KX_API void KX_CALL kxNeonMatrixMul4x4(
    KxNeonMatrix4x4* dst,
    const KxNeonMatrix4x4* a,
    const KxNeonMatrix4x4* b,
    uint32_t count);

/* SIMD-optimized texture sampling */
KX_API void KX_CALL kxNeonSampleTextureBilinear(
    float* output,
    const float* texture,
    const float* uvs,
    uint32_t textureWidth,
    uint32_t textureHeight,
    uint32_t count);

KX_API void KX_CALL kxNeonSampleTextureTrilinear(
    float* output,
    const float* texture,
    const float* uvs,
    uint32_t textureWidth,
    uint32_t textureHeight,
    uint32_t mipLevels,
    uint32_t count);

/* NEON-accelerated vertex transform */
KX_API void KX_CALL kxNeonTransformVertices(
    KxNeonVector4* output,
    const KxNeonVector4* input,
    const KxNeonMatrix4x4* matrix,
    uint32_t count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_ARM_H */