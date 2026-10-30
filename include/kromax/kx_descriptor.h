#ifndef KX_DESCRIPTOR_H
#define KX_DESCRIPTOR_H

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Descriptor Set Layout
 * ======================================================================= */

typedef struct KxDescriptorSetLayoutBinding {
    uint32_t binding;
    KxDescriptorType descriptorType;
    uint32_t descriptorCount;
    KxShaderStageFlags stageFlags;
    const void* pImmutableSamplers; /* array of KxSampler */
} KxDescriptorSetLayoutBinding;

typedef uint32_t KxDescriptorSetLayoutCreateFlags;
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_NONE 0u
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT 0x00000001u
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT 0x00000002u
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxDescriptorSetLayoutCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxDescriptorSetLayoutCreateFlags flags;
    uint32_t bindingCount;
    const KxDescriptorSetLayoutBinding* pBindings;
} KxDescriptorSetLayoutCreateInfo;

/* =======================================================================
 * Descriptor Pool
 * ======================================================================= */

typedef struct KxDescriptorPoolSize {
    KxDescriptorType type;
    uint32_t descriptorCount;
} KxDescriptorPoolSize;

typedef uint32_t KxDescriptorPoolCreateFlags;
#define KX_DESCRIPTOR_POOL_CREATE_NONE 0u
#define KX_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT 0x00000001u
#define KX_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT 0x00000002u
#define KX_DESCRIPTOR_POOL_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxDescriptorPoolCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxDescriptorPoolCreateFlags flags;
    uint32_t maxSets;
    uint32_t poolSizeCount;
    const KxDescriptorPoolSize* pPoolSizes;
} KxDescriptorPoolCreateInfo;

typedef uint32_t KxDescriptorPoolResetFlags;
#define KX_DESCRIPTOR_POOL_RESET_NONE 0u
#define KX_DESCRIPTOR_POOL_RESET_MAX_ENUM 0x7FFFFFFFu

/* =======================================================================
 * Descriptor Set
 * ======================================================================= */

typedef struct KxDescriptorSetAllocateInfo {
    KxStructureType sType;
    const void* pNext;
    KxDescriptorPool descriptorPool;
    uint32_t descriptorSetCount;
    const KxDescriptorSetLayout* pSetLayouts;
} KxDescriptorSetAllocateInfo;

typedef struct KxDescriptorImageInfo {
    KxSampler sampler;
    KxImageView imageView;
    KxImageLayout imageLayout;
} KxDescriptorImageInfo;

typedef struct KxDescriptorBufferInfo {
    KxBuffer buffer;
    uint64_t offset;
    uint64_t range;
} KxDescriptorBufferInfo;

typedef struct KxDescriptorBufferInfo2 {
    KxBuffer buffer;
    uint64_t offset;
    uint64_t size;
    uint64_t stride;
} KxDescriptorBufferInfo2;

typedef struct KxWriteDescriptorSet {
    KxStructureType sType;
    const void* pNext;
    KxDescriptorSet dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
    KxDescriptorType descriptorType;
    const KxDescriptorImageInfo* pImageInfo;
    const KxDescriptorBufferInfo* pBufferInfo;
    const void* pTexelBufferView; /* KxBufferView* */
} KxWriteDescriptorSet;

typedef struct KxCopyDescriptorSet {
    KxStructureType sType;
    const void* pNext;
    KxDescriptorSet srcSet;
    uint32_t srcBinding;
    uint32_t srcArrayElement;
    KxDescriptorSet dstSet;
    uint32_t dstBinding;
    uint32_t dstArrayElement;
    uint32_t descriptorCount;
} KxCopyDescriptorSet;

/* =======================================================================
 * Sampler
 * ======================================================================= */

typedef uint32_t KxSamplerCreateFlags;
#define KX_SAMPLER_CREATE_NONE 0u
#define KX_SAMPLER_CREATE_MAX_ENUM 0x7FFFFFFFu

typedef struct KxSamplerCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxSamplerCreateFlags flags;
    KxFilter magFilter;
    KxFilter minFilter;
    KxSamplerAddressMode addressModeU;
    KxSamplerAddressMode addressModeV;
    KxSamplerAddressMode addressModeW;
    float mipLodBias;
    KxBool32 anisotropyEnable;
    float maxAnisotropy;
    KxBool32 compareEnable;
    KxCompareOp compareOp;
    float minLod;
    float maxLod;
    KxBorderColor borderColor;
    KxBool32 unnormalizedCoordinates;
} KxSamplerCreateInfo;

/* =======================================================================
 * Bound texture (resolved descriptor)
 * ======================================================================= */

typedef struct KxBoundTexture {
    KxSampler sampler;
    KxImageView imageView;
    uint32_t width;
    uint32_t height;
    uint32_t rowPitch;
    KxFormat format;
    const void* pTexels;
} KxBoundTexture;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_DESCRIPTOR_H */