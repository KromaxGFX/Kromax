#include "SoftwareFramebuffer.h"

namespace kx {

SoftwareFramebuffer::SoftwareFramebuffer(
    const KxFramebufferCreateInfo* create_info,
    const std::vector<SoftwareImageView*>& attachments,
    SoftwareRenderPass* render_pass)
    : width_(create_info->width)
    , height_(create_info->height)
    , layers_(create_info->layers)
    , render_pass_(render_pass)
    , attachments_(attachments) {}

} /* namespace kx */