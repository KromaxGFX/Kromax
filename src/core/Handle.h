#ifndef KROMAX_SRC_CORE_HANDLE_H
#define KROMAX_SRC_CORE_HANDLE_H

/*
 * Handle.h
 *
 * Utilities for mapping opaque C handles to internal C++ objects and back.
 */

#include "kx_core.h"

namespace kx {

/*
 * HandleTraits maps a public handle type to its internal implementation
 * class. Specialize for each object type (e.g. HandleTraits<KxInstance>).
 */
template <typename HandleType>
struct HandleTraits;

/*
 * Convert an opaque handle to the internal implementation pointer.
 * Returns nullptr for a null handle.
 */
template <typename HandleType, typename ImplType>
inline ImplType* handle_to_impl(HandleType handle) {
    return static_cast<ImplType*>(handle);
}

/*
 * Convert an internal implementation pointer back to an opaque handle.
 */
template <typename HandleType, typename ImplType>
inline HandleType impl_to_handle(ImplType* impl) {
    return static_cast<HandleType>(impl);
}

} /* namespace kx */

#endif /* KROMAX_SRC_CORE_HANDLE_H */