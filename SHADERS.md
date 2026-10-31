# KromaxGFX Shader System

## Overview

KromaxGFX provides a **multi-language shader compiler** that accepts GLSL, HLSL, WGSL, and KXSL (native dialect) and lowers them to a single shared Intermediate Representation (IR). The IR is executed by a tree-walking interpreter at runtime — no SPIR-V backend required.

## Architecture

```
Source (GLSL/HLSL/WGSL/KXSL)
        │
        ▼
┌───────────────────────┐
│  Lexer + Preprocessor │  (handles #include, #define, #ifdef, etc.)
└───────────────────────┘
        │
        ▼
┌───────────────────────┐
│       Parser          │  (recursive descent, dialect-aware)
└───────────────────────┘
        │
        ▼
┌───────────────────────┐
│       IR Builder      │  (SSA-based, typed instructions)
└───────────────────────┘
        │
        ▼
┌───────────────────────┐
│    Shader Module      │  (holds IR + reflection metadata)
└───────────────────────┘
        │
        ▼
┌───────────────────────┐
│  Tree-Walking Interp. │  (CPU execution, no GPU backend)
└───────────────────────┘
```

## Supported Languages

| Language | Enum | Features |
|----------|------|----------|
| GLSL | `KX_SHADER_LANGUAGE_GLSL` | #version, layout qualifiers, push constants, uniforms |
| HLSL | `KX_SHADER_LANGUAGE_HLSL` | cbuffer, register bindings, SV_ semantics |
| WGSL | `KX_SHADER_LANGUAGE_WGSL` | @group/@binding, fn main(), structured types |
| KXSL | `KX_SHADER_LANGUAGE_KXSL` | Native dialect, GLSL-like without version/layouts |

## API Usage

### Compile from Source (C API)

```c
KxShaderSourceCreateInfo create_info = {
    .sType = KX_STRUCTURE_TYPE_SHADER_SOURCE_CREATE_INFO,
    .language = KX_SHADER_LANGUAGE_GLSL,
    .stage = KX_SHADER_STAGE_VERTEX,
    .pSource = shader_source_string,
    .pEntryPoint = "main",
    .includeCount = 0,
    .pIncludes = nullptr
};

KxShaderModule module;
KxResult res = kxCreateShaderModuleFromSource(device, &create_info, allocator, &module);
```

### Compile Offline (Tooling)

```c
KxShaderCompilationInfo comp_info = {
    .sType = KX_STRUCTURE_TYPE_SHADER_COMPILATION_INFO
};
KxResult res = kxCompileShader(&create_info, &comp_info);

// Check diagnostics
for (uint32_t i = 0; i < comp_info.diagnosticCount; ++i) {
    printf("%s:%u:%u: %s\n",
           comp_info.pDiagnostics[i].file,
           comp_info.pDiagnostics[i].line,
           comp_info.pDiagnostics[i].column,
           comp_info.pDiagnostics[i].message);
}
```

### Language Detection

```c
KxShaderLanguage lang = kxDetectShaderLanguage(source, source_len);
// Returns KX_SHADER_LANGUAGE_GLSL, HLSL, WGSL, KXSL, or UNDEFINED
```

### Get Language Name

```c
const char* name = kxGetShaderLanguageName(lang);  // "GLSL", "HLSL", etc.
```

## Shader Stages

- `KX_SHADER_STAGE_VERTEX` - Vertex shader
- `KX_SHADER_STAGE_FRAGMENT` - Fragment/pixel shader
- `KX_SHADER_STAGE_COMPUTE` - Compute shader

Multiple stages can be combined in a pipeline.

## Built-in Functions (60+)

### Math (12)
`abs`, `sign`, `floor`, `ceil`, `fract`, `mod`, `min`, `max`, `clamp`, `mix`, `step`, `smoothstep`

### Trigonometry (12)
`radians`, `degrees`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `atan2`

### Exponential (8)
`pow`, `exp`, `log`, `exp2`, `log2`, `sqrt`, `inversesqrt`, `rsqrt`

