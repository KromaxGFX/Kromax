/*
 * ShaderIR.cpp
 *
 * Implementation of IR serialization and utilities.
 */

#include "ShaderIR.h"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <string>

namespace kx {
namespace shader {

static const char* shader_stage_name(KxShaderStage stage) {
    switch (stage) {
        case KX_SHADER_STAGE_VERTEX: return "VERTEX";
        case KX_SHADER_STAGE_FRAGMENT: return "FRAGMENT";
        case KX_SHADER_STAGE_COMPUTE: return "COMPUTE";
        default: return "UNKNOWN";
    }
}

static const char* shader_language_name(KxShaderLanguage lang) {
    switch (lang) {
        case KX_SHADER_LANGUAGE_GLSL: return "GLSL";
        case KX_SHADER_LANGUAGE_HLSL: return "HLSL";
        case KX_SHADER_LANGUAGE_WGSL: return "WGSL";
        case KX_SHADER_LANGUAGE_KXSL: return "KXSL";
        default: return "UNKNOWN";
    }
}

std::string dump_program(const ShaderProgram& program) {
    std::ostringstream oss;
    oss << "ShaderProgram {\n";
    oss << "  stage: " << shader_stage_name(program.stage) << "\n";
    oss << "  language: " << shader_language_name(program.language) << "\n";
    oss << "  entry_name: " << program.entry_name << "\n";
    oss << "  entry: " << program.entry << "\n";
    oss << "  structs: " << program.structs.size() << "\n";
    oss << "  blocks: " << program.blocks.size() << "\n";
    oss << "  globals: " << program.globals.size() << "\n";
    oss << "  functions: " << program.functions.size() << "\n";
    oss << "  exprs: " << program.exprs.size() << "\n";

    for (const auto& s : program.structs) {
        oss << "  struct " << s.name << " {\n";
        for (const auto& m : s.members) {
            oss << "    " << m.name << ": type=" << static_cast<int>(m.type.cls)
                << " rows=" << m.type.rows << " cols=" << m.type.cols
                << " offset=" << m.offset << "\n";
        }
        oss << "  } size=" << s.size << "\n";
    }

    for (const auto& b : program.blocks) {
        oss << "  block " << b.name << " set=" << b.set << " binding=" << b.binding
            << " size=" << b.size << " push_constant=" << b.push_constant << "\n";
        for (const auto& m : b.members) {
            oss << "    " << m.name << ": type=" << static_cast<int>(m.type.cls)
                << " storage=" << static_cast<int>(m.storage)
                << " offset=" << m.offset << " size=" << m.size << "\n";
        }
    }

    for (const auto& g : program.globals) {
        oss << "  global " << g.name << ": type=" << static_cast<int>(g.type.cls)
            << " storage=" << static_cast<int>(g.storage)
            << " set=" << g.set << " binding=" << g.binding
            << " location=" << g.location << " offset=" << g.offset << " size=" << g.size << "\n";
    }

    for (const auto& f : program.functions) {
        oss << "  function " << f.name << " entry=" << f.is_entry
            << " builtin=" << f.builtin << " line=" << f.line << "\n";
        oss << "    return_type: " << static_cast<int>(f.return_type.cls)
            << " rows=" << f.return_type.rows << " cols=" << f.return_type.cols << "\n";
        oss << "    params: " << f.params.size() << "\n";
        for (const auto& p : f.params) {
            oss << "      " << p.name << ": type=" << static_cast<int>(p.type.cls) << "\n";
        }
        oss << "    locals: " << f.locals.size() << "\n";
        oss << "    body stmts: " << f.body.size() << "\n";
    }

    oss << "}\n";
    return oss.str();
}

} /* namespace shader */
} /* namespace kx */