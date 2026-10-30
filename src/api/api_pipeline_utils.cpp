/*
 * api_pipeline_utils.cpp
 *
 * Proc addresses and result string conversion C API implementation.
 */

#include "api.h"

#include "api_internal.h"

#include "Device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Proc addresses
 * ======================================================================= */

KxPfnVoidFunction KX_CALL kxGetDeviceProcAddr(
    KxDevice device,
    const char* pName) {
    (void)device; (void)pName;
    return nullptr;
}

KxPfnVoidFunction KX_CALL kxGetInstanceProcAddr(
    KxInstance instance,
    const char* pName) {
    (void)instance; (void)pName;
    return nullptr;
}

/* =======================================================================
 * Result string conversion
 * ======================================================================= */

const char* KX_CALL kxResultToString(KxResult result) {
    switch (result) {
    case KX_SUCCESS: return "KX_SUCCESS";
    case KX_NOT_READY: return "KX_NOT_READY";
    case KX_TIMEOUT: return "KX_TIMEOUT";
    case KX_EVENT_SET: return "KX_EVENT_SET";
    case KX_EVENT_RESET: return "KX_EVENT_RESET";
    case KX_INCOMPLETE: return "KX_INCOMPLETE";
    case KX_ERROR_OUT_OF_HOST_MEMORY: return "KX_ERROR_OUT_OF_HOST_MEMORY";
    case KX_ERROR_OUT_OF_DEVICE_MEMORY: return "KX_ERROR_OUT_OF_DEVICE_MEMORY";
    case KX_ERROR_INITIALIZATION_FAILED: return "KX_ERROR_INITIALIZATION_FAILED";
    case KX_ERROR_DEVICE_LOST: return "KX_ERROR_DEVICE_LOST";
    case KX_ERROR_MEMORY_MAP_FAILED: return "KX_ERROR_MEMORY_MAP_FAILED";
    case KX_ERROR_LAYER_NOT_PRESENT: return "KX_ERROR_LAYER_NOT_PRESENT";
    case KX_ERROR_EXTENSION_NOT_PRESENT: return "KX_ERROR_EXTENSION_NOT_PRESENT";
    case KX_ERROR_FEATURE_NOT_PRESENT: return "KX_ERROR_FEATURE_NOT_PRESENT";
    case KX_ERROR_INCOMPATIBLE_DRIVER: return "KX_ERROR_INCOMPATIBLE_DRIVER";
    case KX_ERROR_TOO_MANY_OBJECTS: return "KX_ERROR_TOO_MANY_OBJECTS";
    case KX_ERROR_FORMAT_NOT_SUPPORTED: return "KX_ERROR_FORMAT_NOT_SUPPORTED";
    case KX_ERROR_FRAGMENTED_POOL: return "KX_ERROR_FRAGMENTED_POOL";
    case KX_ERROR_OUT_OF_POOL_MEMORY: return "KX_ERROR_OUT_OF_POOL_MEMORY";
    case KX_ERROR_INVALID_EXTERNAL_HANDLE: return "KX_ERROR_INVALID_EXTERNAL_HANDLE";
    case KX_ERROR_INVALID_ARGUMENT: return "KX_ERROR_INVALID_ARGUMENT";
    case KX_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "KX_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case KX_ERROR_VALIDATION_FAILED_EXT: return "KX_ERROR_VALIDATION_FAILED_EXT";
    default: return "KX_UNKNOWN_RESULT";
    }
}

#ifdef __cplusplus
} /* extern "C" */
#endif