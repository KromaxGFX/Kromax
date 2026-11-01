/*
 * ShaderInterpreter.cpp
 *
 * Tree walking interpreter for the shader IR.
 */

#include "ShaderInterpreter.h"
#include "BuiltinMath.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Builtin functions - now using BuiltinMath.h
 * --------------------------------------------------------------------- */
namespace builtin = kx::shader::builtin;

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
    int stmt_count = 0;
    for (const auto& stmt : func.body) {
        if (++stmt_count > 1000) {
            return;  /* Prevent infinite loop */
        }
        execute_stmt(stmt);
    }

    if (result && !frame.empty()) {
        *result = frame;
    }

    stack_.pop_back();
    --call_depth_;
}

void Interpreter::execute_stmt(const Stmt& stmt) {
    if (++stmt_counter_ > 10000) return;
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
    int stmt_count = 0;
    for (const auto& stmt : stmts) {
        if (++stmt_count > 1000) return;
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
    if (++stmt_counter_ > 10000) return Value{};
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

    if (expr.builtin != UINT32_MAX && expr.builtin < program_.builtin_names.size()) {
        /* Builtin function - dispatch by name */
        const std::string& name = program_.builtin_names[expr.builtin];

        /* Evaluate all arguments first */
        std::vector<Value> args;
        for (std::size_t i = 1; i < expr.args.size(); ++i) {
            args.push_back(eval_expr(expr.args[i]));
        }

        Value result;

        /* Trigonometric */
        if (name == "sin") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::sin(args[0][0]));
            return result;
        }
        if (name == "cos") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::cos(args[0][0]));
            return result;
        }
        if (name == "tan") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::tan(args[0][0]));
            return result;
        }
        if (name == "asin") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::asin(args[0][0]));
            return result;
        }
        if (name == "acos") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::acos(args[0][0]));
            return result;
        }
        if (name == "atan") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::atan(args[0][0]));
            return result;
        }
        if (name == "sinh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::sinh(args[0][0]));
            return result;
        }
        if (name == "cosh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::cosh(args[0][0]));
            return result;
        }
        if (name == "tanh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::tanh(args[0][0]));
            return result;
        }
        if (name == "asinh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::asinh(args[0][0]));
            return result;
        }
        if (name == "acosh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::acosh(args[0][0]));
            return result;
        }
        if (name == "atanh") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::atanh(args[0][0]));
            return result;
        }

        /* Exponential/Logarithmic */
        if (name == "exp") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::exp(args[0][0]));
            return result;
        }
        if (name == "log") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::log(args[0][0]));
            return result;
        }
        if (name == "exp2") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::exp2(args[0][0]));
            return result;
        }
        if (name == "log2") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::log2(args[0][0]));
            return result;
        }
        if (name == "sqrt") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::sqrt(args[0][0]));
            return result;
        }
        if (name == "inversesqrt") {
            if (!args.empty() && !args[0].empty()) result.push_back(1.0f / std::sqrt(args[0][0]));
            return result;
        }
        if (name == "pow") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty())
                result.push_back(std::pow(args[0][0], args[1][0]));
            return result;
        }

        /* Rounding */
        if (name == "floor") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::floor(args[0][0]));
            return result;
        }
        if (name == "ceil") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::ceil(args[0][0]));
            return result;
        }
        if (name == "fract") {
            if (!args.empty() && !args[0].empty()) result.push_back(args[0][0] - std::floor(args[0][0]));
            return result;
        }
        if (name == "trunc") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::trunc(args[0][0]));
            return result;
        }
        if (name == "round") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::round(args[0][0]));
            return result;
        }
        if (name == "roundEven") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::nearbyint(args[0][0]));
            return result;
        }

        /* Vector */
        if (name == "dot") {
            if (args.size() >= 2) return Value{builtin::dot_vec(args[0], args[1])};
            return Value{};
        }
        if (name == "cross") {
            if (args.size() >= 2) return builtin::cross_vec(args[0], args[1]);
            return Value{};
        }
        if (name == "length") {
            if (!args.empty()) return Value{builtin::length_vec(args[0])};
            return Value{};
        }
        if (name == "distance") {
            if (args.size() >= 2) return Value{builtin::length_vec(args[0])}; // simplified
            return Value{};
        }
        if (name == "normalize") {
            if (!args.empty()) return builtin::normalize_vec(args[0]);
            return Value{};
        }
        if (name == "faceforward") {
            if (args.size() >= 3 && args[0].size() >= 3 && args[2].size() >= 3) {
                float dot = builtin::dot_vec(args[2], args[1]);
                Value r = args[0];
                if (dot < 0.0f) for (auto& v : r) v = -v;
                return r;
            }
            return args.empty() ? Value{} : args[0];
        }
        if (name == "reflect") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty()) {
                float dot = builtin::dot_vec(args[1], args[0]);
                Value r(args[0].size());
                for (std::size_t i = 0; i < r.size(); ++i) r[i] = args[0][i] - 2.0f * dot * args[1][i];
                return r;
            }
            return Value{};
        }
        if (name == "refract") {
            return args.empty() ? Value{} : args[0];
        }

        /* Matrix */
        if (name == "transpose") {
            if (!args.empty() && args[0].size() >= 4) {
                uint32_t cols = 2, rows = 2;
                if (args[0].size() == 9) cols = rows = 3;
                else if (args[0].size() == 16) cols = rows = 4;
                return builtin::transpose_mat(args[0], cols, rows);
            }
            return Value{};
        }
        if (name == "determinant") {
            if (!args.empty() && args[0].size() >= 4) {
                uint32_t size = 2;
                if (args[0].size() == 9) size = 3;
                else if (args[0].size() == 16) size = 4;
                return Value{builtin::determinant_mat(args[0], size)};
            }
            return Value{};
        }
        if (name == "inverse") {
            if (!args.empty() && args[0].size() >= 4) {
                uint32_t size = 2;
                if (args[0].size() == 9) size = 3;
                else if (args[0].size() == 16) size = 4;
                return builtin::inverse_mat(args[0], size);
            }
            return Value{};
        }

        /* Comparison */
        if (name == "lessThan") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av < bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "lessThanEqual") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av <= bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "greaterThan") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av > bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "greaterThanEqual") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av >= bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "equal") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av == bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "notEqual") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = av != bv ? 1.0f : 0.0f;
                }
                return r;
            }
            return Value{};
        }
        if (name == "any") {
            if (!args.empty()) {
                Value r(1);
                r[0] = 0.0f;
                for (float f : args[0]) if (f != 0.0f) { r[0] = 1.0f; break; }
                return r;
            }
            return Value{};
        }
        if (name == "all") {
            if (!args.empty()) {
                Value r(1);
                r[0] = 1.0f;
                for (float f : args[0]) if (f == 0.0f) { r[0] = 0.0f; break; }
                return r;
            }
            return Value{};
        }

        /* Selection */
        if (name == "mix") {
            if (args.size() >= 3) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float av = i < args[0].size() ? args[0][i] : 0.0f;
                    float bv = i < args[1].size() ? args[1][i] : 0.0f;
                    float tv = i < args[2].size() ? args[2][i] : (args[2].size() == 1 ? args[2][0] : 0.0f);
                    r[i] = builtin::mix_f(av, bv, tv);
                }
                return r;
            }
            return Value{};
        }
        if (name == "step") {
            if (args.size() >= 2) {
                Value r(std::max(args[0].size(), args[1].size()));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float ev = i < args[0].size() ? args[0][i] : 0.0f;
                    float xv = i < args[1].size() ? args[1][i] : 0.0f;
                    r[i] = builtin::step_f(ev, xv);
                }
                return r;
            }
            return Value{};
        }
        if (name == "smoothstep") {
            if (args.size() >= 3) {
                Value r(std::max({args[0].size(), args[1].size(), args[2].size()}));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float e0v = i < args[0].size() ? args[0][i] : 0.0f;
                    float e1v = i < args[1].size() ? args[1][i] : 0.0f;
                    float xv = i < args[2].size() ? args[2][i] : 0.0f;
                    r[i] = builtin::smoothstep_f(e0v, e1v, xv);
                }
                return r;
            }
            return Value{};
        }
        if (name == "clamp") {
            if (args.size() >= 3) {
                Value r(std::max({args[0].size(), args[1].size(), args[2].size()}));
                for (std::size_t i = 0; i < r.size(); ++i) {
                    float xv = i < args[0].size() ? args[0][i] : 0.0f;
                    float lov = i < args[1].size() ? args[1][i] : 0.0f;
                    float hiv = i < args[2].size() ? args[2][i] : 0.0f;
                    r[i] = builtin::clamp_f(xv, lov, hiv);
                }
                return r;
            }
            return Value{};
        }

        /* Derivative (simplified - return 0 for now, real impl needs fragment neighbors) */
        if (name == "dFdx") return Value{0.0f};
        if (name == "dFdy") return Value{0.0f};
        if (name == "fwidth") return Value{0.0f};

        /* Texture (simplified - returns 0) */
        if (name == "texture") return Value{0, 0, 0, 1};
        if (name == "textureSize") return Value{0, 0, 0};
        if (name == "textureLod") return Value{0, 0, 0, 1};
        if (name == "textureGrad") return Value{0, 0, 0, 1};
        if (name == "texelFetch") return Value{0, 0, 0, 1};
        if (name == "texelFetchOffset") return Value{0, 0, 0, 1};

        /* Atomic (simplified - not applicable in single-threaded interpreter) */
        if (name == "atomicAdd") return Value{0};
        if (name == "atomicMin") return Value{0};
        if (name == "atomicMax") return Value{0};
        if (name == "atomicAnd") return Value{0};
        if (name == "atomicOr") return Value{0};
        if (name == "atomicXor") return Value{0};
        if (name == "atomicExchange") return Value{0};
        if (name == "atomicCompSwap") return Value{0};

        /* Noise */
        if (name == "noise") {
            if (!args.empty()) {
                if (args[0].size() == 1) return Value{builtin::noise_1d(args[0][0])};
                if (args[0].size() == 2) return Value{builtin::noise_2d(args[0][0], args[0][1])};
                if (args[0].size() == 3) return Value{builtin::noise_3d(args[0][0], args[0][1], args[0][2])};
                if (args[0].size() == 4) return Value{builtin::noise_4d(args[0][0], args[0][1], args[0][2], args[0][3])};
            }
            return Value{0.0f};
        }

        /* Geometry */
        if (name == "radians") {
            if (!args.empty() && !args[0].empty()) result.push_back(args[0][0] * 0.017453292519943295f);
            return result;
        }
        if (name == "degrees") {
            if (!args.empty() && !args[0].empty()) result.push_back(args[0][0] * 57.29577951308232f);
            return result;
        }
        if (name == "sign") {
            if (!args.empty() && !args[0].empty()) result.push_back(static_cast<float>((args[0][0] > 0) - (args[0][0] < 0)));
            return result;
        }
        if (name == "abs") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::fabs(args[0][0]));
            return result;
        }
        if (name == "min") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty())
                result.push_back(std::min(args[0][0], args[1][0]));
            return result;
        }
        if (name == "max") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty())
                result.push_back(std::max(args[0][0], args[1][0]));
            return result;
        }
        if (name == "mod") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty())
                result.push_back(std::fmod(args[0][0], args[1][0]));
            return result;
        }
        if (name == "modf") {
            if (!args.empty() && !args[0].empty()) result.push_back(args[0][0] - std::trunc(args[0][0]));
            return result;
        }
        if (name == "fma") {
            if (args.size() >= 3 && !args[0].empty() && !args[1].empty() && !args[2].empty())
                result.push_back(args[0][0] * args[1][0] + args[2][0]);
            return result;
        }
        if (name == "frexp") return Value{0};
        if (name == "ldexp") {
            if (args.size() >= 2 && !args[0].empty() && !args[1].empty())
                result.push_back(std::ldexp(args[0][0], static_cast<int>(args[1][0])));
            return result;
        }
        if (name == "isnan") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::isnan(args[0][0]) ? 1.0f : 0.0f);
            return result;
        }
        if (name == "isinf") {
            if (!args.empty() && !args[0].empty()) result.push_back(std::isinf(args[0][0]) ? 1.0f : 0.0f);
            return result;
        }

        /* Bit packing */
        if (name == "floatBitsToInt") {
            if (!args.empty() && !args[0].empty()) result.push_back(static_cast<float>(*reinterpret_cast<int*>(&args[0][0])));
            return result;
        }
        if (name == "floatBitsToUint") {
            if (!args.empty() && !args[0].empty()) result.push_back(static_cast<float>(*reinterpret_cast<uint32_t*>(&args[0][0])));
            return result;
        }
        if (name == "intBitsToFloat") {
            if (!args.empty() && !args[0].empty()) {
                int i = static_cast<int>(args[0][0]);
                result.push_back(*reinterpret_cast<float*>(&i));
            }
            return result;
        }
        if (name == "uintBitsToFloat") {
            if (!args.empty() && !args[0].empty()) {
                uint32_t u = static_cast<uint32_t>(args[0][0]);
                result.push_back(*reinterpret_cast<float*>(&u));
            }
            return result;
        }

        if (name == "packSnorm2x16") {
            if (!args.empty()) return Value{builtin::pack_snorm2x16(args[0])};
            return Value{0};
        }
        if (name == "unpackSnorm2x16") {
            if (!args.empty() && !args[0].empty()) return builtin::unpack_snorm2x16(args[0][0]);
            return Value{0, 0};
        }
        if (name == "packUnorm2x16") {
            if (!args.empty()) return Value{builtin::pack_unorm2x16(args[0])};
            return Value{0};
        }
        if (name == "unpackUnorm2x16") {
            if (!args.empty() && !args[0].empty()) return builtin::unpack_unorm2x16(args[0][0]);
            return Value{0, 0};
        }
        if (name == "packHalf2x16") {
            if (!args.empty()) return Value{builtin::pack_half2x16(args[0])};
            return Value{0};
        }
        if (name == "unpackHalf2x16") {
            if (!args.empty() && !args[0].empty()) return builtin::unpack_half2x16(args[0][0]);
            return Value{0, 0};
        }

        /* Matrix extras */
        if (name == "outerProduct") {
            if (args.size() >= 2) return builtin::outer_product(args[0], args[1]);
            return Value{};
        }
        if (name == "matrixCompMult") {
            if (args.size() >= 2) return builtin::matrix_comp_mult(args[0], args[1]);
            return Value{};
        }

        /* Interpolation */
        if (name == "interpolateAtCentroid") return args.empty() ? Value{} : args[0];
        if (name == "interpolateAtSample") return args.empty() ? Value{} : args[0];
        if (name == "interpolateAtOffset") return args.empty() ? Value{} : args[0];

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