### Vector/Matrix (10)
`length`, `distance`, `dot`, `cross`, `normalize`, `faceforward`, `reflect`, `refract`, `determinant`, `inverse`, `transpose`

### Texture Sampling (8)
`texture`, `textureLod`, `textureGrad`, `textureSize`, `texelFetch`, `sampler` creation, `sampler2D` uniforms

### Atomic (4)
`atomicAdd`, `atomicMin`, `atomicMax`, `atomicAnd`, `atomicOr`, `atomicXor`, `atomicExchange`, `atomicCompSwap`

### Derivatives (4)
`dFdx`, `dFdy`, `fwidth`, `fwidthFine`

### Noise (2)
`noise1`, `noise2`, `noise3`, `noise4`

### Packing (8)
`packUnorm2x16`, `packUnorm4x8`, `packSnorm2x16`, `packSnorm4x8`, `packHalf2x16`, `unpackUnorm2x16`, `unpackUnorm4x8`, `unpackHalf2x16`

### Interpolation (4)
`interpolateAtCentroid`, `interpolateAtSample`, `interpolateAtOffset`, `flat` qualifier

## Uniform/Descriptor Layout

### Push Constants
```glsl
layout(push_constant) uniform PushConstants {
    mat4 mvp;
    float time;
} push;
```

### Descriptor Sets (Material System)
```
Set 0:
  Binding 0: uniform MaterialData { baseColor, metallic, roughness, ... }
  Binding 1: sampler2D baseColorTexture
  Binding 2: sampler2D normalTexture
  Binding 3: sampler2D metallicRoughnessTexture
  Binding 4: sampler2D emissiveTexture
  Binding 5: sampler2D occlusionTexture
```

## Reflection

```c
KxShaderCompilationInfo info = { .sType = KX_STRUCTURE_TYPE_SHADER_COMPILATION_INFO };
kxGetShaderModuleInfo(device, module, &info);

// Input attributes
for (i = 0; i < info.inputAttributeCount; ++i) {
    KxShaderInputAttribute attr = info.pInputAttributes[i];
    printf("Location %u: %s (format=%u)\n", attr.location, attr.name, attr.format);
}

// Push constant ranges
for (i = 0; i < info.pushConstantRangeCount; ++i) {
    KxPushConstantRange range = info.pPushConstantRanges[i];
    printf("Stage %u, offset %u, size %u\n", range.stageFlags, range.offset, range.size);
}
```

## Compilation Options

```c
typedef struct KxShaderCompileFlags {
    KX_SHADER_COMPILE_NONE = 0,
    KX_SHADER_COMPILE_DUMP_IR_BIT = 0x00000001,  // Print IR to stdout
    KX_SHADER_COMPILE_OPTIMIZE_BIT = 0x00000002,  // Enable optimizations
} KxShaderCompileFlags;
```

## Runtime Execution

The interpreter executes IR instructions directly on the CPU:

- **Vertex stage**: Runs per-vertex, outputs `gl_Position` + varyings
- **Fragment stage**: Runs per-fragment, outputs `gl_FragColor` + depth
- **Compute stage**: Runs per-workgroup, reads/writes storage buffers

The software rasterizer handles:
- Triangle setup and rasterization
- Attribute interpolation (perspective-correct)
- Depth testing and blending
- Dispatch for compute shaders

## Error Handling

All compilation functions return `KxResult`:
- `KX_SUCCESS` - OK
- `KX_ERROR_INVALID_ARGUMENT` - Bad create info
- `KX_ERROR_VALIDATION_FAILED_EXT` - Shader compilation failed (check diagnostics)

Use `kxCompileShader` or `kxGetShaderModuleInfo` to retrieve detailed diagnostics including line/column numbers.

## Examples

See `examples/05_shader_languages/` for a complete example compiling the same shader in GLSL, WGSL, and KXSL.

## Limitations

- No SPIR-V output (tree-walking interpreter only)
- Subset of GLSL 450 / HLSL 5.0 / WGSL spec
- No geometry/tessellation shaders
- No ray tracing
- Shader compilation is single-threaded