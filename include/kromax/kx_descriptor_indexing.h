#ifndef KX_DESCRIPTOR_INDEXING_H
#define KX_DESCRIPTOR_INDEXING_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Descriptor Indexing Extension (KX_EXT_descriptor_indexing)
 * ======================================================================= */

#define KX_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME "KX_EXT_descriptor_indexing"
#define KX_EXT_DESCRIPTOR_INDEXING_SPEC_VERSION 1

/* Descriptor indexing flags */
typedef uint32_t KxDescriptorIndexingFlags;
#define KX_DESCRIPTOR_INDEXING_NONE 0u
#define KX_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT 0x00000001
#define KX_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT 0x00000002
#define KX_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT 0x00000004
#define KX_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT 0x00000008
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT 0x00000010
#define KX_DESCRIPTOR_INDEXING_MAX_ENUM 0x7FFFFFFFu

/* Descriptor buffer create info */
typedef struct KxDescriptorBufferCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t flags;
    uint32_t descriptorBufferCount;
    const uint64_t* pDescriptorBufferSizes;
} KxDescriptorBufferCreateInfo;

/* Descriptor buffer info - uses KxDescriptorBufferInfo from kx_descriptor.h */
typedef struct KxDescriptorBufferInfo KxDescriptorBufferInfo;

/* Descriptor buffer binding info */
typedef struct KxDescriptorBufferBindingInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t descriptorBufferCount;
    const KxDescriptorBufferInfo* pDescriptorBuffers;
} KxDescriptorBufferBindingInfo;

/* =======================================================================
 * Descriptor Indexing functions
 * ======================================================================= */

/* Create a descriptor buffer for bindless access */
KX_API KxResult KX_CALL kxCreateDescriptorBuffer(
    KxDevice device,
    const KxDescriptorBufferCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxBuffer* pDescriptorBuffer);

/* Bind descriptor buffers */
KX_API void KX_CALL kxCmdBindDescriptorBuffers(
    KxCommandBuffer commandBuffer,
    uint32_t descriptorBufferCount,
    const KxDescriptorBufferBindingInfo* pDescriptorBuffers);

/* Set descriptor buffer offsets */
KX_API void KX_CALL kxCmdSetDescriptorBufferOffsets(
    KxCommandBuffer commandBuffer,
    KxPipelineBindPoint pipelineBindPoint,
    KxPipelineLayout layout,
    uint32_t firstSet,
    uint32_t setCount,
    const uint64_t* pBufferIndices,
    const uint64_t* pOffsets);

/* Get descriptor buffer device address */
KX_API KxResult KX_CALL kxGetDescriptorBufferDeviceAddress(
    KxDevice device,
    KxBuffer descriptorBuffer,
    uint64_t* pDeviceAddress);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_DESCRIPTOR_INDEXING_H */