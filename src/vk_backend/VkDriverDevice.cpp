/*
 * VkDriverDevice.cpp
 *
 * Vulkan implementation of IDriverDevice.
 */

#include "VkDriverDevice.h"
#include "VkDevice.h"
#include "VkShaderModule.h"
#include "VkPipeline.h"
#include "VkDescriptor.h"
#include "VkCommand.h"
#include "VkRenderPass.h"
#include "VkBuffer.h"
#include "VkSwapchain.h"

#include <cstring>

namespace kx {

VkDriverDevice::VkDriverDevice(VkDeviceImpl* device)
    : vk_device_(device) {}

VkDriverDevice::~VkDriverDevice() = default;

VkDeviceImpl* VkDriverDevice::vk_device() const noexcept {
    return vk_device_;
}

VkFunctionTable& VkDriverDevice::functions() noexcept {
    return vk_device_->getFunctionTable();
}

VkFunctionTable& VkDriverDevice::functions() const noexcept {
    return vk_device_->getFunctionTable();
}

// ============================================================================
// Memory
// ============================================================================

KxResult VkDriverDevice::allocate_memory(const KxMemoryAllocateInfo* allocate_info,
                                         const KxAllocationCallbacks* allocator,
                                         KxDeviceMemory* memory) {
    (void)allocator;
    if (!memory) return KX_ERROR_INVALID_ARGUMENT;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto mem = std::make_unique<VkMemoryImpl>();
    if (!mem->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), allocate_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *memory = register_memory(std::move(mem));
    return KX_SUCCESS;
}

void VkDriverDevice::free_memory(KxDeviceMemory memory,
                                 const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!memory) return;

    auto* mem = find_vk_memory(memory);
    if (mem) {
        auto& funcs = functions();
        if (mem->handle() != VK_NULL_HANDLE) {
            funcs.vkFreeMemory(static_cast<VkDevice>(vk_device_->getHandle()), mem->handle(), nullptr);
        }
    }
    unregister_memory(memory);
}

// ============================================================================
// Buffer
// ============================================================================

