#ifndef KROMAX_SRC_SHADER_SHADERINTERPRETER_H
#define KROMAX_SRC_SHADER_SHADERINTERPRETER_H

/*
 * ShaderInterpreter.h
 *
 * Tree walking interpreter for the shader IR. Executes a compiled
 * ShaderProgram against the provided environment (uniforms, textures, samplers).
 */

#include <cstdint>
#include <string>
#include <vector>

#include "ShaderTypes.h"
#include "kx_core.h"
#include "ShaderIR.h"

namespace kx {
namespace shader {

// Forward declare the IR types we need
using ShaderProgram = kx::shader::ShaderProgram;
using UniformBlock = kx::shader::UniformBlock;
using Variable = kx::shader::Variable;
using StorageClass = kx::shader::StorageClass;

/* Forward declarations */
struct InterpreterContext;
struct BuiltinLibrary;

/* Runtime value - a flattened vector of scalars */
using Value = std::vector<float>;

/* -----------------------------------------------------------------------
 * ShaderEnvironment
 *
 * Runtime view of the resources available to a shader: uniform blocks,
 * textures and samplers. Filled by the driver before each draw/dispatch.
 * --------------------------------------------------------------------- */
struct TextureResource {
    const KxBoundTexture* texture = nullptr;
};

struct SamplerResource {
    KxSamplerState state{};
};

struct UniformBlockResource {
    const uint8_t* data = nullptr;
    uint32_t size = 0;
    uint32_t set = 0;
    uint32_t binding = 0;
};

struct ShaderEnvironment {
    std::vector<UniformBlockResource> uniform_blocks;   /* indexed by (set, binding) */
    std::vector<TextureResource> textures;
    std::vector<SamplerResource> samplers;
    const void* push_constants = nullptr;
    uint32_t push_constants_size = 0;

    const UniformBlockResource* get_uniform_block(uint32_t set, uint32_t binding) const noexcept {
        /* Linear search - small number of blocks */
        for (const auto& b : uniform_blocks) {
            if (b.set == set && b.binding == binding) return &b;
        }
        return nullptr;
    }
};

/* -----------------------------------------------------------------------
 * Interpreter
 * --------------------------------------------------------------------- */
class Interpreter {
public:
    explicit Interpreter(const ShaderProgram& program) : program_(program) {}

    /* Execute the entry function with the given environment.
     * For vertex shaders: inputs are in env.vertex_inputs, outputs in env.vertex_outputs.
     * For fragment shaders: inputs in env.fragment_inputs, outputs in env.fragment_outputs.
     * For compute shaders: inputs in env.compute_inputs.
     */
    void execute(const ShaderEnvironment& env, void* stage_input, void* stage_output);

    /* Execute a specific function by index */
    void execute_function(uint32_t func_index, const Value* args, Value* result);

    /* Access the current call stack depth */
    uint32_t call_depth() const noexcept { return call_depth_; }

private:
    const ShaderProgram& program_;
    const ShaderEnvironment* env_ = nullptr;
    uint32_t call_depth_ = 0;
    uint32_t current_function_ = UINT32_MAX;
    std::vector<Value> stack_;      /* call stack frames */

    void execute_stmt(const Stmt& stmt);
    void execute_block(const std::vector<Stmt>& stmts);
    void execute_declare(const Stmt& stmt);
    void execute_assign(const Stmt& stmt);
    void execute_if(const Stmt& stmt);
    void execute_for(const Stmt& stmt);
    void execute_while(const Stmt& stmt);
    void execute_return(const Stmt& stmt);
    void execute_switch(const Stmt& stmt);

    Value eval_expr(uint32_t expr_index);
    Value eval_literal(const Expr& expr);
    Value eval_varref(const Expr& expr);
    Value eval_construct(const Expr& expr);
    Value eval_unary(const Expr& expr);
    Value eval_binary(const Expr& expr);
    Value eval_ternary(const Expr& expr);
    Value eval_call(const Expr& expr);
    Value eval_member(const Expr& expr);
    Value eval_index(const Expr& expr);
    Value eval_swizzle(const Expr& expr);

    Value read_uniform_block(const UniformBlockResource& block,
                             const UniformBlock& ublock);
    Value read_push_constant(const Variable& var);
    Value read_input(const Variable& var);
    Value read_output(const Variable& var);

    void write_output(const Variable& var, const Value& value);
    void write_uniform_block(const UniformBlockResource& block,
                             const UniformBlock& ublock,
                             const Value& value);

    Value sample_texture(uint32_t texture_index, const Value& uv, uint32_t lod = 0);
    Value texture_size(uint32_t texture_index, uint32_t lod = 0);

    bool to_bool(const Value& v) const noexcept { return v.size() > 0 && v[0] != 0.0f; }
    float to_float(const Value& v) const noexcept { return v.size() > 0 ? v[0] : 0.0f; }
    int to_int(const Value& v) const noexcept { return v.size() > 0 ? static_cast<int>(v[0]) : 0; }
    uint32_t to_uint(const Value& v) const noexcept { return v.size() > 0 ? static_cast<uint32_t>(v[0]) : 0u; }
};

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERINTERPRETER_H */