// PBR Shader (GLSL) - Vertex + Fragment
// Metallic-Roughness workflow with IBL support
// Designed to be compiled at runtime by ShaderCompiler

#version 450

// ============================================================================
// Uniforms / Resources
// ============================================================================

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
    vec3 camera_pos;
    float _pad0;
};

layout(set = 0, binding = 1) uniform MaterialData {
    vec4 base_color_factor;      // baseColorFactor (r, g, b, a)
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

layout(set = 1, binding = 0) uniform sampler2D base_color_texture;
layout(set = 1, binding = 1) uniform sampler2D metallic_roughness_texture;
layout(set = 1, binding = 2) uniform sampler2D normal_texture;
layout(set = 1, binding = 3) uniform sampler2D occlusion_texture;
layout(set = 1, binding = 4) uniform sampler2D emissive_texture;

// IBL textures
layout(set = 2, binding = 0) uniform samplerCube irradiance_map;
layout(set = 2, binding = 1) uniform samplerCube prefiltered_map;
layout(set = 2, binding = 2) uniform sampler2D brdf_lut;

// ============================================================================
// Vertex Inputs / Outputs
// ============================================================================

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in vec3 in_tangent;
layout(location = 4) in vec4 in_color;

layout(location = 0) out vec3 out_world_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_texcoord;
layout(location = 3) out vec3 out_tangent;
layout(location = 4) out vec3 out_bitangent;
layout(location = 5) out vec3 out_view_dir;

// ============================================================================
// Vertex Shader
// ============================================================================

void main_vertex() {
    vec4 world_pos = vec4(in_position, 1.0);
    vec4 clip_pos = view_proj * world_pos;
    
    gl_Position = clip_pos;
    
    out_world_pos = world_pos.xyz;
    out_normal = normalize(mat3(view) * in_normal);
    out_texcoord = in_texcoord;
    out_tangent = normalize(mat3(view) * in_tangent);
    out_bitangent = cross(out_normal, out_tangent);
    out_view_dir = normalize(camera_pos - out_world_pos);
}

// ============================================================================
// Fragment Shader - PBR Functions
// ============================================================================

// Schlick's approximation for Fresnel
vec3 fresnel_schlick(float cos_theta, vec3 f0) {
    return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float distribution_ggx(vec3 n, vec3 h, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float n_dot_h = max(dot(n, h), 0.0);
    float n_dot_h2 = n_dot_h * n_dot_h;
    float denom = n_dot_h2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / denom;
}

// Geometry Function (Schlick-GGX)
float geometry_schlick_ggx(float n_dot_v, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float denom = n_dot_v * (1.0 - k) + k;
    return n_dot_v / denom;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness) {
    float n_dot_v = max(dot(n, v), 0.0);
    float n_dot_l = max(dot(n, l), 0.0);
    float ggx1 = geometry_schlick_ggx(n_dot_v, roughness);
    float ggx2 = geometry_schlick_ggx(n_dot_l, roughness);
    return ggx1 * ggx2;
}

// ============================================================================
// IBL Functions
// ============================================================================

vec3 get_ibl_diffuse(vec3 n) {
    return texture(irradiance_map, n).rgb;
}

vec3 get_ibl_specular(vec3 n, vec3 v, float roughness, vec3 f0) {
    // Sample prefiltered environment map
    const int MAX_MIP_LEVELS = 5;
    vec3 prefiltered_color = textureLod(prefiltered_map, n, roughness * MAX_MIP_LEVELS).rgb;
    
    // Sample BRDF LUT
    vec2 brdf = texture(brdf_lut, vec2(max(dot(n, v), 0.0), roughness)).xy;
    
    return prefiltered_color * (f0 * brdf.x + brdf.y);
}

// ============================================================================
// Fragment Shader Main
// ============================================================================

layout(location = 0) out vec4 frag_color;

void main_fragment() {
    // Normal mapping
    vec3 normal = out_normal;
    if (normal_scale > 0.0) {
        vec3 tangent_normal = texture(normal_texture, out_texcoord).xyz * 2.0 - 1.0;
        vec3 t = normalize(out_tangent);
        vec3 b = out_bitangent;
        mat3 tbn = mat3(t, b, normal);
        normal = normalize(tbn * tangent_normal * normal_scale);
    }
    
    // Material properties from textures
    vec4 base_color_tex = texture(base_color_texture, out_texcoord);
    vec3 base_color = base_color_factor.rgb * base_color_tex.rgb * in_color.rgb;
    float alpha = base_color_factor.a * base_color_tex.a * in_color.a;
    
    vec3 metallic_roughness_tex = texture(metallic_roughness_texture, out_texcoord).rgb;
    float metallic = metallic_factor * metallic_roughness_tex.b;
    float roughness = roughness_factor * metallic_roughness_tex.g;
    
    float occlusion = texture(occlusion_texture, out_texcoord).r;
    occlusion = 1.0 - occlusion_strength * (1.0 - occlusion);
    
    vec3 emissive = emissive_factor * texture(emissive_texture, out_texcoord).rgb;
    
    // Alpha cutoff
    if (alpha_mode == 1 && alpha < alpha_cutoff) {
        discard;
    }
    
    // PBR calculations
    vec3 n = normalize(normal);
    vec3 v = normalize(out_view_dir);
    
    // F0 calculation
    vec3 dielectric_f0 = vec3(0.04);
    vec3 f0 = mix(dielectric_f0, base_color, metallic);
    
    // Image-based lighting
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    
    if (ibl_enabled == 1) {
        diffuse = get_ibl_diffuse(n) * base_color * (1.0 - metallic);
        specular = get_ibl_specular(n, v, roughness, f0);
    }
    
    // Ambient
    const vec3 ambient = vec3(0.03) * base_color * occlusion;
    
    // Combine
    vec3 color = ambient + diffuse + specular + emissive;
    
    // Tone mapping (simple Reinhard)
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    frag_color = vec4(color, alpha);
}

// ============================================================================
// Entry Points
// ============================================================================

void main() {
    // This shader uses explicit entry points - vertex and fragment
    // The compiler will use the appropriate entry point based on stage
}

// ============================================================================
// Constants
// ============================================================================

#define PI 3.14159265359