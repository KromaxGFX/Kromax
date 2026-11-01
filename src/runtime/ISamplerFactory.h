#ifndef KROMAX_SRC_RUNTIME_ISAMPLERFACTORY_H
#define KROMAX_SRC_RUNTIME_ISAMPLERFACTORY_H

/*
 * ISamplerFactory.h
 * Sampler creation and destruction.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Sampler;

struct SamplerCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    const void* pNext = nullptr;
    KxSamplerCreateFlags flags = 0;
    KxFilter magFilter = KX_FILTER_NEAREST;
    KxFilter minFilter = KX_FILTER_NEAREST;
    KxSamplerMipmapMode mipmapMode = KX_SAMPLER_MIPMAP_MODE_NEAREST;
    KxSamplerAddressMode addressModeU = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    KxSamplerAddressMode addressModeV = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    KxSamplerAddressMode addressModeW = KX_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.0f;
    KxBool32 anisotropyEnable = false;
    float maxAnisotropy = 1.0f;
    KxBool32 compareEnable = false;
    KxCompareOp compareOp = KX_COMPARE_OP_NEVER;
    float minLod = 0.0f;
    float maxLod = 0.0f;
    KxBorderColor borderColor = KX_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    KxBool32 unnormalizedCoordinates = false;
};

class ISamplerFactory {
public:
    virtual ~ISamplerFactory() = default;

    virtual KxResult createSampler(const SamplerCreateInfo& info, Sampler** outSampler) = 0;
    virtual void destroySampler(Sampler* sampler) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_ISAMPLERFACTORY_H