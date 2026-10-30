/*
 * SoftwareDeviceBuffer.cpp
 *
 * Buffer creation, destruction, and binding.
 */

#include "SoftwareDevice.h"

#include "SoftwareBuffer.h"

namespace kx {

KxResult SoftwareDevice::create_buffer(
    const KxBufferCreateInfo* create_info,
    const KxAllocationCallbacks* allocator,
    KxBuffer* buffer) {
    if (!buffer) return KX_ERROR_INVALID_ARGUMENT;
    *buffer = nullptr;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->sType != KX_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
        return KX_ERROR_INVALID_ARGUMENT;
    if (create_info->size == 0) return KX_ERROR_INVALID_ARGUMENT;

    // Allocate memory for the buffer (use memory type 0 = host visible)
    KxMemoryAllocateInfo mem_info{};
    mem_info.sType = KX_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.allocationSize = create_info->size;
    mem_info.memoryTypeIndex = 0;

    KxDeviceMemory memory = nullptr;
    KxResult result = allocate_memory(&mem_info, allocator, &memory);
    if (result != KX_SUCCESS) return result;

    // Create and register the buffer
    auto sw_memory = find_memory(memory);
    if (!sw_memory) {
        free_memory(memory, allocator);
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    auto buf = std::make_unique<SoftwareBuffer>(
        sw_memory, 0, create_info->size, create_info->usage);
    *buffer = register_buffer(std::move(buf));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_buffer(KxBuffer buffer,
                                    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    auto* buf = find_buffer(buffer);
    if (buf) {
        // Free the underlying memory
        if (buf->memory()) {
            KxDeviceMemory mem = reinterpret_cast<KxDeviceMemory>(buf->memory());
            free_memory(mem, allocator);
        }
        unregister_buffer(buffer);
    }
}

} /* namespace kx */