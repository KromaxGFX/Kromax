# KromaxGFX: Multi-Language Shader System & Software Rasterizer

A Vulkan-style graphics API with a multi-language shader compiler (GLSL/HLSL/WGSL/MSL → shared IR), software rasterizer, and PBR material system. Pure C11 public API, C++17 internal implementation.

## Status

**All Core Features Complete** - 20/20 tests passing (8 existing + 9 shader unit tests + 3 integration tests). Examples 00 and 03 run on MSYS2/MinGW64 Windows.

| Component | Tests | Status |
|-----------|-------|--------|
| Shader Lexer | 4 | ✅ Passing |
| Shader Parser | 4 | ✅ Passing |
| Shader IR | 2 | ✅ Passing |
| Shader Interpreter | 3 | ✅ Passing |
| Shader Builtins (60+) | 3 | ✅ Passing |
| Shader Compiler | 7 | ✅ Passing |
| Sampler Filtering | 3 | ✅ Passing |
| Mipmap Generation | 2 | ✅ Passing |
| Material Lifecycle | 2 | ✅ Passing |
| API Tests | 3 | ✅ Passing |
| Integration (enumerate) | 1 | ✅ Passing |
| Integration (triangle offscreen) | 1 | ✅ Passing |
| Integration (shader languages) | 1 | ✅ Passing |
| Integration (material PBR) | 1 | ✅ Passing |
| Integration (compute shader) | 1 | ✅ Passing |

## Features

- **C11 Public API** — Stable ABI, no C++ in headers
- **C++17 Internal** — Modern implementation, no exceptions/RTTI at boundary
- **Multi-Language Shaders** — Single unified parser with dialect config for GLSL, HLSL, WGSL, MSL
- **Shared IR** — Tree-walking interpreter (no SPIR-V backend needed)
- **60+ Builtins** — Math, vector/matrix, texture sampling, atomic, derivatives, noise, packing, interpolation
- **Software Rasterizer** — Vertex/fragment/compute execution on CPU
- **Texture Pipeline** — Bilinear/trilinear filtering, mip selection via derivatives, all wrap modes, sRGB decode, mipmap generation
- **Material System** — 6 texture slots (albedo, normal, metallic/roughness/AO, emissive), descriptor set publishing, built-in PBR shader
- **Windows Native** — Builds on MSYS2/MinGW64, no Linux dependencies

## Requirements

- **MSYS2/MinGW64** (Windows) — `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake ninja`
- **CMake ≥ 3.24**
- **Ninja** (preferred) or Make

## Quick Start

```bash
# 1. Configure (one-time)
cd KromaxGFX
export PATH="/c/msys64/mingw64/bin:/c/msys64/usr/bin:$PATH"
cmake --preset debug

# 2. Build
cmake --build --preset debug

# 3. Run tests
ctest --output-on-failure

# 4. Run examples
./bin/kromax_example_00_enumerate.exe     # Enumerates software device
./bin/kromax_example_03_triangle.exe      # 3D rotating triangle
./bin/kromax_example_05_shader_languages.exe  # GLSL/WGSL/KXSL compilation demo
./bin/kromax_example_06_material_pbr.exe      # PBR material with textures demo
```

## Project Structure

```
include/kromax/       # Public C11 API headers
src/
  core/               # Instance, Device, Queue, Memory, Commands
  driver_software/    # Software rasterizer + shader module
  shader/             # Compiler + Runtime (Phases B-C)
  material/           # Material system (Phase F)
tests/unit/           # 17 test binaries (doctest)
tests/integration/    # 3 integration tests (shader langs, material, compute)
examples/             # 00_enumerate, 01_clear, 02_kms, 03_triangle, 04_compute, 05_shader_languages, 06_material_pbr
```

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                        C11 Public API (include/kromax)              │
├─────────────────────────────────────────────────────────────────────┤
│  kxCreateInstance  kxCreateDevice  kxCreateShaderModuleFromSource  │
│  kxCreateMaterial  kxCreatePipeline  kxCmdDraw  kxCmdDispatch      │
└─────────────────────────────────────────────────────────────────────┘
                                │
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     C++17 Internal Implementation                   │
├─────────────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │
│  │   Shader     │  │   Material   │  │   Driver     │              │
│  │  Compiler    │  │   System     │  │  (Software)  │              │
│  │              │  │              │  │              │              │
│  │ • Lexer      │  │ • 6 slots    │  │ • Rasterizer │              │
│  │ • Parser     │  │ • Params     │  │ • Interpreter│              │
│  │ • IR Builder │  │ • Descriptor │  │ • Memory     │              │
│  │ • Interpreter│  │   publishing │  │   management │              │
│  │ • 60+Builtins│  │ • PBR shader │  │ • Descriptor │              │
│  └──────────────┘  └──────────────┘  └──────────────┘              │
└─────────────────────────────────────────────────────────────────────┘
```

## Shader Compilation Pipeline

```text
Source (GLSL/HLSL/WGSL/KXSL)
        │
        ▼
┌───────────────────┐
│  Lexer +          │  → Tokens
│  Preprocessor     │     (#include, #define, #ifdef)
└───────────────────┘
        │
        ▼
┌───────────────────┐
│  Parser           │  → AST
│  (recursive       │     (dialect-aware)
│   descent)        │
└───────────────────┘
        │
        ▼
┌───────────────────┐
│  IR Builder       │  → SSA IR
│  (typed instr.)   │     + reflection metadata
└───────────────────┘
        │
        ▼
┌───────────────────┐
│  Shader Module    │  → Runtime object
│  (holds IR +      │     (tree-walking
│   metadata)       │      interpreter)
└───────────────────┘
```

## Documentation

- [SHADERS.md](SHADERS.md) — Shader system: languages, IR, builtins, API, reflection
- [MATERIALS.md](MATERIALS.md) — Material system: PBR workflow, texture slots, parameters, descriptor sets

## Shader Language Support

```c
// Compile any supported language
KxShaderCompiler compiler = kxCreateShaderCompiler(...);
KxShaderProgram prog = kxShaderCompilerCompile(compiler, source, &options);
// options.language = KX_SHADER_LANGUAGE_GLSL | HLSL | WGSL | MSL
```

## Material Usage

```c
KxMaterial mat = kxCreateMaterial(device, &(KxMaterialCreateInfo){
    .pipeline = pipeline,
    .setLayout = descriptorSetLayout,
    .baseColorFactor = {1,1,1,1},
    .metallicFactor = 0.5f,
    .roughnessFactor = 0.5f,
});
kxMaterialSetTexture(mat, KX_MATERIAL_TEXTURE_SLOT_ALBEDO, textureView, sampler);
kxMaterialUpdateDescriptorSet(mat);  // Publishes to GPU
```

## Building from Source

```bash
# Windows (MSYS2)
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake ninja git
git clone https://github.com/your-org/KromaxGFX
cd KromaxGFX
export PATH="/c/msys64/mingw64/bin:/c/msys64/usr/bin:$PATH"
cmake --preset debug
cmake --build --preset debug
ctest --output-on-failure
```

## Testing

```bash
# All 20 test suites
ctest --output-on-failure

# Individual test
./bin/test_shader_compiler.exe
./bin/test_sampler_filtering.exe
./bin/test_material_lifecycle.exe
```

## License

Apache License 2.0 — see [LICENSE](LICENSE)

## See Also

- [SHADERS.md](SHADERS.md) — Shader system deep dive
- [MATERIALS.md](MATERIALS.md) — Material system reference