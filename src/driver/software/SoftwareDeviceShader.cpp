/*
 * SoftwareDeviceShader.cpp
 *
 * Shader module creation/destruction and pipeline/shader lookup, registration, unregistration.
 */

#include "SoftwareDevice.h"

#include "SoftwarePipeline.h"
#include "SoftwarePipelineLayout.h"
#include "SoftwareShaderModule.h"
#include "../shader/ShaderCompiler.h"
#include "../shader/ShaderDiagnostics.h"

namespace kx {

/* Shader Module */

KxResult SoftwareDevice::create_shader_module(
    const KxShaderModuleCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxShaderModule* shader_module) {
    (void)allocator;
    if (!shader_module) return KX_ERROR_INVALID_ARGUMENT;
    *shader_module = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;

    auto module = std::make_unique<SoftwareShaderModule>();
    module->set_create_info(create_info);

    // If native callback format, extract callbacks from pCode
    if (create_info->format == KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT) {
        if (create_info->pCode && create_info->codeSize >= sizeof(KxShaderModuleNativeCallbackInfo)) {
            const KxShaderModuleNativeCallbackInfo* native_info =
                static_cast<const KxShaderModuleNativeCallbackInfo*>(create_info->pCode);
            module->set_native_callbacks(native_info);
        }
    }
    // If source format, compile the shader
    else if (create_info->format == KX_SHADER_FORMAT_SOURCE_EXT) {
        if (!create_info->pCode || create_info->codeSize == 0) {
            return KX_ERROR_INVALID_ARGUMENT;
        }

        const char* source = static_cast<const char*>(create_info->pCode);
        size_t source_size = create_info->codeSize == UINT32_MAX ? std::strlen(source) : create_info->codeSize;

        // Use the pipeline layout to determine stage if possible
        shader::ShaderCompiler::CompileOptions options;
        options.entry_point = "main";

        // Detect stage from the source or use vertex as default
        if (std::strstr(source, "void main()") || std::strstr(source, "fn main(")) {
            // Try to detect stage from attributes/qualifiers
            if (std::strstr(source, "gl_Position") || std::strstr(source, "@builtin(position)"))
                options.stage_hint = KX_SHADER_STAGE_VERTEX;
            else if (std::strstr(source, "gl_FragColor") || std::strstr(source, "@builtin(frag_depth)"))
                options.stage_hint = KX_SHADER_STAGE_FRAGMENT;
            else if (std::strstr(source, "numthreads") || std::strstr(source, "@builtin(global_invocation_id)"))
                options.stage_hint = KX_SHADER_STAGE_COMPUTE;
            else
                options.stage_hint = KX_SHADER_STAGE_VERTEX;
        }

        shader::DiagnosticList diagnostics;
        shader::ShaderCompiler compiler(diagnostics);

        // Set up include resolver if needed (for now, no external includes)
        compiler.set_include_resolver([](const std::string&, std::string&) -> bool {
            return false;
        });

        // Define common macros
        compiler.define("KX_SOFTWARE_DRIVER", "1");

        shader::ShaderProgram program = compiler.compile(std::string(source, source_size), &options);

        if (diagnostics.has_errors()) {
            // Could return error info via a future extension
            return KX_ERROR_VALIDATION_FAILED_EXT;
        }

        module->program = std::make_unique<shader::ShaderProgram>(std::move(program));
        module->interpreter = std::make_unique<shader::Interpreter>(*module->program);
    }

    *shader_module = register_shader_module(std::move(module));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_shader_module(
    KxShaderModule shader_module,
    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_shader_module(shader_module);
}

/* Lookup methods */

SoftwarePipelineLayout* SoftwareDevice::find_pipeline_layout(KxPipelineLayout layout) noexcept {
    for (auto& l : pipeline_layouts_) {
        if (reinterpret_cast<KxPipelineLayout>(l.get()) == layout) return l.get();
    }
    return nullptr;
}

SoftwarePipeline* SoftwareDevice::find_pipeline(KxPipeline pipeline) noexcept {
    for (auto& p : pipelines_) {
        if (reinterpret_cast<KxPipeline>(p.get()) == pipeline) return p.get();
    }
    return nullptr;
}

SoftwareShaderModule* SoftwareDevice::find_shader_module(KxShaderModule module) noexcept {
    for (auto& s : shader_modules_) {
        if (reinterpret_cast<KxShaderModule>(s.get()) == module) return s.get();
    }
    return nullptr;
}

/* Registration */

KxPipelineLayout SoftwareDevice::register_pipeline_layout(std::unique_ptr<SoftwarePipelineLayout> layout) {
    KxPipelineLayout handle = reinterpret_cast<KxPipelineLayout>(layout.get());
    pipeline_layouts_.push_back(std::move(layout));
    return handle;
}

KxPipeline SoftwareDevice::register_pipeline(std::unique_ptr<SoftwarePipeline> pipeline) {
    KxPipeline handle = reinterpret_cast<KxPipeline>(pipeline.get());
    pipelines_.push_back(std::move(pipeline));
    return handle;
}

KxShaderModule SoftwareDevice::register_shader_module(std::unique_ptr<SoftwareShaderModule> module) {
    KxShaderModule handle = reinterpret_cast<KxShaderModule>(module.get());
    shader_modules_.push_back(std::move(module));
    return handle;
}

/* Unregistration */

void SoftwareDevice::unregister_pipeline_layout(KxPipelineLayout layout) {
    for (auto it = pipeline_layouts_.begin(); it != pipeline_layouts_.end(); ++it) {
        if (reinterpret_cast<KxPipelineLayout>(it->get()) == layout) {
            pipeline_layouts_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_pipeline(KxPipeline pipeline) {
    for (auto it = pipelines_.begin(); it != pipelines_.end(); ++it) {
        if (reinterpret_cast<KxPipeline>(it->get()) == pipeline) {
            pipelines_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_shader_module(KxShaderModule module) {
    for (auto it = shader_modules_.begin(); it != shader_modules_.end(); ++it) {
        if (reinterpret_cast<KxShaderModule>(it->get()) == module) {
            shader_modules_.erase(it);
            return;
        }
    }
}

} /* namespace kx */