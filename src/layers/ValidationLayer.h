#ifndef KROMAX_SRC_LAYERS_VALIDATIONLAYER_H
#define KROMAX_SRC_LAYERS_VALIDATIONLAYER_H

/*
 * ValidationLayer.h
 *
 * First validation layer: a decorator over the device dispatch table that
 * verifies correct usage (valid handles, command buffer states, etc.).
 * Disabled by default; enabled via instance extension.
 */

#include "Dispatch.h"

namespace kx {

class ValidationLayer {
public:
    ValidationLayer() = default;
    ~ValidationLayer() = default;

    /* Install validation wrappers over the device dispatch table. */
    void install(DeviceDispatch* dispatch) noexcept;

private:
};

} /* namespace kx */

#endif /* KROMAX_SRC_LAYERS_VALIDATIONLAYER_H */