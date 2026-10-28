# Kromax ADR Index

## 0001-c-abi-cpp-impl.md
- **Title**: C ABI / C++ Implementation Boundary
- **Status**: Accepted
- **Context**: Need public API in C11 for ABI stability, internal implementation in C++20 for modern features
- **Decision**: 
  - Public API headers (`include/kromax/`) compile as both C11 and C++20
  - Internal source files (`src/`) compile as C++20 only
  - `KxAPI` / `KX_CALL` macros for symbol export and calling convention
  - No C++ features (exceptions, RTTI) crossing the C API boundary
  - Errors returned as `KxResult` at the boundary, `kx::Expected<T>` internally
- **Consequences**: 
  - Clean C ABI for external consumers
  - Internal flexibility of C++20 features (templates, STL, etc.)
  - Additional boilerplate for handle translation and error conversion

## 0002-no-vulkan-no-mesa.md
- **Title**: No Vulkan, No Mesa Dependencies
- **Status**: Accepted
- **Context**: Project goal to create graphics API without GPU driver dependencies
- **Decision**: 
  - Zero runtime dependencies besides libc/libstdc++
  - `libdrm` only for KMS display backend, behind `IDisplayBackend` interface
  - Future replacement with direct ioctls possible
  - Software driver as reference backend
- **Consequences**: 
  - Portable to systems without GPU drivers
  - Easier CI setup (no Vulkan/Mesa packages needed)
  - Limited to software rendering unless external drivers added later

## 0003-handle-model.md
- **Title**: Opaque Handle Model
- **Status**: Accepted
- **Context**: Vulkan-style handle design for extensibility and safety
- **Decision**: 
  - `KX_DEFINE_HANDLE(KxInstance)` expands to `typedef struct KxInstance_T* KxInstance`
  - All objects are opaque handles, no exposed internal structure types
  - Handle validity checked internally, not by users
  - `kxDestroy*` functions take handles and free resources
- **Consequences**: 
  - Type safety, handles can be checked for validity
  - Implementation can change internal object layout without affecting ABI
  - Users must not free handles, only through Kromax API