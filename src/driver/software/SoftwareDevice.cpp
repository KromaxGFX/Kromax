/*
 * SoftwareDevice.cpp
 *
 * Software device: main class, find/register/unregister methods.
 */

#include "SoftwareDevice.h"

#include "SoftwareCommandBuffer.h"
#include "SoftwareRenderPass.h"
#include "SoftwareFramebuffer.h"

namespace kx {

SoftwareDevice::SoftwareDevice(PhysicalDevice* physical_device)
    : physical_device_(physical_device) {}

/* Object lookups (public handle -> internal object) */
SoftwareMemory* SoftwareDevice::find_memory(KxDeviceMemory mem) noexcept {
    for (auto& m : memories_) {
        if (reinterpret_cast<KxDeviceMemory>(m.get()) == mem) return m.get();
    }
    return nullptr;
}

SoftwareBuffer* SoftwareDevice::find_buffer(KxBuffer buf) noexcept {
    for (auto& b : buffers_) {
        if (reinterpret_cast<KxBuffer>(b.get()) == buf) return b.get();
    }
    return nullptr;
}

SoftwareImage* SoftwareDevice::find_image(KxImage img) noexcept {
    for (auto& i : images_) {
        if (reinterpret_cast<KxImage>(i.get()) == img) return i.get();
    }
    return nullptr;
}

SoftwareImageView* SoftwareDevice::find_image_view(KxImageView view) noexcept {
    for (auto& v : image_views_) {
        if (reinterpret_cast<KxImageView>(v.get()) == view) return v.get();
    }
    return nullptr;
}

SoftwareRenderPass* SoftwareDevice::find_render_pass(KxRenderPass rp) noexcept {
    for (auto& r : render_passes_) {
        if (reinterpret_cast<KxRenderPass>(r.get()) == rp) return r.get();
    }
    return nullptr;
}

SoftwareFramebuffer* SoftwareDevice::find_framebuffer(KxFramebuffer fb) noexcept {
    for (auto& f : framebuffers_) {
        if (reinterpret_cast<KxFramebuffer>(f.get()) == fb) return f.get();
    }
    return nullptr;
}

SoftwareCommandPool* SoftwareDevice::find_command_pool(KxCommandPool pool) noexcept {
    for (auto& p : command_pools_) {
        if (reinterpret_cast<KxCommandPool>(p.get()) == pool) return p.get();
    }
    return nullptr;
}

SoftwareCommandBuffer* SoftwareDevice::find_command_buffer(KxCommandBuffer cb) noexcept {
    for (auto& c : command_buffers_) {
        if (reinterpret_cast<KxCommandBuffer>(c.get()) == cb) return c.get();
    }
    return nullptr;
}

/* Object registration */
KxDeviceMemory SoftwareDevice::register_memory(std::unique_ptr<SoftwareMemory> mem) {
    KxDeviceMemory handle = reinterpret_cast<KxDeviceMemory>(mem.get());
    memories_.push_back(std::move(mem));
    return handle;
}

KxBuffer SoftwareDevice::register_buffer(std::unique_ptr<SoftwareBuffer> buf) {
    KxBuffer handle = reinterpret_cast<KxBuffer>(buf.get());
    buffers_.push_back(std::move(buf));
    return handle;
}

KxImage SoftwareDevice::register_image(std::unique_ptr<SoftwareImage> img) {
    KxImage handle = reinterpret_cast<KxImage>(img.get());
    images_.push_back(std::move(img));
    return handle;
}

KxCommandPool SoftwareDevice::register_command_pool(std::unique_ptr<SoftwareCommandPool> pool) {
    KxCommandPool handle = reinterpret_cast<KxCommandPool>(pool.get());
    command_pools_.push_back(std::move(pool));
    return handle;
}

KxCommandBuffer SoftwareDevice::register_command_buffer(
    std::unique_ptr<SoftwareCommandBuffer> cmd) {
    KxCommandBuffer handle = reinterpret_cast<KxCommandBuffer>(cmd.get());
    command_buffers_.push_back(std::move(cmd));
    return handle;
}

KxImageView SoftwareDevice::register_image_view(std::unique_ptr<SoftwareImageView> view) {
    KxImageView handle = reinterpret_cast<KxImageView>(view.get());
    image_views_.push_back(std::move(view));
    return handle;
}

KxRenderPass SoftwareDevice::register_render_pass(std::unique_ptr<SoftwareRenderPass> rp) {
    KxRenderPass handle = reinterpret_cast<KxRenderPass>(rp.get());
    render_passes_.push_back(std::move(rp));
    return handle;
}

KxFramebuffer SoftwareDevice::register_framebuffer(std::unique_ptr<SoftwareFramebuffer> fb) {
    KxFramebuffer handle = reinterpret_cast<KxFramebuffer>(fb.get());
    framebuffers_.push_back(std::move(fb));
    return handle;
}

/* Object unregistration */
void SoftwareDevice::unregister_memory(KxDeviceMemory mem) {
    for (auto it = memories_.begin(); it != memories_.end(); ++it) {
        if (reinterpret_cast<KxDeviceMemory>(it->get()) == mem) {
            memories_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_buffer(KxBuffer buf) {
    for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
        if (reinterpret_cast<KxBuffer>(it->get()) == buf) {
            buffers_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_image(KxImage img) {
    for (auto it = images_.begin(); it != images_.end(); ++it) {
        if (reinterpret_cast<KxImage>(it->get()) == img) {
            images_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_image_view(KxImageView view) {
    for (auto it = image_views_.begin(); it != image_views_.end(); ++it) {
        if (reinterpret_cast<KxImageView>(it->get()) == view) {
            image_views_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_render_pass(KxRenderPass rp) {
    for (auto it = render_passes_.begin(); it != render_passes_.end(); ++it) {
        if (reinterpret_cast<KxRenderPass>(it->get()) == rp) {
            render_passes_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_framebuffer(KxFramebuffer fb) {
    for (auto it = framebuffers_.begin(); it != framebuffers_.end(); ++it) {
        if (reinterpret_cast<KxFramebuffer>(it->get()) == fb) {
            framebuffers_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_command_pool(KxCommandPool pool) {
    for (auto it = command_pools_.begin(); it != command_pools_.end(); ++it) {
        if (reinterpret_cast<KxCommandPool>(it->get()) == pool) {
            command_pools_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_command_buffer(KxCommandBuffer cb) {
    for (auto it = command_buffers_.begin(); it != command_buffers_.end(); ++it) {
        if (reinterpret_cast<KxCommandBuffer>(it->get()) == cb) {
            command_buffers_.erase(it);
            return;
        }
    }
}

/* Descriptor set layout */
SoftwareDescriptorSetLayout* SoftwareDevice::find_descriptor_set_layout(KxDescriptorSetLayout layout) noexcept {
    for (auto& l : descriptor_set_layouts_) {
        if (reinterpret_cast<KxDescriptorSetLayout>(l.get()) == layout) return l.get();
    }
    return nullptr;
}

SoftwareDescriptorPool* SoftwareDevice::find_descriptor_pool(KxDescriptorPool pool) noexcept {
    for (auto& p : descriptor_pools_) {
        if (reinterpret_cast<KxDescriptorPool>(p.get()) == pool) return p.get();
    }
    return nullptr;
}

SoftwareDescriptorSet* SoftwareDevice::find_descriptor_set(KxDescriptorSet set) noexcept {
    for (auto& s : descriptor_sets_) {
        if (reinterpret_cast<KxDescriptorSet>(s.get()) == set) return s.get();
    }
    return nullptr;
}

SoftwareSampler* SoftwareDevice::find_sampler(KxSampler sampler) noexcept {
    for (auto& s : samplers_) {
        if (reinterpret_cast<KxSampler>(s.get()) == sampler) return s.get();
    }
    return nullptr;
}

/* Descriptor set layout registration */
KxDescriptorSetLayout SoftwareDevice::register_descriptor_set_layout(std::unique_ptr<SoftwareDescriptorSetLayout> layout) {
    KxDescriptorSetLayout handle = reinterpret_cast<KxDescriptorSetLayout>(layout.get());
    descriptor_set_layouts_.push_back(std::move(layout));
    return handle;
}

KxDescriptorPool SoftwareDevice::register_descriptor_pool(std::unique_ptr<SoftwareDescriptorPool> pool) {
    KxDescriptorPool handle = reinterpret_cast<KxDescriptorPool>(pool.get());
    descriptor_pools_.push_back(std::move(pool));
    return handle;
}

KxDescriptorSet SoftwareDevice::register_descriptor_set(std::unique_ptr<SoftwareDescriptorSet> set) {
    KxDescriptorSet handle = reinterpret_cast<KxDescriptorSet>(set.get());
    descriptor_sets_.push_back(std::move(set));
    return handle;
}

KxSampler SoftwareDevice::register_sampler(std::unique_ptr<SoftwareSampler> sampler) {
    KxSampler handle = reinterpret_cast<KxSampler>(sampler.get());
    samplers_.push_back(std::move(sampler));
    return handle;
}

/* Descriptor unregistration */
void SoftwareDevice::unregister_descriptor_set_layout(KxDescriptorSetLayout layout) {
    for (auto it = descriptor_set_layouts_.begin(); it != descriptor_set_layouts_.end(); ++it) {
        if (reinterpret_cast<KxDescriptorSetLayout>(it->get()) == layout) {
            descriptor_set_layouts_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_descriptor_pool(KxDescriptorPool pool) {
    for (auto it = descriptor_pools_.begin(); it != descriptor_pools_.end(); ++it) {
        if (reinterpret_cast<KxDescriptorPool>(it->get()) == pool) {
            descriptor_pools_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_descriptor_set(KxDescriptorSet set) {
    for (auto it = descriptor_sets_.begin(); it != descriptor_sets_.end(); ++it) {
        if (reinterpret_cast<KxDescriptorSet>(it->get()) == set) {
            descriptor_sets_.erase(it);
            return;
        }
    }
}

void SoftwareDevice::unregister_sampler(KxSampler sampler) {
    for (auto it = samplers_.begin(); it != samplers_.end(); ++it) {
        if (reinterpret_cast<KxSampler>(it->get()) == sampler) {
            samplers_.erase(it);
            return;
        }
    }
}

/* Descriptor set layout */
KxResult SoftwareDevice::create_descriptor_set_layout(const KxDescriptorSetLayoutCreateInfo* info,
                                                      const KxAllocationCallbacks* alloc,
                                                      KxDescriptorSetLayout* layout) {
    (void)alloc;
    if (!info || !layout) return KX_ERROR_INITIALIZATION_FAILED;
    auto ptr = std::make_unique<SoftwareDescriptorSetLayout>(info);
    *layout = register_descriptor_set_layout(std::move(ptr));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_descriptor_set_layout(KxDescriptorSetLayout layout,
                                                    const KxAllocationCallbacks* alloc) {
    (void)alloc;
    unregister_descriptor_set_layout(layout);
}

/* Descriptor pool */
KxResult SoftwareDevice::create_descriptor_pool(const KxDescriptorPoolCreateInfo* info,
                                                const KxAllocationCallbacks* alloc,
                                                KxDescriptorPool* pool) {
    (void)alloc;
    if (!info || !pool) return KX_ERROR_INITIALIZATION_FAILED;
    auto ptr = std::make_unique<SoftwareDescriptorPool>(info);
    *pool = register_descriptor_pool(std::move(ptr));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_descriptor_pool(KxDescriptorPool pool,
                                             const KxAllocationCallbacks* alloc) {
    (void)alloc;
    unregister_descriptor_pool(pool);
}

KxResult SoftwareDevice::reset_descriptor_pool(KxDescriptorPool pool,
                                               KxDescriptorPoolResetFlags flags) {
    (void)flags;
    auto* p = find_descriptor_pool(pool);
    if (!p) return KX_ERROR_DEVICE_LOST;
    p->reset();
    return KX_SUCCESS;
}

/* Descriptor set */
KxResult SoftwareDevice::allocate_descriptor_sets(const KxDescriptorSetAllocateInfo* info,
                                                  KxDescriptorSet* sets) {
    if (!info || !sets) return KX_ERROR_INITIALIZATION_FAILED;
    if (info->descriptorSetCount == 0) return KX_SUCCESS;

    auto* pool = find_descriptor_pool(info->descriptorPool);
    if (!pool) return KX_ERROR_DEVICE_LOST;

    for (uint32_t i = 0; i < info->descriptorSetCount; ++i) {
        auto* layout = find_descriptor_set_layout(info->pSetLayouts[i]);
        if (!layout) return KX_ERROR_DEVICE_LOST;

        if (!pool->can_allocate(layout)) {
            return KX_ERROR_OUT_OF_POOL_MEMORY;
        }

        auto set_ptr = std::make_unique<SoftwareDescriptorSet>();
        set_ptr->init(layout, this);
        sets[i] = register_descriptor_set(std::move(set_ptr));
        pool->reserve_for_set(layout);
    }
    return KX_SUCCESS;
}

KxResult SoftwareDevice::free_descriptor_sets(KxDescriptorPool pool,
                                              uint32_t count,
                                              const KxDescriptorSet* sets) {
    auto* p = find_descriptor_pool(pool);
    if (!p) return KX_ERROR_DEVICE_LOST;
    if (!p->free_descriptor_set_allowed()) return KX_ERROR_FEATURE_NOT_PRESENT;

    for (uint32_t i = 0; i < count; ++i) {
        auto* set = find_descriptor_set(sets[i]);
        if (set) {
            p->release_from_set(set->layout());
            unregister_descriptor_set(sets[i]);
        }
    }
    return KX_SUCCESS;
}

void SoftwareDevice::update_descriptor_sets(uint32_t descriptorWriteCount,
                                            const KxWriteDescriptorSet* pDescriptorWrites,
                                            uint32_t descriptorCopyCount,
                                            const KxCopyDescriptorSet* pDescriptorCopies) {
    (void)descriptorCopyCount;
    (void)pDescriptorCopies; // Not implemented yet

    for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
        const auto& write = pDescriptorWrites[i];
        auto* set = find_descriptor_set(write.dstSet);
        if (set) {
            set->update(write);
        }
    }
}

/* Sampler */
KxResult SoftwareDevice::create_sampler(const KxSamplerCreateInfo* info,
                                        const KxAllocationCallbacks* alloc,
                                        KxSampler* sampler) {
    (void)alloc;
    if (!info || !sampler) return KX_ERROR_INITIALIZATION_FAILED;
    auto ptr = std::make_unique<SoftwareSampler>(info);
    *sampler = register_sampler(std::move(ptr));
    return KX_SUCCESS;
}

void SoftwareDevice::destroy_sampler(KxSampler sampler,
                                     const KxAllocationCallbacks* alloc) {
    (void)alloc;
    unregister_sampler(sampler);
}

} /* namespace kx */