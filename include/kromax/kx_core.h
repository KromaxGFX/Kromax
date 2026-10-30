#ifndef KX_CORE_H
#define KX_CORE_H

/*
 * Kromax Core API - Main Entry Point
 *
 * This header includes all modular Kromax headers.
 * For fine-grained control, include individual headers:
 *   kx_types.h      - Base types, handles, result codes, structure types
 *   kx_enums.h      - All enums (format, image, pipeline, descriptor, sampler, etc.)
 *   kx_structs.h    - Create info structures for memory, buffer, image, renderpass, etc.
 *   kx_pipeline.h   - Pipeline, shader, pipeline layout, vertex input, viewport, etc.
 *   kx_descriptor.h - Descriptor set layout, pool, sets, samplers
 *   kx_api.h        - All function declarations
 */

#include "kx_types.h"
#include "kx_enums.h"
#include "kx_structs.h"
#include "kx_pipeline.h"
#include "kx_descriptor.h"
#include "kx_api.h"

#endif /* KX_CORE_H */