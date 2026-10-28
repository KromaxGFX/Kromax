#ifndef KX_DEBUG_H
#define KX_DEBUG_H

/*
 * kx_debug.h
 *
 * KX_DEBUG_EXT debug extension: KxDebugMessengerEXT and callback-based
 * debug logging. Declared here; implemented in src/layers/ (validation)
 * and src/api/.
 */

#include "kx_core.h"

#ifdef __cplusplus
extern "C" {
#endif

KX_DEFINE_HANDLE(KxDebugMessengerEXT)

typedef enum KxDebugMessageSeverityEXT {
    KX_DEBUG_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001u,
    KX_DEBUG_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010u,
    KX_DEBUG_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100u,
    KX_DEBUG_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000u,
    KX_DEBUG_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} KxDebugMessageSeverityEXT;
typedef uint32_t KxDebugMessageSeverityFlagsEXT;

typedef enum KxDebugMessageTypeEXT {
    KX_DEBUG_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001u,
    KX_DEBUG_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000010u,
    KX_DEBUG_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000100u,
    KX_DEBUG_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} KxDebugMessageTypeEXT;
typedef uint32_t KxDebugMessageTypeFlagsEXT;

typedef KxBool32 (KX_CALL *KxPfnDebugMessengerCallbackEXT)(
    KxDebugMessageSeverityFlagsEXT messageSeverity,
    KxDebugMessageTypeFlagsEXT messageTypes,
    const char* pCallbackData,
    void* pUserData);

typedef struct KxDebugMessengerCreateInfoEXT {
    KxStructureType sType;
    const void* pNext;
    KxDebugMessageSeverityFlagsEXT messageSeverity;
    KxDebugMessageTypeFlagsEXT messageTypes;
    KxPfnDebugMessengerCallbackEXT pfnUserCallback;
    void* pUserData;
} KxDebugMessengerCreateInfoEXT;

KX_API KxResult KX_CALL kxCreateDebugMessengerEXT(
    KxInstance instance,
    const KxDebugMessengerCreateInfoEXT* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxDebugMessengerEXT* pMessenger);

KX_API void KX_CALL kxDestroyDebugMessengerEXT(
    KxInstance instance,
    KxDebugMessengerEXT messenger,
    const KxAllocationCallbacks* pAllocator);

/* Internal helper used by the implementation to dispatch debug messages */
KX_API void KX_CALL kxDebugMessageEXT(
    KxInstance instance,
    KxDebugMessageSeverityFlagsEXT severity,
    KxDebugMessageTypeFlagsEXT type,
    const char* pMessage);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_DEBUG_H */