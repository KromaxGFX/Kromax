#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESHADERMODULE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESHADERMODULE_H

/*
 * SoftwareShaderModule.h
 *
 * Software shader module: holds native callback functions or SPIR-V bytecode.
 */

#include <cstdint>
#include <cstring>
#include <vector>

#include "kx_core.h"

namespace kx {

class SoftwareShaderModule {
public:
    SoftwareShaderModule() = default;
    explicit SoftwareShaderModule(const KxShaderModuleCreateInfo* create_info) {
        set_create_info(create_info);
    }
    ~SoftwareShaderModule() = default;

    SoftwareShaderModule(const SoftwareShaderModule&) = delete;
    SoftwareShaderModule& operator=(const SoftwareShaderModule&) = delete;

    void set_create_info(const KxShaderModuleCreateInfo* create_info) {
        if (!create_info) return;
        create_info_ = *create_info;

        if (create_info->codeSize > 0 && create_info->pCode) {
            code_.resize(create_info->codeSize);
            std::memcpy(code_.data(), create_info->pCode, create_info->codeSize);
        }
    }

    const KxShaderModuleCreateInfo& create_info() const noexcept {
        return create_info_;
    }

    KxShaderFormat format() const noexcept {
        return create_info_.format;
    }

    const void* code() const noexcept {
        return code_.data();
    }

    uint32_t code_size() const noexcept {
        return static_cast<uint32_t>(code_.size());
    }

    // Native callback functions (for KX_SHADER_FORMAT_NATIVE_CALLBACK_EXT)
    KxVertexShaderCallback vertex_callback = nullptr;
    KxFragmentShaderCallback fragment_callback = nullptr;
    KxComputeShaderCallback compute_callback = nullptr;
    const void* push_constants = nullptr;
    uint32_t push_constants_size = 0;

    void set_native_callbacks(const KxShaderModuleNativeCallbackInfo* native_info) {
        if (!native_info) return;
        vertex_callback = native_info->vertex_fn;
        fragment_callback = native_info->fragment_fn;
        compute_callback = native_info->compute_fn;
        push_constants = native_info->push_constants;
        push_constants_size = native_info->push_constants_size;
    }

private:
    KxShaderModuleCreateInfo create_info_{};
    std::vector<uint8_t> code_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWARESHADERMODULE_H */