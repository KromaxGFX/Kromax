/*
 * ShaderInterpreter.cpp
 *
 * Tree walking interpreter for the shader IR.
 */

#include "ShaderInterpreter.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Builtin functions
 * --------------------------------------------------------------------- */
namespace builtin {

inline float clamp_f(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float lerp_f(float a, float b, float t) {
    return a + t * (b - a);
}

inline float smoothstep_f(float edge0, float edge1, float x) {
    float t = clamp_f((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

inline float step_f(float edge, float x) {
    return x < edge ? 0.0f : 1.0f;
}

inline float fract_f(float x) {
    return x - std::floor(x);
}

inline float mix_f(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

/* Swizzle a vector */
Value swizzle_vec(const Value& v, const uint8_t* indices, uint8_t count) {
    Value result;
    result.reserve(count);
    for (uint8_t i = 0; i < count; ++i) {
        result.push_back(indices[i] < v.size() ? v[indices[i]] : 0.0f);
    }
    return result;
}

/* Dot product */
float dot_vec(const Value& a, const Value& b) {
    float sum = 0.0f;
    std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) sum += a[i] * b[i];
    return sum;
}

/* Cross product (3D only) */
Value cross_vec(const Value& a, const Value& b) {
    Value result(3);
    if (a.size() >= 3 && b.size() >= 3) {
        result[0] = a[1] * b[2] - a[2] * b[1];
        result[1] = a[2] * b[0] - a[0] * b[2];
        result[2] = a[0] * b[1] - a[1] * b[0];
    }
    return result;
}

/* Length */
float length_vec(const Value& v) {
    return std::sqrt(dot_vec(v, v));
}

/* Normalize */
Value normalize_vec(const Value& v) {
    float len = length_vec(v);
    if (len == 0.0f) return v;
    Value result(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) result[i] = v[i] / len;
    return result;
}

/* Matrix * Vector (column major matrix) */
Value mat_vec_mul(const Value& mat, const Value& vec, uint32_t cols, uint32_t rows) {
    Value result(rows);
    for (uint32_t r = 0; r < rows; ++r) {
        float sum = 0.0f;
        for (uint32_t c = 0; c < cols; ++c) {
            sum += mat[c * rows + r] * (c < vec.size() ? vec[c] : (c == 3 ? 1.0f : 0.0f));
        }
        result[r] = sum;
    }
    return result;
}

/* Matrix * Matrix */
Value mat_mul(const Value& a, const Value& b, uint32_t a_cols, uint32_t a_rows, uint32_t b_cols) {
    Value result(a_cols * b_cols);
    for (uint32_t col = 0; col < a_cols; ++col) {
        for (uint32_t row = 0; row < b_cols; ++row) {
            float sum = 0.0f;
            for (uint32_t k = 0; k < a_rows; ++k) {
                sum += a[k * a_cols + col] * b[row * b_cols + k];
            }
            result[row * a_cols + col] = sum;
        }
    }
    return result;
}

/* Transpose */
Value transpose_mat(const Value& mat, uint32_t cols, uint32_t rows) {
    Value result(rows * cols);
    for (uint32_t c = 0; c < cols; ++c) {
        for (uint32_t r = 0; r < rows; ++r) {
            result[c * rows + r] = mat[r * cols + c];
        }
    }
    return result;
}

} /* namespace builtin */

/* -----------------------------------------------------------------------
 * Interpreter
 * --------------------------------------------------------------------- */
void Interpreter::execute(const ShaderEnvironment& env, void* stage_input, void* stage_output) {
    env_ = &env;

    if (program_.entry == UINT32_MAX) return;

    const auto& entry_func = program_.functions[program_.entry];

    /* Prepare arguments from stage input */
    std::vector<Value> args;
    for (const auto& param : entry_func.params) {
        args.push_back(Value{}); /* Filled by caller via env */
    }

    Value result;
    execute_function(program_.entry, args.data(), &result);

    /* Copy outputs to stage output */
    for (uint32_t i = 0; i < entry_func.params.size(); ++i) {
        const auto& param = entry_func.params[i];
        if (param.storage == StorageClass::Output) {
            /* Deferred - the driver handles this */
        }
    }
}

void Interpreter::execute_function(uint32_t func_index, const Value* args, Value* result) {
    if (func_index >= program_.functions.size()) return;

    const auto& func = program_.functions[func_index];
    ++call_depth_;

    /* Push new frame */
    stack_.emplace_back();
    Value& frame = stack_.back();

    /* Copy arguments */
    if (args) {
        for (std::size_t i = 0; i < func.params.size(); ++i) {
            frame.insert(frame.end(), args[i].begin(), args[i].end());
        }
    }

    /* Execute body */
    for (const auto& stmt : func.body) {
        execute_stmt(stmt);
    }

    if (result && !frame.empty()) {
        *result = frame;
    }

    stack_.pop_back();
    --call_depth_;
}

void Interpreter::execute_stmt(const Stmt& stmt) {
    switch (stmt.kind) {
        case StmtKind::Block:
            execute_block(stmt.body);
            break;
        case StmtKind::Declare:
            execute_declare(stmt);
            break;
        case StmtKind::Assign:
            execute_assign(stmt);
            break;
        case StmtKind::Expression:
            eval_expr(stmt.expr);
            break;
        case StmtKind::If:
            execute_if(stmt);
            break;
        case StmtKind::For:
            execute_for(stmt);
            break;
        case StmtKind::While:
            execute_while(stmt);
            break;
        case StmtKind::Return:
            execute_return(stmt);
            break;
        case StmtKind::Switch:
            execute_switch(stmt);
            break;
        default:
            break;
    }
}

void Interpreter::execute_block(const std::vector<Stmt>& stmts) {
    for (const auto& stmt : stmts) {
        execute_stmt(stmt);
    }
}

void Interpreter::execute_declare(const Stmt& stmt) {
    /* Declaration handled by the variable being in locals/globals */
    if (stmt.expr != UINT32_MAX) {
        Value val = eval_expr(stmt.expr);
        /* Store in frame */
    }
}

void Interpreter::execute_assign(const Stmt& stmt) {
    Value rhs = eval_expr(stmt.expr2);
    /* LHS handling would need a proper variable resolver */
    (void)rhs;
}

void Interpreter::execute_if(const Stmt& stmt) {
    Value cond = eval_expr(stmt.expr);
    if (to_bool(cond)) {
        execute_block(stmt.body);
    } else if (!stmt.else_body.empty()) {
        execute_block(stmt.else_body);
    }
}

void Interpreter::execute_for(const Stmt& stmt) {
    execute_block(stmt.init);
    while (to_bool(eval_expr(stmt.expr))) {
        execute_block(stmt.body);
        execute_block(stmt.increment);
    }
}

void Interpreter::execute_while(const Stmt& stmt) {
    while (to_bool(eval_expr(stmt.expr))) {
        execute_block(stmt.body);
    }
}

void Interpreter::execute_return(const Stmt& stmt) {
    if (stmt.expr != UINT32_MAX) {
        Value val = eval_expr(stmt.expr);
        if (!stack_.empty()) {
            stack_.back() = std::move(val);
        }
    }
    /* Return is implemented by unwinding - simplified here */
}

void Interpreter::execute_switch(const Stmt& stmt) {
    Value val = eval_expr(stmt.expr);
    float fval = to_float(val);

    for (std::size_t i = 0; i < stmt.case_values.size(); ++i) {
        if (stmt.case_values[i] == UINT32_MAX ||
            (stmt.case_values[i] < program_.exprs.size() &&
             std::abs(to_float(eval_expr(stmt.case_values[i])) - fval) < 1e-5f)) {
            execute_block(stmt.cases[i]);
            break;
        }
    }
}

Value Interpreter::eval_expr(uint32_t expr_index) {
    if (expr_index >= program_.exprs.size()) return Value{};
    const Expr& expr = program_.exprs[expr_index];

    switch (expr.kind) {
        case ExprKind::Literal: return eval_literal(expr);
        case ExprKind::VarRef: return eval_varref(expr);
        case ExprKind::Construct: return eval_construct(expr);
        case ExprKind::Unary: return eval_unary(expr);
        case ExprKind::Binary: return eval_binary(expr);
        case ExprKind::Ternary: return eval_ternary(expr);
        case ExprKind::Call: return eval_call(expr);
        case ExprKind::Member: return eval_member(expr);
        case ExprKind::Index: return eval_index(expr);
        case ExprKind::Swizzle: return eval_swizzle(expr);
        case ExprKind::Assign: return eval_expr(expr.args[1]); /* Assignment returns RHS */
    }
    return Value{};
}

Value Interpreter::eval_literal(const Expr& expr) {
    Value v;
    if (expr.type.is_scalar()) {
        v.push_back(expr.fconst);
    } else if (expr.type.is_vector()) {
        v.push_back(expr.fconst);
    }
    return v;
}

Value Interpreter::eval_varref(const Expr& expr) {
    Value v;
    if (expr.local != UINT32_MAX && current_function_ != UINT32_MAX) {
        /* Local variable - value in stack frame */
    } else if (expr.var != UINT32_MAX && expr.var < program_.globals.size()) {
        const auto& var = program_.globals[expr.var];
        switch (var.storage) {
            case StorageClass::Uniform:
                if (var.block != UINT32_MAX && var.block < program_.blocks.size() && env_) {
                    const auto& block_res = env_->uniform_blocks[var.block];
                    if (block_res.data) {
                        v.resize(var.type.component_count());
                        std::memcpy(v.data(), block_res.data + var.offset, var.type.byte_size());
                    }
                }
                break;
            case StorageClass::PushConstant:
                if (env_ && env_->push_constants) {
                    v.resize(var.type.component_count());
                    std::memcpy(v.data(),
                                static_cast<const uint8_t*>(env_->push_constants) + var.offset,
                                var.type.byte_size());
                }
                break;
            case StorageClass::Input:
                /* Handled by driver */
                break;
            case StorageClass::Output:
                /* Handled by driver */
                break;
            default:
                break;
        }
    }
    return v;
}

Value Interpreter::eval_construct(const Expr& expr) {
    Value result;
    result.resize(expr.type.component_count());

    std::size_t dst = 0;
    for (uint32_t arg_idx : expr.args) {
        Value arg_val = eval_expr(arg_idx);
        for (float f : arg_val) {
            if (dst < result.size()) result[dst++] = f;
            else break;
        }
        if (dst >= result.size()) break;
    }
    return result;
}

Value Interpreter::eval_unary(const Expr& expr) {
    Value operand = eval_expr(expr.args[0]);
    Value result(operand.size());

    switch (expr.unop) {
        case UnaryOp::Negate:
            for (std::size_t i = 0; i < operand.size(); ++i) result[i] = -operand[i];
            break;
        case UnaryOp::LogicalNot:
            for (std::size_t i = 0; i < operand.size(); ++i) result[i] = !operand[i];
            break;
        case UnaryOp::BitNot:
            for (std::size_t i = 0; i < operand.size(); ++i)
                result[i] = static_cast<float>(~static_cast<int>(operand[i]));
            break;
        case UnaryOp::Plus:
            result = operand;
            break;
        case UnaryOp::PreIncrement:
            for (std::size_t i = 0; i < operand.size(); ++i) result[i] = operand[i] + 1.0f;
            break;
        case UnaryOp::PreDecrement:
            for (std::size_t i = 0; i < operand.size(); ++i) result[i] = operand[i] - 1.0f;
            break;
        case UnaryOp::PostIncrement:
        case UnaryOp::PostDecrement:
            result = operand;
            break;
    }
    return result;
}

Value Interpreter::eval_binary(const Expr& expr) {
    Value lhs = eval_expr(expr.args[0]);
    Value rhs = eval_expr(expr.args[1]);
    Value result(std::max(lhs.size(), rhs.size()));

    auto promote = [&](float a, float b, float(*op)(float, float)) -> float {
        return op(a, b);
    };

    for (std::size_t i = 0; i < result.size(); ++i) {
        float a = i < lhs.size() ? lhs[i] : 0.0f;
        float b = i < rhs.size() ? rhs[i] : 0.0f;
        switch (expr.binop) {
            case BinaryOp::Add: result[i] = a + b; break;
            case BinaryOp::Subtract: result[i] = a - b; break;
            case BinaryOp::Multiply: result[i] = a * b; break;
            case BinaryOp::Divide: result[i] = b != 0.0f ? a / b : 0.0f; break;
            case BinaryOp::Modulo: result[i] = std::fmod(a, b); break;
            case BinaryOp::Less: result[i] = a < b ? 1.0f : 0.0f; break;
            case BinaryOp::LessEqual: result[i] = a <= b ? 1.0f : 0.0f; break;
            case BinaryOp::Greater: result[i] = a > b ? 1.0f : 0.0f; break;
            case BinaryOp::GreaterEqual: result[i] = a >= b ? 1.0f : 0.0f; break;
            case BinaryOp::Equal: result[i] = a == b ? 1.0f : 0.0f; break;
            case BinaryOp::NotEqual: result[i] = a != b ? 1.0f : 0.0f; break;
            case BinaryOp::LogicalAnd: result[i] = (a != 0.0f && b != 0.0f) ? 1.0f : 0.0f; break;
            case BinaryOp::LogicalOr: result[i] = (a != 0.0f || b != 0.0f) ? 1.0f : 0.0f; break;
            case BinaryOp::BitAnd: result[i] = static_cast<float>(static_cast<int>(a) & static_cast<int>(b)); break;
            case BinaryOp::BitOr: result[i] = static_cast<float>(static_cast<int>(a) | static_cast<int>(b)); break;
            case BinaryOp::BitXor: result[i] = static_cast<float>(static_cast<int>(a) ^ static_cast<int>(b)); break;
            case BinaryOp::ShiftLeft: result[i] = static_cast<float>(static_cast<int>(a) << static_cast<int>(b)); break;
            case BinaryOp::ShiftRight: result[i] = static_cast<float>(static_cast<int>(a) >> static_cast<int>(b)); break;
        }
    }
    return result;
}

Value Interpreter::eval_ternary(const Expr& expr) {
    Value cond = eval_expr(expr.args[0]);
    if (to_bool(cond)) {
        return eval_expr(expr.args[1]);
    } else {
        return eval_expr(expr.args[2]);
    }
}

Value Interpreter::eval_call(const Expr& expr) {
    if (expr.args.empty()) return Value{};

    uint32_t callee = expr.args[0];
    if (callee >= program_.exprs.size()) return Value{};

    const Expr& callee_expr = program_.exprs[callee];

    if (expr.builtin != UINT32_MAX) {
        /* Builtin function - dispatch by name */
        const std::string& name = ""; /* Would need to store builtin name */

        /* For now, handle based on first argument count */
        Value result;
        if (expr.args.size() == 2) {
            Value arg = eval_expr(expr.args[1]);
            if (arg.size() >= 1) {
                float x = arg[0];
                /* Single argument math functions */
                result.push_back(std::sin(x)); /* placeholder */
            }
        }
        return result;
    }

    if (expr.func != UINT32_MAX) {
        /* User function */
        std::vector<Value> args;
        for (std::size_t i = 1; i < expr.args.size(); ++i) {
            args.push_back(eval_expr(expr.args[i]));
        }
        Value result;
        execute_function(expr.func, args.data(), &result);
        return result;
    }

    return Value{};
}

Value Interpreter::eval_member(const Expr& expr) {
    Value base = eval_expr(expr.args[0]);
    /* Struct member access - would need member offset lookup */
    return base;
}

Value Interpreter::eval_index(const Expr& expr) {
    Value base = eval_expr(expr.args[0]);
    Value index_val = eval_expr(expr.args[1]);
    uint32_t idx = to_uint(index_val);

    if (base.size() > idx) {
        return Value{base[idx]};
    }
    return Value{0.0f};
}

Value Interpreter::eval_swizzle(const Expr& expr) {
    Value base = eval_expr(expr.args[0]);
    return builtin::swizzle_vec(base, expr.swizzle, expr.swizzle_count);
}

Value Interpreter::read_uniform_block(const UniformBlockResource& block,
                                      const UniformBlock& ublock) {
    Value v;
    if (block.data && block.size > 0) {
        v.resize(ublock.size / 4);
        std::memcpy(v.data(), block.data, block.size);
    }
    return v;
}

Value Interpreter::read_push_constant(const Variable& var) {
    Value v;
    if (env_ && env_->push_constants) {
        v.resize(var.type.component_count());
        std::memcpy(v.data(),
                    static_cast<const uint8_t*>(env_->push_constants) + var.offset,
                    var.type.byte_size());
    }
    return v;
}

Value Interpreter::read_input(const Variable& var) {
    Value v;
    /* Driver provides these */
    return v;
}

Value Interpreter::read_output(const Variable& var) {
    Value v;
    return v;
}

void Interpreter::write_output(const Variable& var, const Value& value) {
    /* Driver handles this */
}

void Interpreter::write_uniform_block(const UniformBlockResource& block,
                                      const UniformBlock& ublock,
                                      const Value& value) {
    /* Not typically done in shaders */
}

Value Interpreter::sample_texture(uint32_t texture_index, const Value& uv, uint32_t lod) {
    if (!env_ || texture_index >= env_->textures.size()) return Value{0, 0, 0, 1};

    const auto& tex = env_->textures[texture_index];
    if (!tex.texture || !tex.texture->pTexels) return Value{0, 0, 0, 1};

    /* Simple nearest neighbour for now */
    float u = uv.size() > 0 ? uv[0] : 0.0f;
    float v = uv.size() > 1 ? uv[1] : 0.0f;

    u = builtin::clamp_f(u, 0.0f, 1.0f);
    v = builtin::clamp_f(v, 0.0f, 1.0f);

    int ix = static_cast<int>(u * (tex.texture->width - 1));
    int iy = static_cast<int>(v * (tex.texture->height - 1));

    const uint8_t* texels = static_cast<const uint8_t*>(tex.texture->pTexels);
    const uint8_t* pixel = texels + iy * tex.texture->rowPitch + ix * 4;

    Value result(4);
    result[0] = pixel[0] / 255.0f;
    result[1] = pixel[1] / 255.0f;
    result[2] = pixel[2] / 255.0f;
    result[3] = pixel[3] / 255.0f;

    return result;
}

Value Interpreter::texture_size(uint32_t texture_index, uint32_t lod) {
    if (!env_ || texture_index >= env_->textures.size()) return Value{0, 0, 0};
    const auto& tex = env_->textures[texture_index];
    if (!tex.texture) return Value{0, 0, 0};
    return Value{static_cast<float>(tex.texture->width >> lod),
                 static_cast<float>(tex.texture->height >> lod),
                 static_cast<float>(tex.texture->arrayLayers)};
}

} /* namespace shader */
} /* namespace kx */