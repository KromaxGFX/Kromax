/*
 * test_shader_ir.cpp
 * Unit tests for shader IR construction and serialization
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include "shader/ShaderIR.h"
#include "shader/ShaderTypes.h"

using namespace kx::shader;

TEST_CASE("ShaderIR - TypeSystem") {
    ShaderType t_void = make_void();
    CHECK_EQ(t_void.cls, TypeClass::Void);
    
    ShaderType t_bool = make_bool();
    CHECK_EQ(t_bool.cls, TypeClass::Bool);
    
    ShaderType t_int = make_int();
    CHECK_EQ(t_int.cls, TypeClass::Int);
    
    ShaderType t_uint = make_uint();
    CHECK_EQ(t_uint.cls, TypeClass::UInt);
    
    ShaderType t_float = make_float();
    CHECK_EQ(t_float.cls, TypeClass::Float);
    
    ShaderType t_vec2 = make_vector(2);
    CHECK_EQ(t_vec2.cls, TypeClass::Vector);
    CHECK_EQ(t_vec2.rows, 2);
    CHECK_EQ(t_vec2.component_count(), 2);
    
    ShaderType t_vec3 = make_vector(3);
    CHECK_EQ(t_vec3.component_count(), 3);
    
    ShaderType t_vec4 = make_vector(4);
    CHECK_EQ(t_vec4.component_count(), 4);
    
    ShaderType t_mat2 = make_matrix(2, 2);
    CHECK_EQ(t_mat2.cls, TypeClass::Matrix);
    CHECK_EQ(t_mat2.rows, 2);
    CHECK_EQ(t_mat2.cols, 2);
    CHECK_EQ(t_mat2.component_count(), 4);
    
    ShaderType t_mat4 = make_matrix(4, 4);
    CHECK_EQ(t_mat4.component_count(), 16);
}

TEST_CASE("ShaderIR - TypePromotion") {
    ShaderType t_float = make_float();
    ShaderType t_int = make_int();
    ShaderType t_vec3 = make_vector(3);
    t_vec3.cls = TypeClass::Float;
    ShaderType t_ivec3 = make_vector(3);
    t_ivec3.cls = TypeClass::Int;
    
    ShaderType result1 = promote(t_float, t_int);
    CHECK_EQ(result1.cls, TypeClass::Float);
    
    ShaderType result2 = promote(t_vec3, t_int);
    CHECK_EQ(result2.cls, TypeClass::Vector);
    CHECK_EQ(result2.rows, 3);
    
    ShaderType result3 = promote(t_vec3, t_vec3);
    CHECK_EQ(result3.cls, TypeClass::Vector);
    CHECK_EQ(result3.rows, 3);
    
    ShaderType result4 = promote(t_ivec3, t_ivec3);
    CHECK_EQ(result4.cls, TypeClass::Vector);
    CHECK_EQ(result4.rows, 3);
}

TEST_CASE("ShaderIR - ProgramConstruction") {
    ShaderProgram program;
    program.stage = KX_SHADER_STAGE_VERTEX;
    program.language = KX_SHADER_LANGUAGE_GLSL;
    program.entry_name = "main";
    
    StructDef s;
    s.name = "Vertex";
    s.members.push_back({"pos", make_vector(3), 0});
    s.members.push_back({"normal", make_vector(3), 12});
    s.size = 24;
    program.structs.push_back(std::move(s));
    
    UniformBlock ub;
    ub.name = "CameraData";
    ub.set = 0;
    ub.binding = 0;
    ub.members.push_back({"view", make_matrix(4, 4), StorageClass::Uniform, 0, 64});
    ub.members.push_back({"proj", make_matrix(4, 4), StorageClass::Uniform, 64, 64});
    ub.size = 128;
    program.blocks.push_back(std::move(ub));
    
    Function f;
    f.name = "main";
    f.return_type = make_void();
    f.is_entry = true;
    program.functions.push_back(std::move(f));
    program.entry = 0;
    
    CHECK_EQ(program.structs.size(), 1);
    CHECK_EQ(program.blocks.size(), 1);
    CHECK_EQ(program.functions.size(), 1);
    CHECK_EQ(program.entry, 0);
}

TEST_CASE("ShaderIR - ExpressionConstruction") {
    ShaderProgram program;
    
    Expr lit;
    lit.kind = ExprKind::Literal;
    lit.type = make_float();
    lit.fconst = 3.14f;
    program.exprs.push_back(lit);
    
    Expr var;
    var.kind = ExprKind::VarRef;
    var.type = make_float();
    var.var = 0;
    program.exprs.push_back(var);
    
    Expr bin;
    bin.kind = ExprKind::Binary;
    bin.type = make_float();
    bin.binop = BinaryOp::Add;
    bin.args = {0, 1};
    program.exprs.push_back(bin);
    
    CHECK_EQ(program.exprs.size(), 3);
    CHECK_EQ(program.exprs[0].kind, ExprKind::Literal);
    CHECK_EQ(program.exprs[1].kind, ExprKind::VarRef);
    CHECK_EQ(program.exprs[2].kind, ExprKind::Binary);
    CHECK_EQ(program.exprs[2].binop, BinaryOp::Add);
}

TEST_CASE("ShaderIR - StatementConstruction") {
    ShaderProgram program;
    
    Stmt decl;
    decl.kind = StmtKind::Declare;
    decl.var = 0;
    decl.expr = 0;
    program.exprs.push_back(Expr{});
    program.exprs.back().kind = ExprKind::Literal;
    program.exprs.back().type = make_float();
    program.exprs.back().fconst = 1.0f;
    
    program.functions.push_back(Function{});
    program.functions[0].body.push_back(decl);
    
    CHECK_EQ(program.functions[0].body.size(), 1);
    CHECK_EQ(program.functions[0].body[0].kind, StmtKind::Declare);
}

TEST_CASE("ShaderIR - IRSerialization") {
    ShaderProgram program;
    program.stage = KX_SHADER_STAGE_FRAGMENT;
    program.language = KX_SHADER_LANGUAGE_HLSL;
    program.entry_name = "PSMain";
    
    Function f;
    f.name = "PSMain";
    f.return_type = make_vector(4);
    f.return_type.cls = TypeClass::Vector;
    f.is_entry = true;
    program.functions.push_back(f);
    program.entry = 0;
    
    std::string dump = dump_program(program);
    
    CHECK(dump.find("ShaderProgram") != std::string::npos);
    CHECK(dump.find("FRAGMENT") != std::string::npos);
    CHECK(dump.find("HLSL") != std::string::npos);
    CHECK(dump.find("PSMain") != std::string::npos);
}