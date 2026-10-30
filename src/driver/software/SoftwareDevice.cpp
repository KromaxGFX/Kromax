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

} /* namespace kx */