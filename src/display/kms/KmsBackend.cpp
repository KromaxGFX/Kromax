#include "KmsBackend.h"

#ifdef KX_HAVE_LIBDRM

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "Logging.h"

namespace kx {

KxResult KmsBackend::initialize(const void* /*pCreateInfo*/) {
    log_info("DRM/KMS display backend initialized (libdrm)");
    return KX_SUCCESS;
}

KxResult KmsBackend::create_framebuffer(uint32_t /*width*/,
                                        uint32_t /*height*/,
                                        KxFormat /*format*/,
                                        BackendFramebuffer* /*out*/) {
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

void KmsBackend::destroy_framebuffer(BackendFramebuffer* /*fb*/) {}

KxResult KmsBackend::present(const BackendFramebuffer* /*fb*/) {
    return KX_ERROR_FEATURE_NOT_PRESENT;
}

KxResult KmsBackend::wait_vblank() { return KX_ERROR_FEATURE_NOT_PRESENT; }

void KmsBackend::shutdown() {}

} /* namespace kx */

#else

namespace kx {

KxResult KmsBackend::initialize(const void*) { return KX_ERROR_FEATURE_NOT_PRESENT; }
KxResult KmsBackend::create_framebuffer(uint32_t, uint32_t, KxFormat,
                                         BackendFramebuffer*) {
    return KX_ERROR_FEATURE_NOT_PRESENT;
}
void KmsBackend::destroy_framebuffer(BackendFramebuffer*) {}
KxResult KmsBackend::present(const BackendFramebuffer*) {
    return KX_ERROR_FEATURE_NOT_PRESENT;
}
KxResult KmsBackend::wait_vblank() { return KX_ERROR_FEATURE_NOT_PRESENT; }
void KmsBackend::shutdown() {}

} /* namespace kx */

#endif /* KX_HAVE_LIBDRM */
