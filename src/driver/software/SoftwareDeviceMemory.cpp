/*
 * SoftwareDeviceMemory.cpp
 *
 * Memory allocation and management.
 */

#include "SoftwareDevice.h"

#include <cstring>

#include "Allocator.h"
#include "SoftwareMemory.h"

namespace kx {

KxResult SoftwareDevice::allocate_memory(
    const KxMemoryAllocateInfo* allocate_info,
    const KxAllocationCallbacks* allocator,
    KxDeviceMemory* memory) {
    if (!memory) return KX_ERROR_INVALID_ARGUMENT;
    *memory = nullptr;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;

    auto mem = std::make_unique<SoftwareMemory>(
        allocate_info->allocationSize,
        allocate_info->memoryTypeIndex,
        allocator);
    *memory = register_memory(std::move(mem));
    return KX_SUCCESS;
}

void SoftwareDevice::free_memory(KxDeviceMemory memory,
                                 const KxAllocationCallbacks* allocator) {
    (void)allocator;
    unregister_memory(memory);
}

KxResult SoftwareDevice::map_memory(
    KxDeviceMemory memory,
    uint64_t offset,
    uint64_t size,
    KxMemoryMapFlags flags,
    void** pp_data) {
    (void)flags;
    if (!pp_data) return KX_ERROR_INVALID_ARGUMENT;
    *pp_data = nullptr;
    auto* mem = find_memory(memory);
    if (!mem) return KX_ERROR_INVALID_ARGUMENT;
    
    // Handle KX_WHOLE_SIZE (UINT64_MAX) - map from offset to end
    uint64_t map_size = size;
    if (size == KX_WHOLE_SIZE) {
        if (offset >= mem->size()) return KX_ERROR_MEMORY_MAP_FAILED;
        map_size = mem->size() - offset;
    }
    
    if (offset + map_size > mem->size()) return KX_ERROR_MEMORY_MAP_FAILED;
    *pp_data = mem->data() + offset;
    return KX_SUCCESS;
}

void SoftwareDevice::unmap_memory(KxDeviceMemory memory) {
    (void)memory;
    // No-op for host-visible memory
}

KxResult SoftwareDevice::bind_buffer_memory(
    KxBuffer buffer,
    KxDeviceMemory memory,
    uint64_t memory_offset) {
    auto* buf = find_buffer(buffer);
    auto* mem = find_memory(memory);
    if (!buf || !mem) return KX_ERROR_INVALID_ARGUMENT;
    buf->bind_memory(mem, memory_offset);
    return KX_SUCCESS;
}

KxResult SoftwareDevice::bind_image_memory(
    KxImage image,
    KxDeviceMemory memory,
    uint64_t memory_offset) {
    (void)memory_offset;
    auto* img = find_image(image);
    auto* mem = find_memory(memory);
    if (!img || !mem) return KX_ERROR_INVALID_ARGUMENT;
    img->bind_memory(mem);
    return KX_SUCCESS;
}

} /* namespace kx */