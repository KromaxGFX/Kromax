#ifndef KROMAX_SRC_RUNTIME_ISHADERMODULE_H
#define KROMAX_SRC_RUNTIME_ISHADERMODULE_H

/*
 * IShaderModule.h
 *
 * Backend-agnostic shader module interface.
 * Contains IR + reflection + optional SPIR-V bytecode.
 */

#include <cstdint>
#include <vector>
#include <string>
#include <optional>

#include "kx_core.h"

namespace kx {

enum class ShaderStageFlagBits : uint32_t {
    VERTEX                  = 0x00000001,
    TESSELLATION_CONTROL   = 0x00000002,
    TESSELLATION_EVALUATION = 0x00000004,
    GEOMETRY               = 0x00000008,
    FRAGMENT               = 0x00000010,
    COMPUTE                = 0x00000020,
    TASK_EXT               = 0x00000040,
    MESH_EXT               = 0x00000080,
    RAYGEN_KHR             = 0x00000100,
    ANY_HIT_KHR            = 0x00000200,
    CLOSEST_HIT_KHR        = 0x00000400,
    MISS_KHR               = 0x00000800,
    INTERSECTION_KHR       = 0x00001000,
    CALLABLE_KHR           = 0x00002000,
};

using ShaderStageFlags = uint32_t;

struct ShaderResource {
    std::string name;
    uint32_t binding;
    uint32_t set;
    KxDescriptorType type;
    ShaderStageFlags stages;
    uint32_t arraySize = 1;
    uint32_t offset = 0;
    uint32_t size = 0;  // For buffers
    KxFormat format = KX_FORMAT_UNDEFINED;  // For texel buffers
};

struct PushConstantRange {
    std::string name;
    ShaderStageFlags stages;
    uint32_t offset;
    uint32_t size;
};

struct VertexInputAttribute {
    std::string name;
    uint32_t location;
    uint32_t binding;
    KxFormat format;
    uint32_t offset;
};

struct VertexInputBinding {
    uint32_t binding;
    uint32_t stride;
    KxVertexInputRate inputRate;
};

struct ShaderReflection {
    std::string entryPoint;
    ShaderStageFlags stage;
    std::vector<ShaderResource> resources;
    std::vector<PushConstantRange> pushConstants;
    std::vector<VertexInputAttribute> vertexAttributes;
    std::vector<VertexInputBinding> vertexBindings;
    uint32_t localSizeX = 1;
    uint32_t localSizeY = 1;
    uint32_t localSizeZ = 1;
    // Ray tracing
    uint32_t maxRecursionDepth = 1;
    uint32_t payloadSize = 0;
    uint32_t hitAttributeSize = 0;
};

class IShaderModule {
public:
    virtual ~IShaderModule() = default;

    // Get reflection data
    virtual const ShaderReflection& getReflection() const noexcept = 0;

    // Get SPIR-V bytecode (if backend supports it)
    virtual const std::vector<uint32_t>* getSPIRV() const noexcept = 0;

    // Get IR blob (serialized internal IR)
    virtual const std::vector<uint8_t>* getIR() const noexcept = 0;

    // Shader stage
    virtual ShaderStageFlags getStage() const noexcept = 0;

    // Entry point name
    virtual const char* getEntryPoint() const noexcept = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISHADERMODULE_H