#ifndef KX_SHADER_H
#define KX_SHADER_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Shader compilation
 *
 * Kromax accepts several shader authoring languages and lowers all of them
 * onto a single internal representation. The same pipeline, descriptor and
 * material code therefore works with GLSL, HLSL, WGSL or the native KXSL
 * dialect.
 * ======================================================================= */

typedef uint32_t KxShaderCompileFlags;
#define KX_SHADER_COMPILE_NONE 0u
#define KX_SHADER_COMPILE_DUMP_IR_BIT 0x00000001u
#define KX_SHADER_COMPILE_OPTIMIZE_BIT 0x00000002u
#define KX_SHADER_COMPILE_MAX_ENUM 0x7FFFFFFFu

/* A pre-resolved include. */
typedef struct KxShaderInclude {
    const char* pName;       /* include name as written in the source */
    const char* pSource;     /* resolved contents                     */
    size_t sourceSize;       /* length of pSource in bytes            */
} KxShaderInclude;

/* Include resolver. Return KX_TRUE when the include was resolved. */
typedef KxBool32 (KX_CALL* KxShaderIncludeCallback)(
    void* pUserData,
    const char* pName,
    KxShaderInclude* pOutInclude);

/* Source shader description. */
typedef struct KxShaderSourceCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxShaderLanguage language;      /* KX_SHADER_LANGUAGE_UNDEFINED = auto detect */
    KxShaderStage stage;            /* hint; ignored when the source declares it */
    const char* pSource;
    size_t sourceSize;              /* 0 = NUL terminated                         */
    const char* pEntryPoint;        /* entry point name; NULL = "main"          */
    uint32_t includeCount;
    const KxShaderInclude* pIncludes;
    KxShaderIncludeCallback pfnInclude;
    void* pIncludeUserData;
    const char* pDefines;           /* "NAME=VALUE\nNAME2\n" style defines      */
    KxShaderCompileFlags flags;
} KxShaderSourceCreateInfo;

/* A single compiler message. */
typedef struct KxShaderDiagnostic {
    KxShaderSeverity severity;
    uint32_t line;                  /* 1 based */
    uint32_t column;                /* 1 based */
    uint32_t offset;                /* byte offset in the source */
    char message[256];
} KxShaderDiagnostic;

/* Result of a compilation. */
typedef struct KxShaderCompilationInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t diagnosticCount;
    const KxShaderDiagnostic* pDiagnostics;
    uint32_t irSize;                /* length of pIr in bytes (0 if not dumped) */
    const char* pIr;                /* textual representation of the IR        */
} KxShaderCompilationInfo;

/* Compile a shader from source and create a shader module. */
KX_API KxResult KX_CALL kxCreateShaderModuleFromSource(
    KxDevice device,
    const KxShaderSourceCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxShaderModule* pShaderModule);

/* Compile a shader without creating a device object. Useful for tooling and
 * for validating shaders before device creation. */
KX_API KxResult KX_CALL kxCompileShader(
    const KxShaderSourceCreateInfo* pCreateInfo,
    KxShaderCompilationInfo* pCompilationInfo);

/* Retrieve the diagnostics recorded while a shader module was compiled. */
KX_API KxResult KX_CALL kxGetShaderModuleInfo(
    KxDevice device,
    KxShaderModule shaderModule,
    KxShaderCompilationInfo* pCompilationInfo);

/* Human readable language name, e.g. "GLSL". Never NULL. */
KX_API const char* KX_CALL kxGetShaderLanguageName(KxShaderLanguage language);

/* Guess the source language from the contents of the source. Returns
 * KX_SHADER_LANGUAGE_UNDEFINED when the language cannot be determined. */
KX_API KxShaderLanguage KX_CALL kxDetectShaderLanguage(
    const char* pSource,
    size_t sourceSize);

/* =======================================================================
 * Shader environment
 *
 * A shader environment carries the resources a shader can read: uniform
 * blocks, sampled textures and samplers. Environments can be bound to a
 * command buffer, in which case they are merged with the resources coming
 * from the bound descriptor sets (environment entries take precedence).
 * ======================================================================= */

KX_API KxResult KX_CALL kxCreateShaderEnvironment(
    KxDevice device,
    const KxAllocationCallbacks* pAllocator,
    KxShaderEnvironment* pEnvironment);

KX_API void KX_CALL kxDestroyShaderEnvironment(
    KxDevice device,
    KxShaderEnvironment environment,
    const KxAllocationCallbacks* pAllocator);

/* Publish a uniform block at (set, binding). The data is copied. */
KX_API KxResult KX_CALL kxShaderEnvironmentSetUniform(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    const void* pData,
    size_t size);

/* Publish a sampled texture (and optional sampler) at (set, binding). */
KX_API KxResult KX_CALL kxShaderEnvironmentSetTexture(
    KxDevice device,
    KxShaderEnvironment environment,
    uint32_t set,
    uint32_t binding,
    KxImageView imageView,
    KxSampler sampler);

/* Bind an environment to a command buffer. */
KX_API void KX_CALL kxCmdBindShaderEnvironment(
    KxCommandBuffer commandBuffer,
    KxShaderEnvironment environment);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_SHADER_H */
