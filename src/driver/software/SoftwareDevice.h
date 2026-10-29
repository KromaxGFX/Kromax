#ifndef KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H
#define KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H

/*
 * SoftwareDevice.h
 *
 * Software implementation of IDriverDevice. Holds memory pools, buffers,
 * images, command pools and command buffers, and executes recorded
 * command buffers on the CPU.
 */

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "IDriverDevice.h"
#include "PhysicalDevice.h"
#include "SoftwareCommandBuffer.h"
#include "SoftwareMemory.h"
#include "SoftwareBuffer.h"
#include "SoftwareImage.h"
#include "SoftwareCommandPool.h"

namespace kx {

class SoftwareDevice : public IDriverDevice {
public:
    explicit SoftwareDevice(PhysicalDevice* physical_device);
    ~SoftwareDevice() override = default;

    PhysicalDevice* physical_device() const noexcept { return physical_device_; }

    /* Object lookups (public handle -> internal object) */
    SoftwareMemory* find_memory(KxDeviceMemory mem) const noexcept;
    SoftwareBuffer* find_buffer(KxBuffer buf) const noexcept;
    SoftwareImage* find_image(KxImage img) const noexcept;
    SoftwareCommandPool* find_command_pool(KxCommandPool pool) const noexcept;
    SoftwareCommandBuffer* find_command_buffer(KxCommandBuffer cb) const noexcept;

    /* Object registration */
    KxDeviceMemory register_memory(std::unique_ptr<SoftwareMemory> mem);
    KxBuffer register_buffer(std::unique_ptr<SoftwareBuffer> buf);
    KxImage register_image(std::unique_ptr<SoftwareImage> img);
    KxCommandPool register_command_pool(std::unique_ptr<SoftwareCommandPool> pool);
    KxCommandBuffer register_command_buffer(
        std::unique_ptr<SoftwareCommandBuffer> cmd);

    void unregister_memory(KxDeviceMemory mem);
    void unregister_buffer(KxBuffer buf);
    void unregister_image(KxImage img);
    void unregister_command_pool(KxCommandPool pool);
    void unregister_command_buffer(KxCommandBuffer cb);

    /* IDriverDevice */
    KxResult allocate_memory(
        const KxMemoryAllocateInfo* allocate_info,
        const KxAllocationCallbacks* allocator,
        KxDeviceMemory* memory) override;
    void free_memory(KxDeviceMemory memory,
                     const KxAllocationCallbacks* allocator) override;
    KxResult create_buffer(
        const KxBufferCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxBuffer* buffer) override;
    void destroy_buffer(KxBuffer buffer,
                        const KxAllocationCallbacks* allocator) override;
    KxResult create_image(
        const KxImageCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxImage* image) override;
    void destroy_image(KxImage image,
                       const KxAllocationCallbacks* allocator) override;
    KxResult create_command_pool(
        const KxCommandPoolCreateInfo* create_info,
        const KxAllocationCallbacks* allocator,
        KxCommandPool* command_pool) override;
    void destroy_command_pool(KxCommandPool command_pool,
                              const KxAllocationCallbacks* allocator) override;
    KxResult allocate_command_buffers(
        const KxCommandBufferAllocateInfo* allocate_info,
        KxCommandBuffer* command_buffers) override;
    void free_command_buffers(KxCommandPool command_pool,
                              uint32_t count,
                              const KxCommandBuffer* command_buffers) override;
    KxResult begin_command_buffer(
        KxCommandBuffer command_buffer,
        const KxCommandBufferBeginInfo* begin_info) override;
    KxResult end_command_buffer(KxCommandBuffer command_buffer) override;
    KxResult queue_submit(uint32_t submit_count,
                          const KxSubmitInfo* submits,
                          KxFence fence) override;
    KxResult queue_wait_idle() override;
    KxResult device_wait_idle() override;

private:
    PhysicalDevice* physical_device_;

    std::vector<std::unique_ptr<SoftwareMemory>> memories_;
    std::vector<std::unique_ptr<SoftwareBuffer>> buffers_;
    std::vector<std::unique_ptr<SoftwareImage>> images_;
    std::vector<std::unique_ptr<SoftwareCommandPool>> command_pools_;
    std::vector<std::unique_ptr<SoftwareCommandBuffer>> command_buffers_;
};

} /* namespace kx */

#endif /* KROMAX_SRC_DRIVER_SOFTWARE_SOFTWAREDEVICE_H */