# KromaxGFX: Multi-Language Shader System & Software Rasterizer

A Vulkan-style graphics API with a multi-language shader compiler (GLSL/HLSL/WGSL/MSL → shared IR), software rasterizer, and PBR material system. Pure C11 public API, C++17 internal implementation.

## Status

**Active Development** - Sprints 1-5 complete (Phases A-F). All 17 unit tests passing. Examples 00 and 03 run on MSYS2/MinGW64 Windows.

| Sprint | Focus | Status |
|--------|-------|--------|
| 1 | Shader Compiler (Lexer, Preprocessor, Parser, IR, Compiler) | ✅ Done |
| 2 | Shader Runtime (Interpreter, 60+ Builtins) | ✅ Done |
| 3 | Driver Integration (Descriptor resolution, Compute dispatch) | ✅ Done |
| 4 | Textures & Samplers (Bilinear/trilinear, mipmaps, wrap modes, sRGB) | ✅ Done |
| 5 | Materials (PBR, 6 texture slots, descriptor sets) + **All Tests Pass** | ✅ Done |
| 6 | Integration Tests (Material+Texture+Sampler) | 🔄 Next |
| 7 | Example Updates (New features) | ⏳ Planned |
| 8 | Documentation & Validation | ⏳ Planned |

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
./bin/kromax_example_00_enumerate.exe   # Enumerates software device
./bin/kromax_example_03_triangle.exe    # 3D rotating triangle
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
examples/             # 00_enumerate, 01_clear, 02_kms, 03_triangle, 04_compute
```

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
# All 17 test suites
ctest --output-on-failure

# Individual test
./bin/test_shader_compiler.exe
./bin/test_sampler_filtering.exe
./bin/test_material_lifecycle.exe
```

## License

Apache License 2.0 — see [LICENSE](LICENSE)