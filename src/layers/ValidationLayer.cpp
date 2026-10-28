#include "ValidationLayer.h"
#include "Logging.h"

namespace kx {

void ValidationLayer::install(DeviceDispatch* /*dispatch*/) noexcept {
    log_info("Validation layer installed (stub)");
}

} /* namespace kx */