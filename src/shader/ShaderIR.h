#ifndef KROMAX_SRC_SHADER_SHADERIR_H
#define KROMAX_SRC_SHADER_SHADERIR_H

/*
 * ShaderIR.h
 *
 * Language independent intermediate representation for Kromax shaders.
 *
 * Every front-end (GLSL, HLSL, WGSL, KXSL) lowers its source onto this IR and
 * a single interpreter executes it. The IR is intentionally close to the source
 * language: a typed expression tree plus a flat statement tree. It is stored in
 * arenas (expressions and statements are referenced by index) so that executing
 * a shader does not allocate.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "ShaderTypes.h"
#include "kx_core.h"

namespace kx {
namespace shader {

/* -----------------------------------------------------------------------
 * Storage classes
 * --------------------------------------------------------------------- */
enum class StorageClass : uint8_t {
    Local,         /* function local variable                   */
    Constant,      /* compile time constant                     */
    Uniform,       /* uniform block member                      */
    PushConstant,  /* push constant block member                */
    Input,         /* stage input (vertex attribute / varying)  */
    Output,        /* stage output (varying / render target)    */
    Texture,       /* sampled image resource                    */
    Sampler,       /* sampler resource                          */
    StorageBuffer, /* read/write buffer resource                */
    UniformBuffer, /* read only buffer resource                 */
};

/* -----------------------------------------------------------------------
 * Declarations
 * --------------------------------------------------------------------- */
struct StructMember {
    std::string name;
    ShaderType type;
    uint32_t offset = 0;   /* byte offset inside the struct */
};

struct StructDef {
    std::string name;
    std::vector<StructMember> members;
    uint32_t size = 0;
};

struct Variable {
    std::string name;
    ShaderType type;
    StorageClass storage = StorageClass::Local;
    uint32_t set = 0;
    uint32_t binding = 0;
    uint32_t location = UINT32_MAX;
    uint32_t offset = 0;        /* byte offset inside a uniform block */
    uint32_t size = 0;          /* byte size                         */
    uint32_t block = UINT32_MAX;/* index into ShaderProgram::blocks   */
    uint32_t member = UINT32_MAX; /* struct member index              */
    bool array = false;
    uint32_t array_size = 0;
};

struct UniformBlock {
    std::string name;
    uint32_t set = 0;
    uint32_t binding = 0;
    std::vector<Variable> members;
    uint32_t size = 0;
    bool push_constant = false;
};

/* -----------------------------------------------------------------------
 * Expressions
 * --------------------------------------------------------------------- */
enum class ExprKind : uint8_t {
    Literal,
    VarRef,
    Swizzle,
    Member,
    Index,
    Unary,
    Binary,
    Ternary,
    Call,        /* builtin or user function */
    Construct,   /* type constructor */
    Assign,
};

enum class UnaryOp : uint8_t {
    Negate,
    LogicalNot,
    BitNot,
    Plus,
    PreIncrement,
    PreDecrement,
    PostIncrement,
    PostDecrement,
};

enum class BinaryOp : uint8_t {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    LogicalAnd,
    LogicalOr,
    BitAnd,
    BitOr,
    BitXor,
    ShiftLeft,
    ShiftRight,
};

struct Expr {
    ExprKind kind = ExprKind::Literal;
    ShaderType type;
    UnaryOp unop = UnaryOp::Negate;
    BinaryOp binop = BinaryOp::Add;

    float fconst = 0.0f;
    int32_t iconst = 0;
    uint32_t uconst = 0;
    bool bconst = false;

    uint32_t var = UINT32_MAX;     /* VarRef: global variable index */
    uint32_t local = UINT32_MAX;   /* VarRef: local variable index  */
    uint32_t func = UINT32_MAX;    /* Call: user function index     */
    uint32_t builtin = UINT32_MAX; /* Call: builtin id              */
    uint32_t member = UINT32_MAX;  /* Member: struct member index   */

    uint8_t swizzle[4] = {0, 0, 0, 0};
    uint8_t swizzle_count = 0;

    std::vector<uint32_t> args;    /* expression indices */

    uint32_t line = 0;
};

/* -----------------------------------------------------------------------
 * Statements
 * --------------------------------------------------------------------- */
enum class StmtKind : uint8_t {
    Block,
    Declare,
    Assign,
    Expression,
    If,
    For,
    While,
    Return,
    Break,
    Continue,
    Discard,
    Switch,
};

struct Stmt {
    StmtKind kind = StmtKind::Block;
    uint32_t expr = UINT32_MAX;   /* primary expression                */
    uint32_t expr2 = UINT32_MAX;  /* secondary expression (rhs / cond) */
    uint32_t expr3 = UINT32_MAX;  /* tertiary expression (index)       */
    uint32_t var = UINT32_MAX;    /* Declare: variable index           */
    uint32_t line = 0;

    std::vector<Stmt> body;
    std::vector<Stmt> else_body;

    std::vector<Stmt> init;        /* for-loop initialiser */
    std::vector<Stmt> increment;   /* for-loop increment  */

    /* switch */
    std::vector<std::vector<Stmt>> cases;
    std::vector<uint32_t> case_values; /* expression index, UINT32_MAX = default */
};

/* -----------------------------------------------------------------------
 * Functions
 * --------------------------------------------------------------------- */
struct Function {
    std::string name;
    ShaderType return_type;
    std::vector<Variable> params;
    std::vector<Variable> locals;  /* function scope variables */
    std::vector<Stmt> body;
    bool is_entry = false;
    bool builtin = false;           /* implemented by the runtime */
    uint32_t line = 0;
};

/* -----------------------------------------------------------------------
 * Program
 * --------------------------------------------------------------------- */
struct ShaderProgram {
    KxShaderStage stage = KX_SHADER_STAGE_VERTEX;
    KxShaderLanguage language = KX_SHADER_LANGUAGE_UNDEFINED;
    std::string entry_name = "main";

    std::vector<StructDef> structs;
    std::vector<UniformBlock> blocks;
    std::vector<Variable> globals;   /* uniforms, inputs, outputs, resources */
    std::vector<Function> functions;
    std::vector<Expr> exprs;
    std::vector<std::string> builtin_names;  /* Maps builtin ID -> name */

    uint32_t entry = UINT32_MAX;

    /* Helpers */
    const Function* entry_function() const noexcept {
        return entry < functions.size() ? &functions[entry] : nullptr;
    }

    const UniformBlock* find_block(uint32_t set, uint32_t binding) const noexcept {
        for (const auto& b : blocks) {
            if (b.set == set && b.binding == binding) return &b;
        }
        return nullptr;
    }

    const Variable* find_global(const std::string& name) const noexcept {
        for (const auto& g : globals) {
            if (g.name == name) return &g;
        }
        return nullptr;
    }
};

/* Serialise a program to a human readable form (used by diagnostics and by
 * the KX_SHADER_COMPILE_DUMP_IR_BIT compile flag). */
std::string dump_program(const ShaderProgram& program);

} /* namespace shader */
} /* namespace kx */

#endif /* KROMAX_SRC_SHADER_SHADERIR_H */
