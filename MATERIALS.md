# KromaxGFX Material System

## Overview

The material system provides a high-level abstraction for PBR (Physically Based Rendering) materials. It handles:

- **Material parameters** (base color, metallic, roughness, emissive, etc.)
- **Texture slots** (6 standard PBR textures)
- **Descriptor set publishing** (automatic GPU resource binding)
- **Built-in PBR shader** (works without writing shader code)

## Material Structure

```
KxMaterial
├── Parameters (uniform buffer)
│   ├── baseColor[4]          // RGBA
│   ├── metallic              // 0.0 - 1.0
│   ├── roughness             // 0.0 - 1.0
│   ├── emissive[3]           // RGB
│   ├── normalScale           // 0.0 - 1.0
│   ├── occlusionStrength     // 0.0 - 1.0
│   ├── alphaCutoff           // Alpha test threshold
│   └── alphaMode             // OPAQUE/MASK/BLEND
├── Texture Slots (6)
│   ├── BASE_COLOR (albedo)
│   ├── NORMAL
│   ├── METALLIC_ROUGHNESS
│   ├── EMISSIVE
│   ├── OCCLUSION
│   └── HEIGHT (parallax)
└── Descriptor Set (published to GPU)
```

## Creating Materials

### Default PBR Material (Built-in Shader)

```c
KxMaterial material;
KxResult res = kxCreateDefaultMaterial(device, allocator, &material);
// Creates material with:
// - baseColor = {1,1,1,1} (white)
// - metallic = 0.0
// - roughness = 1.0
// - built-in PBR shader
```

### Custom Material

```c
KxMaterialCreateInfo create_info = {
    .sType = KX_STRUCTURE_TYPE_MATERIAL_CREATE_INFO,
    .shaderModule = custom_shader_module,  // NULL = built-in PBR
    .baseColor = {0.8f, 0.6f, 0.4f, 1.0f},
    .metallic = 0.2f,
    .roughness = 0.5f,
    .emissive = {0.0f, 0.0f, 0.0f},
    .normalScale = 1.0f,
    .occlusionStrength = 1.0f,
    .alphaCutoff = 0.5f,
    .textureCount = 0,
    .pTextures = nullptr
};

KxMaterial material;
res = kxCreateMaterial(device, &create_info, allocator, &material);
```

## Texture Slots

| Slot | Enum | Description | Channels |
|------|------|-------------|----------|
| Base Color | `KX_MATERIAL_TEXTURE_BASE_COLOR` | Albedo/diffuse | RGB + A |
| Normal | `KX_MATERIAL_TEXTURE_NORMAL` | Tangent-space normals | RGB |
| Metallic/Roughness | `KX_MATERIAL_TEXTURE_METALLIC_ROUGHNESS` | R=Metallic, G=Roughness | RG |
| Emissive | `KX_MATERIAL_TEXTURE_EMISSIVE` | Self-illumination | RGB |
| Occlusion | `KX_MATERIAL_TEXTURE_OCCLUSION` | Ambient occlusion | R |
| Height | `KX_MATERIAL_TEXTURE_HEIGHT` | Parallax/displacement | R |

### Setting Textures

```c
// Set texture with custom sampler
KxResult res = kxMaterialSetTexture(device, material,
    KX_MATERIAL_TEXTURE_BASE_COLOR,
    albedo_image_view,
    albedo_sampler);

// Set texture with default sampler (pass NULL for sampler)
res = kxMaterialSetTexture(device, material,
    KX_MATERIAL_TEXTURE_NORMAL,
    normal_image_view,
    nullptr);

// Clear texture slot
res = kxMaterialSetTexture(device, material,
    KX_MATERIAL_TEXTURE_EMISSIVE,
    nullptr,
    nullptr);
```

### Querying Textures

```c
KxImageView view;
KxSampler sampler;
res = kxMaterialGetTexture(device, material,
    KX_MATERIAL_TEXTURE_BASE_COLOR,
    &view, &sampler);
```

## Material Parameters

### Set Parameter

```c
// Scalar parameters
float metallic = 0.8f;
kxMaterialSetParameter(device, material, "metallic", &metallic, sizeof(float));

float roughness = 0.3f;
kxMaterialSetParameter(device, material, "roughness", &roughness, sizeof(float));

// Vector parameters
float base_color[4] = {1.0f, 0.5f, 0.2f, 1.0f};
kxMaterialSetParameter(device, material, "baseColor", base_color, sizeof(base_color));

float emissive[3] = {1.0f, 0.5f, 0.0f};
kxMaterialSetParameter(device, material, "emissive", emissive, sizeof(emissive));
```

### Supported Parameter Names

| Name | Type | Size | Default | Range |
|------|------|------|---------|-------|
| `baseColor` | float[4] | 16 bytes | {1,1,1,1} | [0,1] |
| `metallic` | float | 4 bytes | 0.0 | [0,1] |
| `roughness` | float | 4 bytes | 1.0 | [0,1] |
| `emissive` | float[3] | 12 bytes | {0,0,0} | [0,∞] |
| `normalScale` | float | 4 bytes | 1.0 | [0,1] |
| `occlusionStrength` | float | 4 bytes | 1.0 | [0,1] |
| `alphaCutoff` | float | 4 bytes | 0.5 | [0,1] |
| `alphaMode` | uint32_t | 4 bytes | 0 (OPAQUE) | 0,1,2 |

### Get Parameter

```c
float get_metallic;
kxMaterialGetParameter(device, material, "metallic", &get_metallic, sizeof(float));

float get_base_color[4];
kxMaterialGetParameter(device, material, "baseColor", get_base_color, sizeof(get_base_color));
```

