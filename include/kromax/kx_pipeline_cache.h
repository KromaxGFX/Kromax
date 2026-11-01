#ifndef KX_PIPELINE_CACHE_H
#define KX_PIPELINE_CACHE_H

#include "kx_types.h"
#include "kx_enums.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =======================================================================
 * Pipeline Cache Extension (KX_EXT_pipeline_cache)
 * ======================================================================= */

#define KX_EXT_PIPELINE_CACHE_EXTENSION_NAME "KX_EXT_pipeline_cache"
#define KX_EXT_PIPELINE_CACHE_SPEC_VERSION 1

typedef struct KxPipelineCache_T* KxPipelineCache;

/* Pipeline cache create flags */
typedef uint32_t KxPipelineCacheCreateFlags;
#define KX_PIPELINE_CACHE_CREATE_NONE 0u
#define KX_PIPELINE_CACHE_CREATE_MAX_ENUM 0x7FFFFFFFu

/* Pipeline cache header (v1) */
#define KX_PIPELINE_CACHE_HEADER_SIZE 32

typedef struct KxPipelineCacheCreateInfo {
    KxStructureType sType;
    const void* pNext;
    KxPipelineCacheCreateFlags flags;
    size_t initialDataSize;
    const void* pInitialData;
} KxPipelineCacheCreateInfo;

/* Pipeline cache header for serialization */
typedef struct KxPipelineCacheHeader {
    char magic[4];              /* "KXPC" */
    uint32_t version;           /* 1 */
    uint32_t vendorID;          /* Vendor ID */
    uint32_t deviceID;          /* Device ID */
    uint64_t timestamp;         /* Unix milliseconds */
    uint32_t entryCount;        /* Number of entries */
    uint32_t headerSize;        /* Size of this header (32) */
    uint32_t reserved;
} KxPipelineCacheHeader;

/* Pipeline cache entry (for internal use) */
typedef struct KxPipelineCacheEntry {
    uint8_t layoutHash[32];     /* SHA-256 of pipeline layout */
    uint8_t shaderHashes[3][32]; /* SHA-256 of vert/frag/comp IR */
    uint8_t createInfoHash[32]; /* SHA-256 of full create info */
    uint64_t timestamp;         /* Creation timestamp */
    size_t irSize;              /* Serialized IR size */
    size_t reflectionSize;      /* Reflection metadata size */
    size_t bytecodeSize;        /* Backend bytecode size (SPIR-V, etc.) */
} KxPipelineCacheEntry;

/* =======================================================================
 * Pipeline Cache functions
 * ======================================================================= */

/* Create a pipeline cache */
KX_API KxResult KX_CALL kxCreatePipelineCache(
    KxDevice device,
    const KxPipelineCacheCreateInfo* pCreateInfo,
    const KxAllocationCallbacks* pAllocator,
    KxPipelineCache* pPipelineCache);

/* Destroy a pipeline cache */
KX_API void KX_CALL kxDestroyPipelineCache(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const KxAllocationCallbacks* pAllocator);

/* Save pipeline cache to file */
KX_API KxResult KX_CALL kxSavePipelineCache(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const char* filename);

/* Load pipeline cache from file */
KX_API KxResult KX_CALL kxLoadPipelineCache(
    KxDevice device,
    KxPipelineCache pipelineCache,
    const char* filename);

/* Get pipeline cache data for embedding */
KX_API KxResult KX_CALL kxGetPipelineCacheData(
    KxDevice device,
    KxPipelineCache pipelineCache,
    size_t* pDataSize,
    void* pData);

/* Merge pipeline caches */
KX_API KxResult KX_CALL kxMergePipelineCaches(
    KxDevice device,
    KxPipelineCache dstCache,
    uint32_t srcCacheCount,
    const KxPipelineCache* pSrcCaches);

/* Get pipeline cache header info */
KX_API KxResult KX_CALL kxGetPipelineCacheInfo(
    KxPipelineCache pipelineCache,
    KxPipelineCacheHeader* pHeader);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* KX_PIPELINE_CACHE_H */