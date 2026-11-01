#ifndef KX_VALIDATION_H
#define KX_VALIDATION_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Validation Layer Extension (KX_EXT_validation_layer)
 * ======================================================================= */

#define KX_EXT_VALIDATION_LAYER_EXTENSION_NAME "KX_EXT_validation_layer"
#define KX_EXT_VALIDATION_LAYER_SPEC_VERSION 1

/* Validation checks bitmask */
typedef uint32_t KxValidationChecks;
#define KX_VALIDATION_CHECK_NONE                    0u
#define KX_VALIDATION_CHECK_DESCRIPTOR_BOUNDS       0x00000001u
#define KX_VALIDATION_CHECK_PIPELINE_COMPATIBILITY  0x00000002u
#define KX_VALIDATION_CHECK_MEMORY_HAZARDS          0x00000004u
#define KX_VALIDATION_CHECK_RENDER_PASS_COMPAT      0x00000008u
#define KX_VALIDATION_CHECK_SHADER_RESOURCE_BINDING 0x00000010u
#define KX_VALIDATION_CHECK_COMMAND_BUFFER_STATE    0x00000020u
#define KX_VALIDATION_CHECK_IMAGE_LAYOUTS           0x00000040u
#define KX_VALIDATION_CHECK_SYNCHRONIZATION         0x00000080u
#define KX_VALIDATION_CHECK_ALL                     0xFFFFFFFFu

/* Validation severity */
typedef enum KxValidationSeverity {
    KX_VALIDATION_SEVERITY_INFO = 0,
    KX_VALIDATION_SEVERITY_WARNING = 1,
    KX_VALIDATION_SEVERITY_ERROR = 2,
    KX_VALIDATION_SEVERITY_MAX_ENUM = 0x7FFFFFFF
} KxValidationSeverity;

/* Validation callback */
typedef void (KX_CALL* KxValidationCallback)(
    KxValidationSeverity severity,
    const char* message,
    void* userData);

/* Set validation callback */
KX_API void KX_CALL kxSetValidationCallback(
    KxValidationCallback callback,
    void* userData);

/* Enable/disable validation checks */
KX_API void KX_CALL kxSetValidationChecks(
    KxValidationChecks checks);

/* Get current validation checks */
KX_API KxValidationChecks KX_CALL kxGetValidationChecks(void);

/* =======================================================================
 * Debug Markers (for PIX/RenderDoc/NSight)
 * ======================================================================= */

/* Debug marker callbacks */
typedef void (KX_CALL* KxDebugMarkerCallback)(
    KxCommandBuffer commandBuffer,
    const char* name,
    const float color[4],
    void* userData);

/* Set debug marker callbacks */
KX_API void KX_CALL kxSetDebugMarkerCallbacks(
    KxDebugMarkerCallback begin,
    KxDebugMarkerCallback end,
    KxDebugMarkerCallback insert,
    void* userData);

/* Debug marker functions */
KX_API void KX_CALL kxCmdDebugMarkerBegin(
    KxCommandBuffer commandBuffer,
    const char* name,
    const float color[4]);

KX_API void KX_CALL kxCmdDebugMarkerEnd(
    KxCommandBuffer commandBuffer);

KX_API void KX_CALL kxCmdDebugMarkerInsert(
    KxCommandBuffer commandBuffer,
    const char* name,
    const float color[4]);

/* Object naming for debug tools */
KX_API KxResult KX_CALL kxSetObjectName(
    KxDevice device,
    KxObjectType objectType,
    uint64_t objectHandle,
    const char* name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_VALIDATION_H */