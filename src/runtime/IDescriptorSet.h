#ifndef KROMAX_SRC_RUNTIME_IDESCRIPTORSET_H
#define KROMAX_SRC_RUNTIME_IDESCRIPTORSET_H

/*
 * IDescriptorSet.h
 *
 * Backend-agnostic descriptor set interface.
 */

#include <cstdint>

#include "kx_core.h"

namespace kx {

class DescriptorSetLayout;
class Buffer;
class ImageView;
class Sampler;

struct DescriptorBufferInfo {
    Buffer buffer = nullptr;
    KxDeviceSize offset = 0;
    KxDeviceSize range = KX_WHOLE_SIZE;
};

struct DescriptorImageInfo {
    Sampler sampler = nullptr;
    ImageView imageView = nullptr;
    KxImageLayout imageLayout = KX_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
};

struct WriteDescriptorSet {
    KxStructureType sType = KX_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    const void* pNext = nullptr;
    class IDescriptorSet* dstSet = nullptr;
    uint32_t dstBinding = 0;
    uint32_t dstArrayElement = 0;
    uint32_t descriptorCount = 0;
    KxDescriptorType descriptorType = KX_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    const DescriptorImageInfo* pImageInfo = nullptr;
    const DescriptorBufferInfo* pBufferInfo = nullptr;
    const KxBufferView* pTexelBufferView = nullptr;
};

struct CopyDescriptorSet {
    KxStructureType sType = KX_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    const void* pNext = nullptr;
    IDescriptorSet* srcSet = nullptr;
    uint32_t srcBinding = 0;
    uint32_t srcArrayElement = 0;
    IDescriptorSet* dstSet = nullptr;
    uint32_t dstBinding = 0;
    uint32_t dstArrayElement = 0;
    uint32_t descriptorCount = 0;
};

class IDescriptorSetLayout {
public:
    virtual ~IDescriptorSetLayout() = default;

    virtual uint32_t getBindingCount() const noexcept = 0;
    virtual const KxDescriptorSetLayoutBinding* getBindings() const noexcept = 0;
};

class IDescriptorSet {
public:
    virtual ~IDescriptorSet() = default;

    virtual DescriptorSetLayout getLayout() const noexcept = 0;
    virtual void update(uint32_t writeCount, const WriteDescriptorSet* pWrites) = 0;
};

class IDescriptorPool {
public:
    virtual ~IDescriptorPool() = default;

    virtual Result reset() = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IDESCRIPTORSET_H