Value Interpreter::read_input(const Variable&) {
    Value v;
    /* Driver provides these */
    return v;
}

Value Interpreter::read_output(const Variable&) {
    Value v;
    return v;
}

void Interpreter::write_output(const Variable&, const Value&) {
    /* Driver handles this */
}

void Interpreter::write_uniform_block(const UniformBlockResource&, const UniformBlock&, const Value&) {
    /* Not typically done in shaders */
}

Value Interpreter::sample_texture(uint32_t texture_index, const Value& uv, uint32_t) {
    if (!env_ || texture_index >= env_->textures.size()) return Value{0, 0, 0, 1};

    const auto& tex = env_->textures[texture_index];
    if (!tex.texture || !tex.texture->pTexels) return Value{0, 0, 0, 1};

    /* Simple nearest neighbour for now */
    float u = uv.size() > 0 ? uv[0] : 0.0f;
    float v = uv.size() > 1 ? uv[1] : 0.0f;

    u = builtin::clamp_f(u, 0.0f, 1.0f);
    v = builtin::clamp_f(v, 0.0f, 1.0f);

    int ix = static_cast<int>(u * (static_cast<float>(tex.texture->width) - 1.0f));
    int iy = static_cast<int>(v * (static_cast<float>(tex.texture->height) - 1.0f));

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