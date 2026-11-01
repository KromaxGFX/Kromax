#ifndef KROMAX_SRC_RUNTIME_IPIPELINEFACTORY_H
#define KROMAX_SRC_RUNTIME_IPIPELINEFACTORY_H

/*
 * IPipelineFactory.h
 * Pipeline layout, graphics/compute pipeline creation and destruction.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class Pipeline;
class PipelineLayout;
class DescriptorSetLayout;

struct PipelineLayoutCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineLayoutCreateFlags flags = 0;
    uint32_t setLayoutCount = 0;
    const DescriptorSetLayout* pSetLayouts = nullptr;
    uint32_t pushConstantRangeCount = 0;
    const KxPushConstantRange* pPushConstantRanges = nullptr;
};

struct PipelineCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_PIPELINE_CREATE_INFO;
    const void* pNext = nullptr;
    KxPipelineCreateFlags flags = 0;
    PipelineLayout layout = nullptr;
    // Pipeline-specific data would go here
};

class IPipelineFactory {
public:
    virtual ~IPipelineFactory() = default;

    virtual KxResult createPipelineLayout(const PipelineLayoutCreateInfo& info, PipelineLayout** outLayout) = 0;
    virtual void destroyPipelineLayout(PipelineLayout layout) = 0;
    virtual KxResult createGraphicsPipelines(uint32_t createInfoCount, const PipelineCreateInfo* pCreateInfos, Pipeline* pPipelines) = 0;
    virtual KxResult createComputePipelines(uint32_t createInfoCount, const PipelineCreateInfo* pCreateInfos, Pipeline* pPipelines) = 0;
    virtual void destroyPipeline(Pipeline pipeline) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IPIPELINEFACTORY_H