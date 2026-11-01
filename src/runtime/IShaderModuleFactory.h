#ifndef KROMAX_SRC_RUNTIME_ISHADERMODULEFACTORY_H
#define KROMAX_SRC_RUNTIME_ISHADERMODULEFACTORY_H

/*
 * IShaderModuleFactory.h
 * Shader module creation and destruction.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class ShaderModule;

struct ShaderModuleCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    const void* pNext = nullptr;
    KxShaderModuleCreateFlags flags = 0;
    size_t codeSize = 0;
    const uint32_t* pCode = nullptr;
};

class IShaderModuleFactory {
public:
    virtual ~IShaderModuleFactory() = default;

    virtual KxResult createShaderModule(const ShaderModuleCreateInfo& info, ShaderModule** outModule) = 0;
    virtual void destroyShaderModule(ShaderModule* module) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISHADERMODULEFACTORY_H