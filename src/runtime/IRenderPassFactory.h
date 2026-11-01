#ifndef KROMAX_SRC_RUNTIME_IRENDERPASSFACTORY_H
#define KROMAX_SRC_RUNTIME_IRENDERPASSFACTORY_H

/*
 * IRenderPassFactory.h
 * Render pass and framebuffer creation and destruction.
 */

#include "kx_core.h"
#include "Expected.h"

#include <cstdint>

namespace kx {

class RenderPass;
class Framebuffer;
class ImageView;

struct RenderPassCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    const void* pNext = nullptr;
    KxRenderPassCreateFlags flags = 0;
    uint32_t attachmentCount = 0;
    const KxAttachmentDescription* pAttachments = nullptr;
    uint32_t subpassCount = 0;
    const KxSubpassDescription* pSubpasses = nullptr;
    uint32_t dependencyCount = 0;
    const KxSubpassDependency* pDependencies = nullptr;
};

struct FramebufferCreateInfo {
    KxStructureType sType = KX_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    const void* pNext = nullptr;
    KxFramebufferCreateFlags flags = 0;
    RenderPass renderPass = nullptr;
    uint32_t attachmentCount = 0;
    const ImageView* pAttachments = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t layers = 1;
};

class IRenderPassFactory {
public:
    virtual ~IRenderPassFactory() = default;

    virtual KxResult createRenderPass(const RenderPassCreateInfo& info, RenderPass** outRenderPass) = 0;
    virtual void destroyRenderPass(RenderPass renderPass) = 0;
    virtual KxResult createFramebuffer(const FramebufferCreateInfo& info, Framebuffer** outFramebuffer) = 0;
    virtual void destroyFramebuffer(Framebuffer framebuffer) = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IRENDERPASSFACTORY_H