KxResult VkDriverDevice::create_buffer(const KxBufferCreateInfo* create_info,
                                       const KxAllocationCallbacks* allocator,
                                       KxBuffer* buffer) {
    (void)allocator;
    if (!buffer) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto buf = std::make_unique<VkBufferImpl>();
    if (!buf->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *buffer = register_buffer(std::move(buf));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_buffer(KxBuffer buffer,
                                    const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!buffer) return;

    auto* buf = find_vk_buffer(buffer);
    if (buf) {
        auto& funcs = functions();
        if (buf->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyBuffer(static_cast<VkDevice>(vk_device_->getHandle()), buf->handle(), nullptr);
        }
    }
    unregister_buffer(buffer);
}

// ============================================================================
// Image
// ============================================================================

KxResult VkDriverDevice::create_image(const KxImageCreateInfo* create_info,
                                      const KxAllocationCallbacks* allocator,
                                      KxImage* image) {
    (void)allocator;
    if (!image) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto img = std::make_unique<VkImageImpl>();
    if (!img->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *image = register_image(std::move(img));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_image(KxImage image,
                                   const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!image) return;

    auto* img = find_vk_image(image);
    if (img) {
        auto& funcs = functions();
        if (img->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyImage(static_cast<VkDevice>(vk_device_->getHandle()), img->handle(), nullptr);
        }
    }
    unregister_image(image);
}

// ============================================================================
// Image View
// ============================================================================

KxResult VkDriverDevice::create_image_view(const KxImageViewCreateInfo* create_info,
                                           const KxAllocationCallbacks* allocator,
                                           KxImageView* image_view) {
    (void)allocator;
    if (!image_view) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto view = std::make_unique<VkImageViewImpl>();
    if (!view->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *image_view = register_image_view(std::move(view));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_image_view(KxImageView image_view,
                                        const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!image_view) return;

    auto* view = find_vk_image_view(image_view);
    if (view) {
        auto& funcs = functions();
        if (view->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyImageView(static_cast<VkDevice>(vk_device_->getHandle()), view->handle(), nullptr);
        }
    }
    unregister_image_view(image_view);
}

// ============================================================================
// Render Pass
// ============================================================================

KxResult VkDriverDevice::create_render_pass(const KxRenderPassCreateInfo* create_info,
                                            const KxAllocationCallbacks* allocator,
                                            KxRenderPass* render_pass) {
    (void)allocator;
    if (!render_pass) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto rp = std::make_unique<VkRenderPassImpl>();
    if (!rp->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *render_pass = register_render_pass(std::move(rp));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_render_pass(KxRenderPass render_pass,
                                         const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!render_pass) return;

    auto* rp = find_vk_render_pass(render_pass);
    if (rp) {
        auto& funcs = functions();
        if (rp->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyRenderPass(static_cast<VkDevice>(vk_device_->getHandle()), rp->handle(), nullptr);
        }
    }
    unregister_render_pass(render_pass);
}

// ============================================================================
// Framebuffer
// ============================================================================

KxResult VkDriverDevice::create_framebuffer(const KxFramebufferCreateInfo* create_info,
                                            const KxAllocationCallbacks* allocator,
                                            KxFramebuffer* framebuffer) {
    (void)allocator;
    if (!framebuffer) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto fb = std::make_unique<VkFramebufferImpl>();
    if (!fb->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *framebuffer = register_framebuffer(std::move(fb));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_framebuffer(KxFramebuffer framebuffer,
                                         const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!framebuffer) return;

    auto* fb = find_vk_framebuffer(framebuffer);
    if (fb) {
        auto& funcs = functions();
        if (fb->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyFramebuffer(static_cast<VkDevice>(vk_device_->getHandle()), fb->handle(), nullptr);
        }
    }
    unregister_framebuffer(framebuffer);
}

// ============================================================================
// Command Pool
// ============================================================================

KxResult VkDriverDevice::create_command_pool(const KxCommandPoolCreateInfo* create_info,
                                             const KxAllocationCallbacks* allocator,
                                             KxCommandPool* command_pool) {
    (void)allocator;
    if (!command_pool) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto pool = std::make_unique<VkCommandPoolImpl>();
    if (!pool->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *command_pool = register_command_pool(std::move(pool));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_command_pool(KxCommandPool command_pool,
                                          const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!command_pool) return;

    auto* pool = find_vk_command_pool(command_pool);
    if (pool) {
        auto& funcs = functions();
        if (pool->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyCommandPool(static_cast<VkDevice>(vk_device_->getHandle()), pool->handle(), nullptr);
        }
    }
    unregister_command_pool(command_pool);
}

// ============================================================================
// Command Buffer
// ============================================================================

KxResult VkDriverDevice::allocate_command_buffers(const KxCommandBufferAllocateInfo* allocate_info,
                                                  KxCommandBuffer* command_buffers) {
    if (!command_buffers) return KX_ERROR_INVALID_ARGUMENT;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();

    auto* pool = find_vk_command_pool(allocate_info->commandPool);
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;

    uint32_t count = allocate_info->commandBufferCount;
    KxCommandBufferLevel level = allocate_info->level;

    for (uint32_t i = 0; i < count; ++i) {
        auto cmd = std::make_unique<VkCommandBufferImpl>();
        if (!cmd->initialize(funcs,
                             static_cast<VkDevice>(vk_device_->getHandle()),
                             pool->handle(),
                             level)) {
            // Free already allocated
            for (uint32_t j = 0; j < i; ++j) {
                unregister_command_buffer(command_buffers[j]);
            }
            return KX_ERROR_OUT_OF_DEVICE_MEMORY;
        }
        command_buffers[i] = register_command_buffer(std::move(cmd));
    }

    return KX_SUCCESS;
}

void VkDriverDevice::free_command_buffers(KxCommandPool command_pool,
                                          uint32_t count,
                                          const KxCommandBuffer* command_buffers) {
    if (!command_pool || !command_buffers || count == 0) return;

    auto* pool = find_vk_command_pool(command_pool);
    if (!pool) return;

    auto& funcs = functions();
    for (uint32_t i = 0; i < count; ++i) {
        auto* cmd = find_vk_command_buffer(command_buffers[i]);
        if (cmd) {
            VkCommandBuffer handle = cmd->handle();
            funcs.vkFreeCommandBuffers(static_cast<VkDevice>(vk_device_->getHandle()),
                                       pool->handle(), 1, &handle);
        }
        unregister_command_buffer(command_buffers[i]);
    }
}

KxResult VkDriverDevice::begin_command_buffer(KxCommandBuffer command_buffer,
                                              const KxCommandBufferBeginInfo* begin_info) {
    if (!command_buffer) return KX_ERROR_INVALID_ARGUMENT;

    auto* cmd = find_vk_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();

    VkCommandBufferBeginInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_info.flags = static_cast<VkCommandBufferUsageFlags>(begin_info->flags);
    vk_info.pInheritanceInfo = nullptr; // TODO: Support secondary command buffers

    VkResult result = funcs.vkBeginCommandBuffer(cmd->handle(), &vk_info);
    if (result == VK_SUCCESS) {
        cmd->set_recording(true);
    }
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult VkDriverDevice::end_command_buffer(KxCommandBuffer command_buffer) {
    if (!command_buffer) return KX_ERROR_INVALID_ARGUMENT;

    auto* cmd = find_vk_command_buffer(command_buffer);
    if (!cmd) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    VkResult result = funcs.vkEndCommandBuffer(cmd->handle());
    if (result == VK_SUCCESS) {
        cmd->set_recording(false);
    }
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

// ============================================================================
// Queue Operations
// ============================================================================

KxResult VkDriverDevice::queue_submit(uint32_t submit_count,
                                      const KxSubmitInfo* submits,
                                      KxFence fence) {
    if (submit_count == 0 || !submits) return KX_ERROR_INVALID_ARGUMENT;

    // Get the first queue (graphics queue)
    auto& funcs = functions();
    auto queues = vk_device_->getQueues();
    if (queues.empty()) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* queue_impl = find_vk_queue(static_cast<KxQueue>(queues[0]));
    if (!queue_impl) return KX_ERROR_FEATURE_NOT_PRESENT;

    VkFence vk_fence = fence ? reinterpret_cast<VkFence>(fence) : VK_NULL_HANDLE;
    VkResult result = queue_impl->submit(funcs, submit_count, submits, vk_fence);
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult VkDriverDevice::queue_wait_idle() {
    auto& funcs = functions();
    auto queues = vk_device_->getQueues();
    if (queues.empty()) return KX_ERROR_FEATURE_NOT_PRESENT;

    auto* queue_impl = find_vk_queue(static_cast<KxQueue>(queues[0]));
    if (!queue_impl) return KX_ERROR_FEATURE_NOT_PRESENT;

    VkResult result = queue_impl->wait_idle(funcs);
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult VkDriverDevice::device_wait_idle() {
    auto& funcs = functions();
    return funcs.vkDeviceWaitIdle(static_cast<VkDevice>(vk_device_->getHandle())) == VK_SUCCESS
        ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

// ============================================================================
// Memory Binding
// ============================================================================

KxResult VkDriverDevice::bind_buffer_memory(KxBuffer buffer,
                                            KxDeviceMemory memory,
                                            uint64_t memory_offset) {
    if (!buffer || !memory) return KX_ERROR_INVALID_ARGUMENT;

    auto* buf = find_vk_buffer(buffer);
    auto* mem = find_vk_memory(memory);
    if (!buf || !mem) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    VkResult result = funcs.vkBindBufferMemory(static_cast<VkDevice>(vk_device_->getHandle()),
                                                buf->handle(), mem->handle(), memory_offset);
    if (result == VK_SUCCESS) {
        buf->set_memory(mem->handle(), memory_offset);
    }
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult VkDriverDevice::bind_image_memory(KxImage image,
                                           KxDeviceMemory memory,
                                           uint64_t memory_offset) {
    if (!image || !memory) return KX_ERROR_INVALID_ARGUMENT;

    auto* img = find_vk_image(image);
    auto* mem = find_vk_memory(memory);
    if (!img || !mem) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    VkResult result = funcs.vkBindImageMemory(static_cast<VkDevice>(vk_device_->getHandle()),
                                               img->handle(), mem->handle(), memory_offset);
    if (result == VK_SUCCESS) {
        img->set_memory(mem->handle(), memory_offset);
    }
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult VkDriverDevice::map_memory(KxDeviceMemory memory,
                                    uint64_t offset,
                                    uint64_t size,
                                    KxMemoryMapFlags flags,
                                    void** pp_data) {
    (void)flags;
    if (!memory || !pp_data) return KX_ERROR_INVALID_ARGUMENT;

    auto* mem = find_vk_memory(memory);
    if (!mem) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    void* data = nullptr;
    VkResult result = funcs.vkMapMemory(static_cast<VkDevice>(vk_device_->getHandle()),
                                         mem->handle(), offset, size, 0, &data);
    if (result == VK_SUCCESS) {
        mem->set_mapped_ptr(data);
        *pp_data = data;
        return KX_SUCCESS;
    }
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void VkDriverDevice::unmap_memory(KxDeviceMemory memory) {
    if (!memory) return;

    auto* mem = find_vk_memory(memory);
    if (!mem) return;

    auto& funcs = functions();
    funcs.vkUnmapMemory(static_cast<VkDevice>(vk_device_->getHandle()), mem->handle());
    mem->set_mapped_ptr(nullptr);
}

// ============================================================================
// Pipeline
// ============================================================================

KxResult VkDriverDevice::create_pipeline_layout(const KxPipelineLayoutCreateInfo* create_info,
                                                const KxAllocationCallbacks* allocator,
                                                KxPipelineLayout* pipeline_layout) {
    (void)allocator;
    if (!pipeline_layout) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto layout = std::make_unique<VkPipelineLayoutImpl>();
    if (!layout->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *pipeline_layout = register_pipeline_layout(std::move(layout));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_pipeline_layout(KxPipelineLayout pipeline_layout,
                                             const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!pipeline_layout) return;

    auto* layout = find_vk_pipeline_layout(pipeline_layout);
    if (layout) {
        auto& funcs = functions();
        if (layout->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyPipelineLayout(static_cast<VkDevice>(vk_device_->getHandle()), layout->handle(), nullptr);
        }
    }
    unregister_pipeline_layout(pipeline_layout);
}

KxResult VkDriverDevice::create_pipeline(const KxPipelineCreateInfo* create_info,
                                         const KxAllocationCallbacks* allocator,
                                         KxPipeline* pipeline) {
    (void)allocator;
    if (!pipeline) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto pipeline_impl = std::make_unique<VkPipelineImpl>();

    // Determine if graphics or compute pipeline
    // For now, assume graphics
    bool success = pipeline_impl->initialize_graphics(funcs,
                                                      static_cast<VkDevice>(vk_device_->getHandle()),
                                                      VK_NULL_HANDLE, // No pipeline cache for now
                                                      create_info);
    if (!success) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *pipeline = register_pipeline(std::move(pipeline_impl));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_pipeline(KxPipeline pipeline,
                                      const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!pipeline) return;

    auto* pipe = find_vk_pipeline(pipeline);
    if (pipe) {
        auto& funcs = functions();
        if (pipe->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyPipeline(static_cast<VkDevice>(vk_device_->getHandle()), pipe->handle(), nullptr);
        }
    }
    unregister_pipeline(pipeline);
}

// ============================================================================
// Shader Module
// ============================================================================

KxResult VkDriverDevice::create_shader_module(const KxShaderModuleCreateInfo* create_info,
                                              const KxAllocationCallbacks* allocator,
                                              KxShaderModule* shader_module) {
    (void)allocator;
    if (!shader_module) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto module = std::make_unique<VkShaderModuleImpl>();

    std::vector<uint32_t> spirv;
    spirv.resize(create_info->codeSize / sizeof(uint32_t));
    std::memcpy(spirv.data(), create_info->pCode, create_info->codeSize);

    if (!module->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), spirv)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *shader_module = register_shader_module(std::move(module));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_shader_module(KxShaderModule shader_module,
                                           const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!shader_module) return;

    auto* module = find_vk_shader_module(shader_module);
    if (module) {
        auto& funcs = functions();
        if (module->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyShaderModule(static_cast<VkDevice>(vk_device_->getHandle()), module->handle(), nullptr);
        }
    }
    unregister_shader_module(shader_module);
}

// ============================================================================
// Descriptor Set Layout
// ============================================================================

KxResult VkDriverDevice::create_descriptor_set_layout(const KxDescriptorSetLayoutCreateInfo* create_info,
                                                      const KxAllocationCallbacks* allocator,
                                                      KxDescriptorSetLayout* layout) {
    (void)allocator;
    if (!layout) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto dsl = std::make_unique<VkDescriptorSetLayoutImpl>();
    if (!dsl->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *layout = register_descriptor_set_layout(std::move(dsl));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_descriptor_set_layout(KxDescriptorSetLayout layout,
                                                   const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!layout) return;

    auto* dsl = find_vk_descriptor_set_layout(layout);
    if (dsl) {
        auto& funcs = functions();
        if (dsl->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyDescriptorSetLayout(static_cast<VkDevice>(vk_device_->getHandle()), dsl->handle(), nullptr);
        }
    }
    unregister_descriptor_set_layout(layout);
}

// ============================================================================
// Descriptor Pool
// ============================================================================

KxResult VkDriverDevice::create_descriptor_pool(const KxDescriptorPoolCreateInfo* create_info,
                                                const KxAllocationCallbacks* allocator,
                                                KxDescriptorPool* pool) {
    (void)allocator;
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto dp = std::make_unique<VkDescriptorPoolImpl>();
    if (!dp->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *pool = register_descriptor_pool(std::move(dp));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_descriptor_pool(KxDescriptorPool pool,
                                             const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!pool) return;

    auto* dp = find_vk_descriptor_pool(pool);
    if (dp) {
        auto& funcs = functions();
        if (dp->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroyDescriptorPool(static_cast<VkDevice>(vk_device_->getHandle()), dp->handle(), nullptr);
        }
    }
    unregister_descriptor_pool(pool);
}

KxResult VkDriverDevice::reset_descriptor_pool(KxDescriptorPool pool,
                                               KxDescriptorPoolResetFlags flags) {
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;

    auto* dp = find_vk_descriptor_pool(pool);
    if (!dp) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    VkResult result = funcs.vkResetDescriptorPool(static_cast<VkDevice>(vk_device_->getHandle()),
                                                   dp->handle(), static_cast<VkDescriptorPoolResetFlags>(flags));
    return result == VK_SUCCESS ? KX_SUCCESS : KX_ERROR_FEATURE_NOT_PRESENT;
}

// ============================================================================
// Descriptor Set
// ============================================================================

KxResult VkDriverDevice::allocate_descriptor_sets(const KxDescriptorSetAllocateInfo* allocate_info,
                                                  KxDescriptorSet* sets) {
    if (!sets) return KX_ERROR_INVALID_ARGUMENT;
    if (!allocate_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();

    auto* pool = find_vk_descriptor_pool(allocate_info->descriptorPool);
    if (!pool) return KX_ERROR_INVALID_ARGUMENT;

    uint32_t count = allocate_info->descriptorSetCount;

    // Get layouts
    std::vector<VkDescriptorSetLayout> layouts;
    layouts.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        auto* dsl = find_vk_descriptor_set_layout(allocate_info->pSetLayouts[i]);
        if (dsl) {
            layouts.push_back(dsl->handle());
        }
    }

    VkDescriptorSetAllocateInfo vk_info{};
    vk_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vk_info.descriptorPool = pool->handle();
    vk_info.descriptorSetCount = count;
    vk_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> vk_sets(count);
    VkResult result = funcs.vkAllocateDescriptorSets(static_cast<VkDevice>(vk_device_->getHandle()), &vk_info, vk_sets.data());
    if (result != VK_SUCCESS) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    for (uint32_t i = 0; i < count; ++i) {
        auto set = std::make_unique<VkDescriptorSetImpl>();
        set->set_pool(pool->handle());
        if (i < layouts.size()) {
            set->set_layout(layouts[i]);
        }
        // We need to store the VkDescriptorSet handle
        // Since VkDescriptorSetImpl stores it internally, we'll use a different approach
        // For now, register and then set the handle
        sets[i] = register_descriptor_set(std::move(set));
        // TODO: Set the actual VkDescriptorSet handle
    }

    return KX_SUCCESS;
}

KxResult VkDriverDevice::free_descriptor_sets(KxDescriptorPool pool,
                                              uint32_t count,
                                              const KxDescriptorSet* sets) {
    if (!pool || !sets || count == 0) return KX_ERROR_INVALID_ARGUMENT;

    auto* dp = find_vk_descriptor_pool(pool);
    if (!dp) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    std::vector<VkDescriptorSet> vk_sets;
    vk_sets.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        auto* set = find_vk_descriptor_set(sets[i]);
        if (set) {
            vk_sets.push_back(set->handle());
        }
    }

    if (!vk_sets.empty()) {
        VkResult result = funcs.vkFreeDescriptorSets(static_cast<VkDevice>(vk_device_->getHandle()),
                                                      dp->handle(), static_cast<uint32_t>(vk_sets.size()), vk_sets.data());
        if (result != VK_SUCCESS) {
            return KX_ERROR_FEATURE_NOT_PRESENT;
        }
    }

    for (uint32_t i = 0; i < count; ++i) {
        unregister_descriptor_set(sets[i]);
    }

    return KX_SUCCESS;
}

void VkDriverDevice::update_descriptor_sets(uint32_t descriptor_write_count,
                                            const KxWriteDescriptorSet* descriptor_writes,
                                            uint32_t descriptor_copy_count,
                                            const KxCopyDescriptorSet* descriptor_copies) {
    if (descriptor_write_count == 0 && descriptor_copy_count == 0) return;

    auto& funcs = functions();

    // Build VkWriteDescriptorSet array
    std::vector<VkWriteDescriptorSet> vk_writes;
    vk_writes.reserve(descriptor_write_count);

    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkBufferView> texel_buffer_views;

    for (uint32_t i = 0; i < descriptor_write_count; ++i) {
        const KxWriteDescriptorSet& kx_write = descriptor_writes[i];
        VkWriteDescriptorSet vk_write{};
        vk_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vk_write.dstSet = reinterpret_cast<VkDescriptorSet>(kx_write.dstSet);
        vk_write.dstBinding = kx_write.dstBinding;
        vk_write.dstArrayElement = kx_write.dstArrayElement;
        vk_write.descriptorCount = kx_write.descriptorCount;
        vk_write.descriptorType = static_cast<VkDescriptorType>(kx_write.descriptorType);

        if (kx_write.pImageInfo) {
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                const KxDescriptorImageInfo& kx_img = kx_write.pImageInfo[j];
                VkDescriptorImageInfo vk_img{};
                vk_img.sampler = reinterpret_cast<VkSampler>(kx_img.sampler);
                vk_img.imageView = reinterpret_cast<VkImageView>(kx_img.imageView);
                vk_img.imageLayout = static_cast<VkImageLayout>(kx_img.imageLayout);
                image_infos.push_back(vk_img);
            }
            vk_write.pImageInfo = &image_infos[image_infos.size() - kx_write.descriptorCount];
        }

        if (kx_write.pBufferInfo) {
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                const KxDescriptorBufferInfo& kx_buf = kx_write.pBufferInfo[j];
                VkDescriptorBufferInfo vk_buf{};
                vk_buf.buffer = reinterpret_cast<VkBuffer>(kx_buf.buffer);
                vk_buf.offset = kx_buf.offset;
                vk_buf.range = kx_buf.range;
                buffer_infos.push_back(vk_buf);
            }
            vk_write.pBufferInfo = &buffer_infos[buffer_infos.size() - kx_write.descriptorCount];
        }

        if (kx_write.pTexelBufferView) {
            const VkBufferView* texel_views = reinterpret_cast<const VkBufferView*>(kx_write.pTexelBufferView);
            for (uint32_t j = 0; j < kx_write.descriptorCount; ++j) {
                texel_buffer_views.push_back(texel_views[j]);
            }
            vk_write.pTexelBufferView = &texel_buffer_views[texel_buffer_views.size() - kx_write.descriptorCount];
        }

        vk_writes.push_back(vk_write);
    }

    // Build VkCopyDescriptorSet array
    std::vector<VkCopyDescriptorSet> vk_copies;
    vk_copies.reserve(descriptor_copy_count);
    for (uint32_t i = 0; i < descriptor_copy_count; ++i) {
        const KxCopyDescriptorSet& kx_copy = descriptor_copies[i];
        VkCopyDescriptorSet vk_copy{};
        vk_copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        vk_copy.srcSet = reinterpret_cast<VkDescriptorSet>(kx_copy.srcSet);
        vk_copy.srcBinding = kx_copy.srcBinding;
        vk_copy.srcArrayElement = kx_copy.srcArrayElement;
        vk_copy.dstSet = reinterpret_cast<VkDescriptorSet>(kx_copy.dstSet);
        vk_copy.dstBinding = kx_copy.dstBinding;
        vk_copy.dstArrayElement = kx_copy.dstArrayElement;
        vk_copy.descriptorCount = kx_copy.descriptorCount;
        vk_copies.push_back(vk_copy);
    }

    funcs.vkUpdateDescriptorSets(static_cast<VkDevice>(vk_device_->getHandle()),
                                 static_cast<uint32_t>(vk_writes.size()),
                                 vk_writes.data(),
                                 static_cast<uint32_t>(vk_copies.size()),
                                 vk_copies.data());
}

// ============================================================================
// Sampler
// ============================================================================

KxResult VkDriverDevice::create_sampler(const KxSamplerCreateInfo* create_info,
                                        const KxAllocationCallbacks* allocator,
                                        KxSampler* sampler) {
    (void)allocator;
    if (!sampler) return KX_ERROR_INVALID_ARGUMENT;
    if (!create_info) return KX_ERROR_INVALID_ARGUMENT;

    auto& funcs = functions();
    auto samp = std::make_unique<VkSamplerImpl>();
    if (!samp->initialize(funcs, static_cast<VkDevice>(vk_device_->getHandle()), create_info)) {
        return KX_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    *sampler = register_sampler(std::move(samp));
    return KX_SUCCESS;
}

void VkDriverDevice::destroy_sampler(KxSampler sampler,
                                     const KxAllocationCallbacks* allocator) {
    (void)allocator;
    if (!sampler) return;

    auto* samp = find_vk_sampler(sampler);
    if (samp) {
        auto& funcs = functions();
        if (samp->handle() != VK_NULL_HANDLE) {
            funcs.vkDestroySampler(static_cast<VkDevice>(vk_device_->getHandle()), samp->handle(), nullptr);
        }
    }
    unregister_sampler(sampler);
}

// ============================================================================
// Object Registration
// ============================================================================

KxShaderModule VkDriverDevice::register_shader_module(std::unique_ptr<VkShaderModuleImpl> module) {
    uint64_t handle = next_handle_++;
    shader_modules_[handle] = std::move(module);
    return reinterpret_cast<KxShaderModule>(handle);
}

KxPipelineLayout VkDriverDevice::register_pipeline_layout(std::unique_ptr<VkPipelineLayoutImpl> layout) {
    uint64_t handle = next_handle_++;
    pipeline_layouts_[handle] = std::move(layout);
    return reinterpret_cast<KxPipelineLayout>(handle);
}

KxPipeline VkDriverDevice::register_pipeline(std::unique_ptr<VkPipelineImpl> pipeline) {
    uint64_t handle = next_handle_++;
    pipelines_[handle] = std::move(pipeline);
    return reinterpret_cast<KxPipeline>(handle);
}

KxDescriptorSetLayout VkDriverDevice::register_descriptor_set_layout(std::unique_ptr<VkDescriptorSetLayoutImpl> layout) {
    uint64_t handle = next_handle_++;
    descriptor_set_layouts_[handle] = std::move(layout);
    return reinterpret_cast<KxDescriptorSetLayout>(handle);
}

KxDescriptorPool VkDriverDevice::register_descriptor_pool(std::unique_ptr<VkDescriptorPoolImpl> pool) {
    uint64_t handle = next_handle_++;
    descriptor_pools_[handle] = std::move(pool);
    return reinterpret_cast<KxDescriptorPool>(handle);
}

KxDescriptorSet VkDriverDevice::register_descriptor_set(std::unique_ptr<VkDescriptorSetImpl> set) {
    uint64_t handle = next_handle_++;
    descriptor_sets_[handle] = std::move(set);
    return reinterpret_cast<KxDescriptorSet>(handle);
}

KxCommandPool VkDriverDevice::register_command_pool(std::unique_ptr<VkCommandPoolImpl> pool) {
    uint64_t handle = next_handle_++;
    command_pools_[handle] = std::move(pool);
    return reinterpret_cast<KxCommandPool>(handle);
}

KxCommandBuffer VkDriverDevice::register_command_buffer(std::unique_ptr<VkCommandBufferImpl> buffer) {
    uint64_t handle = next_handle_++;
    command_buffers_[handle] = std::move(buffer);
    return reinterpret_cast<KxCommandBuffer>(handle);
}

KxRenderPass VkDriverDevice::register_render_pass(std::unique_ptr<VkRenderPassImpl> render_pass) {
    uint64_t handle = next_handle_++;
    render_passes_[handle] = std::move(render_pass);
    return reinterpret_cast<KxRenderPass>(handle);
}

KxFramebuffer VkDriverDevice::register_framebuffer(std::unique_ptr<VkFramebufferImpl> framebuffer) {
    uint64_t handle = next_handle_++;
    framebuffers_[handle] = std::move(framebuffer);
    return reinterpret_cast<KxFramebuffer>(handle);
}

KxBuffer VkDriverDevice::register_buffer(std::unique_ptr<VkBufferImpl> buffer) {
    uint64_t handle = next_handle_++;
    buffers_[handle] = std::move(buffer);
    return reinterpret_cast<KxBuffer>(handle);
}

KxImage VkDriverDevice::register_image(std::unique_ptr<VkImageImpl> image) {
    uint64_t handle = next_handle_++;
    images_[handle] = std::move(image);
    return reinterpret_cast<KxImage>(handle);
}

KxImageView VkDriverDevice::register_image_view(std::unique_ptr<VkImageViewImpl> view) {
    uint64_t handle = next_handle_++;
    image_views_[handle] = std::move(view);
    return reinterpret_cast<KxImageView>(handle);
}

KxSampler VkDriverDevice::register_sampler(std::unique_ptr<VkSamplerImpl> sampler) {
    uint64_t handle = next_handle_++;
    samplers_[handle] = std::move(sampler);
    return reinterpret_cast<KxSampler>(handle);
}

KxDeviceMemory VkDriverDevice::register_memory(std::unique_ptr<VkMemoryImpl> memory) {
    uint64_t handle = next_handle_++;
    memories_[handle] = std::move(memory);
    return reinterpret_cast<KxDeviceMemory>(handle);
}

KxFence VkDriverDevice::register_fence(std::unique_ptr<VkFenceImpl> fence) {
    uint64_t handle = next_handle_++;
    fences_[handle] = std::move(fence);
    return reinterpret_cast<KxFence>(handle);
}

KxSemaphore VkDriverDevice::register_semaphore(std::unique_ptr<VkSemaphoreImpl> semaphore) {
    uint64_t handle = next_handle_++;
    semaphores_[handle] = std::move(semaphore);
    return reinterpret_cast<KxSemaphore>(handle);
}

KxQueue VkDriverDevice::register_queue(std::unique_ptr<VkQueueImpl> queue) {
    uint64_t handle = next_handle_++;
    queues_[handle] = std::move(queue);
    return reinterpret_cast<KxQueue>(handle);
}

// ============================================================================
// Object Finders
// ============================================================================

VkShaderModuleImpl* VkDriverDevice::find_vk_shader_module(KxShaderModule module) {
    auto it = shader_modules_.find(reinterpret_cast<uint64_t>(module));
    return it != shader_modules_.end() ? it->second.get() : nullptr;
}

VkPipelineLayoutImpl* VkDriverDevice::find_vk_pipeline_layout(KxPipelineLayout layout) {
    auto it = pipeline_layouts_.find(reinterpret_cast<uint64_t>(layout));
    return it != pipeline_layouts_.end() ? it->second.get() : nullptr;
}

VkPipelineImpl* VkDriverDevice::find_vk_pipeline(KxPipeline pipeline) {
    auto it = pipelines_.find(reinterpret_cast<uint64_t>(pipeline));
    return it != pipelines_.end() ? it->second.get() : nullptr;
}

VkDescriptorSetLayoutImpl* VkDriverDevice::find_vk_descriptor_set_layout(KxDescriptorSetLayout layout) {
    auto it = descriptor_set_layouts_.find(reinterpret_cast<uint64_t>(layout));
    return it != descriptor_set_layouts_.end() ? it->second.get() : nullptr;
}

VkDescriptorPoolImpl* VkDriverDevice::find_vk_descriptor_pool(KxDescriptorPool pool) {
    auto it = descriptor_pools_.find(reinterpret_cast<uint64_t>(pool));
    return it != descriptor_pools_.end() ? it->second.get() : nullptr;
}

VkDescriptorSetImpl* VkDriverDevice::find_vk_descriptor_set(KxDescriptorSet set) {
    auto it = descriptor_sets_.find(reinterpret_cast<uint64_t>(set));
    return it != descriptor_sets_.end() ? it->second.get() : nullptr;
}

VkCommandPoolImpl* VkDriverDevice::find_vk_command_pool(KxCommandPool pool) {
    auto it = command_pools_.find(reinterpret_cast<uint64_t>(pool));
    return it != command_pools_.end() ? it->second.get() : nullptr;
}

VkCommandBufferImpl* VkDriverDevice::find_vk_command_buffer(KxCommandBuffer buffer) {
    auto it = command_buffers_.find(reinterpret_cast<uint64_t>(buffer));
    return it != command_buffers_.end() ? it->second.get() : nullptr;
}

VkRenderPassImpl* VkDriverDevice::find_vk_render_pass(KxRenderPass render_pass) {
    auto it = render_passes_.find(reinterpret_cast<uint64_t>(render_pass));
    return it != render_passes_.end() ? it->second.get() : nullptr;
}

VkFramebufferImpl* VkDriverDevice::find_vk_framebuffer(KxFramebuffer framebuffer) {
    auto it = framebuffers_.find(reinterpret_cast<uint64_t>(framebuffer));
    return it != framebuffers_.end() ? it->second.get() : nullptr;
}

VkBufferImpl* VkDriverDevice::find_vk_buffer(KxBuffer buffer) {
    auto it = buffers_.find(reinterpret_cast<uint64_t>(buffer));
    return it != buffers_.end() ? it->second.get() : nullptr;
}

VkImageImpl* VkDriverDevice::find_vk_image(KxImage image) {
    auto it = images_.find(reinterpret_cast<uint64_t>(image));
    return it != images_.end() ? it->second.get() : nullptr;
}

VkImageViewImpl* VkDriverDevice::find_vk_image_view(KxImageView view) {
    auto it = image_views_.find(reinterpret_cast<uint64_t>(view));
    return it != image_views_.end() ? it->second.get() : nullptr;
}

VkSamplerImpl* VkDriverDevice::find_vk_sampler(KxSampler sampler) {
    auto it = samplers_.find(reinterpret_cast<uint64_t>(sampler));
    return it != samplers_.end() ? it->second.get() : nullptr;
}

VkMemoryImpl* VkDriverDevice::find_vk_memory(KxDeviceMemory memory) {
    auto it = memories_.find(reinterpret_cast<uint64_t>(memory));
    return it != memories_.end() ? it->second.get() : nullptr;
}

VkFenceImpl* VkDriverDevice::find_vk_fence(KxFence fence) {
    auto it = fences_.find(reinterpret_cast<uint64_t>(fence));
    return it != fences_.end() ? it->second.get() : nullptr;
}

VkSemaphoreImpl* VkDriverDevice::find_vk_semaphore(KxSemaphore semaphore) {
    auto it = semaphores_.find(reinterpret_cast<uint64_t>(semaphore));
    return it != semaphores_.end() ? it->second.get() : nullptr;
}

VkQueueImpl* VkDriverDevice::find_vk_queue(KxQueue queue) {
    auto it = queues_.find(reinterpret_cast<uint64_t>(queue));
    return it != queues_.end() ? it->second.get() : nullptr;
}

// ============================================================================
// Object Unregistration
// ============================================================================

void VkDriverDevice::unregister_shader_module(KxShaderModule module) {
    shader_modules_.erase(reinterpret_cast<uint64_t>(module));
}

void VkDriverDevice::unregister_pipeline_layout(KxPipelineLayout layout) {
    pipeline_layouts_.erase(reinterpret_cast<uint64_t>(layout));
}

void VkDriverDevice::unregister_pipeline(KxPipeline pipeline) {
    pipelines_.erase(reinterpret_cast<uint64_t>(pipeline));
}

void VkDriverDevice::unregister_descriptor_set_layout(KxDescriptorSetLayout layout) {
    descriptor_set_layouts_.erase(reinterpret_cast<uint64_t>(layout));
}

void VkDriverDevice::unregister_descriptor_pool(KxDescriptorPool pool) {
    descriptor_pools_.erase(reinterpret_cast<uint64_t>(pool));
}

void VkDriverDevice::unregister_descriptor_set(KxDescriptorSet set) {
    descriptor_sets_.erase(reinterpret_cast<uint64_t>(set));
}

void VkDriverDevice::unregister_command_pool(KxCommandPool pool) {
    command_pools_.erase(reinterpret_cast<uint64_t>(pool));
}

void VkDriverDevice::unregister_command_buffer(KxCommandBuffer buffer) {
    command_buffers_.erase(reinterpret_cast<uint64_t>(buffer));
}

void VkDriverDevice::unregister_render_pass(KxRenderPass render_pass) {
    render_passes_.erase(reinterpret_cast<uint64_t>(render_pass));
}

void VkDriverDevice::unregister_framebuffer(KxFramebuffer framebuffer) {
    framebuffers_.erase(reinterpret_cast<uint64_t>(framebuffer));
}

void VkDriverDevice::unregister_buffer(KxBuffer buffer) {
    buffers_.erase(reinterpret_cast<uint64_t>(buffer));
}

void VkDriverDevice::unregister_image(KxImage image) {
    images_.erase(reinterpret_cast<uint64_t>(image));
}

void VkDriverDevice::unregister_image_view(KxImageView view) {
    image_views_.erase(reinterpret_cast<uint64_t>(view));
}

void VkDriverDevice::unregister_sampler(KxSampler sampler) {
    samplers_.erase(reinterpret_cast<uint64_t>(sampler));
}

void VkDriverDevice::unregister_memory(KxDeviceMemory memory) {
    memories_.erase(reinterpret_cast<uint64_t>(memory));
}

void VkDriverDevice::unregister_fence(KxFence fence) {
    fences_.erase(reinterpret_cast<uint64_t>(fence));
}

void VkDriverDevice::unregister_semaphore(KxSemaphore semaphore) {
    semaphores_.erase(reinterpret_cast<uint64_t>(semaphore));
}

void VkDriverDevice::unregister_queue(KxQueue queue) {
    queues_.erase(reinterpret_cast<uint64_t>(queue));
}

} // namespace kx