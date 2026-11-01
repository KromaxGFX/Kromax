#ifndef KROMAX_SRC_RUNTIME_IDESCRIPTORFACTORY_H
#define KROMAX_SRC_RUNTIME_IDESCRIPTORFACTORY_H

/*
 * IDescriptorFactory.h
 * Descriptor set layout, pool, allocation, and updates.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class DescriptorSetLayout;
class DescriptorPool;
class DescriptorSet;

struct DescriptorSetLayoutCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    const void* pNext = nullptr;
    KxDescriptorSetLayoutCreateFlags flags = 0;
    uint32_t bindingCount = 0;
    const KxDescriptorSetLayoutBinding* pBindings = nullptr;
};

struct DescriptorPoolCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    const void* pNext = nullptr;
    KxDescriptorPoolCreateFlags flags = 0;
    uint32_t maxSets = 0;
    uint32_t poolSizeCount = 0;
    const KxDescriptorPoolSize* pPoolSizes = nullptr;
};

struct DescriptorSetAllocateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    const void* pNext = nullptr;
    DescriptorPool descriptorPool = nullptr;
    uint32_t descriptorSetCount = 0;
    const DescriptorSetLayout* pSetLayouts = nullptr;
};

class IDescriptorFactory {
public:
    virtual ~IDescriptorFactory() = default;

    virtual KxResult createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo& info, DescriptorSetLayout** outLayout) = 0;
    virtual void destroyDescriptorSetLayout(DescriptorSetLayout layout) = 0;
    virtual KxResult createDescriptorPool(const DescriptorPoolCreateInfo& info, DescriptorPool** outPool) = 0;
    virtual void destroyDescriptorPool(DescriptorPool pool) = 0;
    virtual KxResult allocateDescriptorSets(const DescriptorSetAllocateInfo& info, DescriptorSet* pDescriptorSets) = 0;
    virtual KxResult freeDescriptorSets(DescriptorPool pool, uint32_t descriptorSetCount, const DescriptorSet* pDescriptorSets) = 0;
    virtual KxResult updateDescriptorSets(uint32_t descriptorWriteCount, const KxWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const KxCopyDescriptorSet* pDescriptorCopies) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IDESCRIPTORFACTORY_H