## Descriptor Set Publishing

The material system publishes resources to a `KxDescriptorSet` that can be bound to a pipeline.

### Required Descriptor Set Layout

```c
// Binding 0: Uniform buffer (material parameters)
VkDescriptorSetLayoutBinding bindings[6] = {
    {0, UNIFORM_BUFFER, 1, VERTEX | FRAGMENT},           // MaterialData
    {1, COMBINED_IMAGE_SAMPLER, 1, FRAGMENT},            // BaseColor
    {2, COMBINED_IMAGE_SAMPLER, 1, FRAGMENT},            // Normal
    {3, COMBINED_IMAGE_SAMPLER, 1, FRAGMENT},            // MetallicRoughness
    {4, COMBINED_IMAGE_SAMPLER, 1, FRAGMENT},            // Emissive
    {5, COMBINED_IMAGE_SAMPLER, 1, FRAGMENT},            // Occlusion
};
```

### Update Descriptor Set

```c
// Allocate descriptor set from pool with matching layout
KxDescriptorSet descriptor_set;
// ... allocate from pool ...

// Publish material resources (firstBinding = 0 for standard layout)
res = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 0);
```

### Custom Binding Offset

```c
// If your layout uses different base bindings
res = kxMaterialUpdateDescriptorSet(device, material, descriptor_set, 3);
// Textures go to bindings 4,5,6,7,8 (3 + slot enum value)
```

## Rendering with Materials

### Pipeline Creation

```c
// Pipeline layout must include the material's descriptor set layout
KxPipelineLayoutCreateInfo layout_info = {
    .sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .setLayoutCount = 1,
    .pSetLayouts = &material_descriptor_set_layout,
    .pushConstantRangeCount = 1,
    .pPushConstantRanges = &(KxPushConstantRange){
        .stageFlags = KX_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(Mat4)  // MVP matrix
    }
};

KxPipelineLayout pipeline_layout;
kxCreatePipelineLayout(device, &layout_info, nullptr, &pipeline_layout);
```

### Command Buffer Recording

```c
// Begin render pass...
kxCmdBeginRenderPass(cmd, &rp_begin, KX_SUBPASS_CONTENTS_INLINE);

// Bind pipeline (uses built-in PBR shader if material created with NULL shaderModule)
kxCmdBindPipeline(cmd, KX_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

// Bind material descriptor set
kxCmdBindDescriptors(cmd, KX_PIPELINE_BIND_POINT_GRAPHICS,
                     pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

// Bind vertex buffers
kxCmdBindVertexBuffers(cmd, 0, 1, &vertex_buffer, &offset);

// Push MVP matrix
kxCmdPushConstants(cmd, pipeline_layout, KX_SHADER_STAGE_VERTEX_BIT,
                   0, sizeof(Mat4), &mvp);

// Draw
kxCmdDraw(cmd, vertex_count, 1, 0, 0);

kxCmdEndRenderPass(cmd);
```

## Built-in PBR Shader

The built-in shader (`src/shader/pbr_shader.glsl`) implements:

- **Metallic-Roughness workflow** (glTF 2.0 compatible)
- **Image-Based Lighting (IBL)** support (prefiltered environment + BRDF LUT)
- **Normal mapping** with configurable scale
- **Emissive** textures with strength factor
- **Occlusion** mapping
- **Alpha modes**: OPAQUE, MASK (alpha test), BLEND
- **Vertex attributes**: position, normal, tangent, texcoord, color

### Camera Uniforms (Set 0, Binding 0)

```glsl
layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
    vec3 camera_pos;
    float _pad0;
};
```

### Material Uniforms (Set 0, Binding 0 in material layout)

```glsl
layout(set = 0, binding = 0) uniform MaterialData {
    vec4 base_color_factor;      // baseColorFactor
    vec3 emissive_factor;        // emissiveFactor
    float metallic_factor;       // metallicFactor
    float roughness_factor;      // roughnessFactor
    float normal_scale;          // normalTexture scale
    float occlusion_strength;    // occlusionTexture strength
    float alpha_cutoff;          // alphaCutoff
    float alpha_mode;            // 0=OPAQUE, 1=MASK, 2=BLEND
    int  ibl_enabled;            // bool
    float _pad1[2];
};
```

### Texture Bindings (Set 1 in built-in shader)

```glsl
layout(set = 1, binding = 0) uniform sampler2D baseColorTexture;
layout(set = 1, binding = 1) uniform sampler2D normalTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D emissiveTexture;
layout(set = 1, binding = 4) uniform sampler2D occlusionTexture;
```

## Complete Example

See `examples/06_material_pbr/` for a complete working example demonstrating:

1. Creating default material
2. Creating procedural textures for all 5 slots
3. Setting material parameters
4. Publishing to descriptor set
5. Rendering with animation loop

## Best Practices

1. **Reuse descriptor pools** - Create once, allocate many sets
2. **Batch material updates** - Call `kxMaterialUpdateDescriptorSet` once per frame after all changes
3. **Use default sampler** - Pass `NULL` for sampler to use device default
4. **Match descriptor layout** - Pipeline layout must match material expectations
5. **Clean up in reverse order** - Destroy material before descriptor pool/set layout

## Error Handling

- `kxMaterialSetParameter` returns `KX_ERROR_INVALID_ARGUMENT` for unknown names
- `kxMaterialUpdateDescriptorSet` returns error if descriptor set layout doesn't match
- All functions return `KX_SUCCESS` on success