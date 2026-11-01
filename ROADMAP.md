# KromaxGFX — Technical Roadmap v2.0

**Status**: Post-Sprint 8 | **Target**: CPU-first (Windows/Linux) with GPU-ready architecture  
**Scope**: Full feature parity with Vulkan 1.3 + extensions, software reference implementation, backend-agnostic design

---

## 1. Architectural Principles

| Principle | Application |
|-----------|-------------|
| **Dependency Inversion** | `IRuntime`/`IDevice` interfaces in `runtime/`; backends implement, frontend depends on abstractions |
| **Interface Segregation** | Fine-grained interfaces: `IShaderModule`, `IPipeline`, `IDescriptorSet`, `ICommandBuffer` |
| **Single Responsibility** | Each backend module owns one Vulkan domain (shader, pipeline, memory, sync, swapchain) |
| **Open/Closed** | New backends (Metal, DX12) add implementations without modifying frontend |
| **Liskov Substitution** | `CpuDevice`/`VkDevice` interchangeable via `IDevice`; behaviorally identical |
| **Clean Code** | No magic numbers → enums/bitflags; no raw pointers → RAII handles; self-documenting names |

**Coding Standards**:
- C11 public API (stable ABI, no C++ in headers)
- C++17 internal (no exceptions/RTTI at boundary, RAII everywhere)
- `clang-format` + `clang-tidy` enforced in CI
- Doxygen for all public symbols; internal docs in `docs/arch/`

---

## 2. Repository Structure (Target)

```
kromaxgfx/
├── include/kromax/           # Public C11 API (stable)
│   ├── kx_api.h              # Core: instance, device, queue, memory, commands
│   ├── kx_shader.h           # Shader compilation, reflection, SPIR-V export
│   ├── kx_material.h         # Material system, PBR, descriptor publishing
│   ├── kx_swapchain.h        # KX_KHR_swapchain extension
│   ├── kx_timeline.h         # KX_KHR_timeline_semaphore
│   ├── kx_descriptor_indexing.h  # KX_EXT_descriptor_indexing
│   └── kx_raytracing.h       # KX_KHR_ray_tracing
├── src/
│   ├── runtime/              # Backend-agnostic interfaces (SOLID)
│   │   ├── IRuntime.h        # Factory: createInstance, enumerateDevices, loadBackend
│   │   ├── IDevice.h         # Resource factory: buffers, images, pipelines, descriptors
│   │   ├── ICommandBuffer.h  # Recording: draw, dispatch, barriers, render passes
│   │   ├── IShaderModule.h   # IR + reflection + optional SPIR-V export
│   │   ├── IPipeline.h       # Pipeline state object
│   │   ├── IDescriptorSet.h  # Descriptor set operations
│   │   ├── ISwapchain.h      # Surface, acquire, present
│   │   ├── ISemaphore.h      # Binary + timeline
│   │   ├── IBackend.h        # Plugin interface: init/shutdown, version negotiation
│   │   └── BackendRegistry.cpp  # Dynamic loading via KX_BACKEND env
│   ├── cpu_backend/          # Reference implementation (current software rasterizer)
│   │   ├── CpuRuntime.cpp
│   │   ├── CpuDevice.cpp
│   │   ├── CpuCommandBuffer.cpp
│   │   ├── CpuShaderModule.cpp
│   │   ├── CpuPipeline.cpp
│   │   ├── CpuDescriptorSet.cpp
│   │   ├── CpuSwapchain.cpp
│   │   ├── CpuSemaphore.cpp
│   │   ├── SoftwareRasterizer.cpp
│   │   └── SoftwareShaderInterpreter.cpp
│   ├── vk_backend/           # Vulkan backend (Phase K)
│   │   ├── VkRuntime.cpp
│   │   ├── VkDevice.cpp
│   │   ├── VkCommandBuffer.cpp
│   │   ├── VkShaderModule.cpp    # IR → SPIR-V (spirv-tools)
│   │   ├── VkPipeline.cpp
│   │   ├── VkDescriptorSet.cpp
│   │   ├── VkSwapchain.cpp
│   │   ├── VkMemory.cpp
│   │   ├── VkSemaphore.cpp
│   │   └── VkSync.cpp
│   ├── shader/               # Frontend compiler (shared, backend-agnostic)
│   │   ├── lexer/            # Tokenizer, preprocessor (#include, #define, #ifdef)
│   │   ├── parser/           # Recursive descent, dialect-aware (GLSL/HLSL/WGSL/KXSL)
│   │   ├── ir/               # SSA IR: instructions, types, dominance, CFG
│   │   ├── compiler/         # IR builder, optimization passes, validation
│   │   ├── reflection/       # Entry points, resources, push constants, attributes
│   │   └── diagnostics/      # Errors/warnings with source locations
│   ├── material/             # PBR material system (shared)
│   │   ├── Material.cpp
│   │   ├── MaterialDescriptorPublisher.cpp
│   │   └── PbrShader.glsl    # Built-in shader source (compiled at runtime)
│   ├── platform/             # OS abstraction
│   │   ├── Platform.h        # Interface: library loading, surface creation
│   │   ├── PlatformWin32.cpp
│   │   └── PlatformLinux.cpp
│   └── core/                 # API entry points (thin wrappers → runtime)
├── tests/
│   ├── unit/                 # 20+ test suites (doctest)
│   ├── integration/          # End-to-end: swapchain, compute, raytracing, etc.
│   └── benchmarks/           # Performance regression suite
├── examples/                 # 10+ examples demonstrating features
├── docs/
│   ├── arch/                 # Architecture decision records (ADRs)
│   ├── api/                  # Generated API reference
│   └── guides/               # Migration, best practices
├── cmake/                    # Toolchains, Find modules, FetchContent deps
├── .github/workflows/        # CI: Linux (Ubuntu) + Windows (MSYS2) + macOS (ARM64)
└── ROADMAP.md                # This file
```

---

## 3. Phase Definitions

### Phase A — Linux Support & Continuous Integration
**Duration**: 1 week | **Priority**: P0 | **Dependencies**: None

**Deliverables**:
- Native Linux build (GCC 13+, Clang 17+) via CMake preset `linux-debug`/`linux-release`
- Platform abstraction: `PlatformLinux.cpp` (dlopen Vulkan, xcb/wayland surface stubs)
- GitHub Actions workflow: dual OS matrix (ubuntu-latest, windows-latest, macos-latest)
- Dockerfile for reproducible builds
- `ctest` passing on all three platforms

**Technical Details**:
```cmake
# cmake/presets/linux.cmake
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(Vulkan QUIET)  # Optional for CPU backend
find_package(X11 QUIET)
find_package(Wayland QUIET)
```

**Acceptance Criteria**:
- Zero warnings on `-Wall -Wextra -Wpedantic -Werror`
- Build time < 300s on GitHub Actions runners
- All 20 existing tests pass on Ubuntu 24.04 LTS

---

### Phase B — Backend-Agnostic Architecture
**Duration**: 2 weeks | **Priority**: P0 | **Dependencies**: Phase A

**Deliverables**:
- `runtime/` interfaces (pure virtual, no implementation details)
- `BackendRegistry`: dynamic loading via `KX_BACKEND=cpu|vulkan|metal|dx12`
- Migration of existing `driver/software/` → `cpu_backend/`
- SPIR-V export-only API: `kxCompileShaderToSPIRV()`

**Interface Contracts** (excerpt):
```cpp
// IRuntime.h
class IRuntime {
public:
    virtual ~IRuntime() = default;
    virtual Result createInstance(const InstanceCreateInfo&, Instance*) = 0;
    virtual Result enumerateDevices(Instance, uint32_t*, PhysicalDevice*) = 0;
    virtual Result createDevice(PhysicalDevice, const DeviceCreateInfo*, Device*) = 0;
    virtual Result compileShader(const ShaderSourceCreateInfo*, ShaderModule*) = 0;
    virtual Result compileShaderToSPIRV(const ShaderSourceCreateInfo*, std::vector<uint32_t>*) = 0;
    virtual const char* getBackendName() const = 0;
    virtual BackendType getBackendType() const = 0;
};

// IDevice.h
class IDevice {
public:
    virtual ~IDevice() = default;
    virtual Result createBuffer(const BufferCreateInfo*, Buffer*) = 0;
    virtual Result createImage(const ImageCreateInfo*, Image*) = 0;
    virtual Result createShaderModule(const ShaderModuleCreateInfo*, ShaderModule*) = 0;
    virtual Result createPipeline(const PipelineCreateInfo*, Pipeline*) = 0;
    virtual Result createDescriptorSetLayout(const DescriptorSetLayoutCreateInfo*, DescriptorSetLayout*) = 0;
    virtual Result allocateDescriptorSets(const DescriptorSetAllocateInfo*, DescriptorSet*) = 0;
    virtual Result createCommandPool(const CommandPoolCreateInfo*, CommandPool*) = 0;
    virtual Result createSwapchain(const SwapchainCreateInfo*, Swapchain*) = 0;
    virtual Result createSemaphore(const SemaphoreCreateInfo*, Semaphore*) = 0;
    // ... all resource creation
};
```

**Backend Registration**:
```cpp
// BackendRegistry.cpp
using BackendFactoryFn = std::unique_ptr<IRuntime>(*)();

struct BackendEntry {
    BackendType type;
    const char* libraryName;  // "cpu_backend", "vk_backend"
    BackendFactoryFn factory;
};

static std::vector<BackendEntry> g_backends = {
    {BackendType::CPU, "cpu_backend", &createCpuRuntime},
    {BackendType::VULKAN, "vk_backend", &createVkRuntime},
    // Metal, DX12 added later
};
```

**Acceptance Criteria**:
- `KX_BACKEND=cpu` → all existing tests pass
- `KX_BACKEND=vulkan` loads plugin (stub returns `ERROR_FEATURE_NOT_PRESENT`)
- Zero coupling between `include/kromax/` and backend implementations

---

### Phase C — Complete Shader Stages
**Duration**: 3 weeks | **Priority**: P0 | **Dependencies**: Phase B

**Scope**: Implement all Vulkan 1.3 shader stages in frontend + CPU backend.

| Stage | Frontend | CPU Backend | New Builtins |
|-------|----------|-------------|--------------|
| Vertex | ✅ | ✅ | — |
| Fragment | ✅ | ✅ | — |
| Compute | ✅ | ✅ | — |
| **Geometry** | Parser + IR | Emit/EndPrimitive per vertex | `EmitVertex`, `EndPrimitive`, `EmitStreamVertex`, `EndStreamPrimitive` |
| **Tessellation** (Hull/Domain) | Parser + IR | Bézier/BSpline eval, tessellation factor clamping | `gl_TessLevelOuter[4]`, `gl_TessLevelInner[2]`, `gl_TessCoord`, `gl_PatchVerticesIn` |
| **Mesh/Task** | Parser + IR | Task→payload→Mesh workgroups, `SetMeshOutputsEXT` | `gl_MeshVerticesEXT`, `gl_PrimitiveIndicesEXT`, `gl_PrimitiveCountEXT` |
| **Ray Tracing** | Parser + IR | BVH (SAH), packet traversal, ray shading | `traceRayEXT`, `reportIntersectionEXT`, `payloadEXT`, `hitAttributeEXT`, `rayTminEXT`, `rayTmaxEXT`, `worldRayOriginEXT`, `worldRayDirectionEXT`, `instanceCustomIndexEXT`, `geometryIndexEXT`, `primitiveIdEXT`, `instanceIdEXT` |

**Subgroup Operations** (all stages):
`subgroupAdd`, `subgroupMul`, `subgroupMin`, `subgroupMax`, `subgroupAnd/Or/Xor`, `subgroupShuffle*`, `subgroupQuadSwap*`, `subgroupBroadcast*`, `subgroupBallot*`, `subgroupElect`, `subgroupBarrier`, `subgroupMemoryBarrier`

**IR Extensions**:
```cpp
// ir/Instruction.h
enum class OpCode : uint16_t {
    // ... existing
    EmitVertex,
    EndPrimitive,
    EmitStreamVertex,
    EndStreamPrimitive,
    SetMeshOutputsEXT,
    TraceRayEXT,
    ReportIntersectionEXT,
    IgnoreIntersectionEXT,
    TerminateRayEXT,
    ExecuteCallableEXT,
    // Subgroup
    SubgroupAdd, SubgroupMul, SubgroupMin, SubgroupMax,
    SubgroupAnd, SubgroupOr, SubgroupXor,
    SubgroupShuffle, SubgroupShuffleXor, SubgroupQuadSwapHorizontal,
    SubgroupQuadSwapVertical, SubgroupQuadSwapDiagonal,
    SubgroupBroadcast, SubgroupBroadcastFirst,
    SubgroupBallot, SubgroupInverseBallot,
    SubgroupBallotBitExtract, SubgroupBallotBitCount,
    SubgroupBallotInclusiveBitCount, SubgroupBallotExclusiveBitCount,
    SubgroupElect, SubgroupBarrier, SubgroupMemoryBarrier,
};
```

**Test Suite Additions**:
- `test_shader_geometry.cpp` — point → triangle via GS
- `test_shader_tessellation.cpp` — quad patch 4×4 subdivision
- `test_shader_mesh_task.cpp` — task generates 64 mesh workgroups
- `test_shader_raytracing.cpp` — Cornell box (BVH + diffuse/specular)

**Acceptance Criteria**:
- 100% new tests pass on CPU backend
- Coverage > 90% on new builtins
- IR round-trip (serialize/deserialize) preserves semantics

---

### Phase D — Swapchain & Present
**Duration**: 1 week | **Priority**: P1 | **Dependencies**: Phase B

**API Surface** (`include/kromax/kx_swapchain.h`):
```c
typedef enum KxPresentModeKHR {
    KX_PRESENT_MODE_IMMEDIATE_KHR = 0,
    KX_PRESENT_MODE_MAILBOX_KHR = 1,
    KX_PRESENT_MODE_FIFO_KHR = 2,
    KX_PRESENT_MODE_FIFO_RELAXED_KHR = 3,
} KxPresentModeKHR;

typedef struct KxSwapchainCreateInfoKHR {
    KxStructureType sType;
    const void* pNext;
    KxSwapchainCreateFlagsKHR flags;
    KxSurfaceKHR surface;
    uint32_t minImageCount;
    KxFormat imageFormat;
    KxColorSpaceKHR imageColorSpace;
    KxExtent2D imageExtent;
    uint32_t imageArrayLayers;
    KxImageUsageFlags imageUsage;
    KxSharingMode imageSharingMode;
    uint32_t queueFamilyIndexCount;
    const uint32_t* pQueueFamilyIndices;
    KxSurfaceTransformFlagBitsKHR preTransform;
    KxCompositeAlphaFlagBitsKHR compositeAlpha;
    KxPresentModeKHR presentMode;
    KxBool32 clipped;
    KxSwapchainKHR oldSwapchain;
} KxSwapchainCreateInfoKHR;

KX_API KxResult KX_CALL kxCreateSwapchainKHR(KxDevice, const KxSwapchainCreateInfoKHR*, const KxAllocationCallbacks*, KxSwapchainKHR*);
KX_API void KX_CALL kxDestroySwapchainKHR(KxDevice, KxSwapchainKHR, const KxAllocationCallbacks*);
KX_API KxResult KX_CALL kxGetSwapchainImagesKHR(KxDevice, KxSwapchainKHR, uint32_t*, KxImage*);
KX_API KxResult KX_CALL kxAcquireNextImageKHR(KxDevice, KxSwapchainKHR, uint64_t, KxSemaphore, KxFence, uint32_t*);
KX_API KxResult KX_CALL kxQueuePresentKHR(KxQueue, const KxPresentInfoKHR*);
```

**CPU Present Implementation**:
- Windows: `StretchDIBits`/`BitBlt` to `HDC` + `InvalidateRect`
- Linux/X11: `XShmPutImage` to `Window`
- Linux/Wayland: `wl_surface_attach` + `wl_surface_damage_buffer` + `wl_surface_commit`

**Example**: `07_swapchain` — 800×600 window, rotating triangle, 60 FPS, triple-buffered.

**Acceptance Criteria**:
- 60 FPS sustained on Windows 10/11 + Ubuntu 24.04
- Frame latency (acquire → present) < 16ms
- No tearing in FIFO mode; minimal latency in MAILBOX

---

### Phase E — Multi-Threaded Command Recording
**Duration**: 2 weeks | **Priority**: P1 | **Dependencies**: Phase B

**Design**:
- `KxCommandBuffer` uses thread-local storage for recording state
- `CommandPool` internally mutex-protected for allocation
- `Queue::submit()` accepts `std::vector<CommandBuffer*>` from arbitrary threads
- Render pass dependency graph for parallel subpass execution

**Thread Pool for Rasterizer**:
```cpp
class TileWorkStealingPool {
    struct Tile { uint32_t x, y, w, h; };
    std::atomic<uint32_t> nextTile_;
    std::vector<std::thread> workers_;
    std::vector<Tile> tiles_;
    std::mutex mergeMutex_;
    Framebuffer* target_;
    
    void workerLoop() {
        while (uint32_t idx = nextTile_.fetch_add(1)) {
            if (idx >= tiles_.size()) return;
            rasterizeTile(tiles_[idx]);
            std::lock_guard lock(mergeMutex_);
            mergeTile(tiles_[idx]);
        }
    }
};
```

**API**:
```c
typedef struct KxThreadPoolCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint32_t threadCount;  // 0 = hardware_concurrency
} KxThreadPoolCreateInfo;

KX_API KxResult KX_CALL kxCreateThreadPool(KxDevice, const KxThreadPoolCreateInfo*, KxThreadPool*);
KX_API void KX_CALL kxDestroyThreadPool(KxDevice, KxThreadPool, const KxAllocationCallbacks*);
KX_API void KX_CALL kxCmdSetThreadPool(KxCommandBuffer, KxThreadPool);
```

**Example**: `08_multithread` — 4 threads recording 4 command buffers, single submit.

**Acceptance Criteria**:
- Speedup ≥ 3.5× on 4 cores vs single-threaded
- ThreadSanitizer clean
- No deadlocks under stress (10k submits)

---

### Phase F — Persistent Pipeline Cache
**Duration**: 1 week | **Priority**: P2 | **Dependencies**: Phase B

**Cache Format** (`kxcache` v1):
```cpp
struct CacheHeader {
    char magic[4];           // "KXCH"
    uint32_t version;        // 1
    uint32_t entryCount;
    uint64_t timestamp;      // Unix ms
    uint32_t backendType;    // BackendType enum
};

struct CacheEntry {
    SHA256Hash layoutHash;           // Pipeline layout
    SHA256Hash shaderHashes[3];      // Vert/Frag/Comp IR hashes
    SHA256Hash createInfoHash;       // Full PipelineCreateInfo
    std::vector<uint8_t> irBlob;     // Serialized IR
    std::vector<uint8_t> reflection; // Reflection metadata
    std::vector<uint8_t> backendBytecode; // SPIR-V for Vulkan (optional)
};
```

**API**:
```c
KX_API KxResult KX_CALL kxSavePipelineCache(KxDevice, const char* filename);
KX_API KxResult KX_CALL kxLoadPipelineCache(KxDevice, const char* filename);
KX_API KxResult KX_CALL kxGetPipelineCacheData(KxDevice, size_t*, void*);  // For embedding
```

**Invalidation Logic**:
```cpp
bool isCacheValid(const CacheEntry& entry, const PipelineCreateInfo& info) {
    return entry.layoutHash == hash(info.layout) &&
           entry.shaderHashes[0] == hash(info.vertIR) &&
           entry.shaderHashes[1] == hash(info.fragIR) &&
           entry.createInfoHash == hash(info);
}
```

**Acceptance Criteria**:
- Cache hit → 0 recompilation, load < 50ms for 100 pipelines
- Cache miss → falls back to compile, writes new entry
- Cross-backend compatibility: CPU cache usable by Vulkan (IR only)

---

### Phase G — Validation & Debug Layers
**Duration**: 1 week | **Priority**: P1 | **Dependencies**: Phase B

**Validation Layer** (`KX_VALIDATION_LAYER=1`):
```cpp
enum class ValidationCheck : uint32_t {
    DESCRIPTOR_BOUNDS            = 1u << 0,
    PIPELINE_COMPATIBILITY       = 1u << 1,
    MEMORY_HAZARDS               = 1u << 2,
    RENDER_PASS_COMPATIBILITY    = 1u << 3,
    SHADER_RESOURCE_BINDING      = 1u << 4,
    COMMAND_BUFFER_STATE         = 1u << 5,
    ALL                          = 0xFFFFFFFF
};

using ValidationCallback = void(*)(ValidationSeverity, const char* message, void* userData);
KX_API void KX_CALL kxSetValidationCallback(ValidationCallback, void* userData);
KX_API void KX_CALL kxSetValidationChecks(ValidationChecks checks);
```

**Shader Debug Info**:
```c
typedef struct KxShaderDebugInfo {
    uint32_t lineMappingCount;
    const KxLineMapping* pLineMappings;      // IR instruction offset → source line
    uint32_t variableCount;
    const KxVariableInfo* pVariables;        // Name, type, location, size, component count
} KxShaderDebugInfo;

KX_API KxResult KX_CALL kxGetShaderDebugInfo(KxDevice, KxShaderModule, KxShaderDebugInfo*);
```

**Debug Markers** (PIX/RenderDoc/NSight integration):
```c
KX_API void KX_CALL kxCmdDebugMarkerBegin(KxCommandBuffer, const char* name, const float color[4]);
KX_API void KX_CALL kxCmdDebugMarkerEnd(KxCommandBuffer);
KX_API void KX_CALL kxCmdDebugMarkerInsert(KxCommandBuffer, const char* name, const float color[4]);
```

**Acceptance Criteria**:
- Catches 100% of VUID-critical errors in test suite
- Debug markers visible in RenderDoc/PIX
- Shader variable inspection works in NSight

---

### Phase H — Async Compute & Advanced Sync
**Duration**: 1 week | **Priority**: P2 | **Dependencies**: Phase E

**Features**:
```c
// Timeline semaphores
typedef struct KxTimelineSemaphoreCreateInfo {
    KxStructureType sType;
    const void* pNext;
    uint64_t initialValue;
} KxTimelineSemaphoreCreateInfo;

KX_API KxResult KX_CALL kxCreateTimelineSemaphore(KxDevice, const KxTimelineSemaphoreCreateInfo*, const KxAllocationCallbacks*, KxSemaphore*);
KX_API KxResult KX_CALL kxSignalSemaphore(KxDevice, const KxSemaphoreSignalInfo*);  // value + stage
KX_API KxResult KX_CALL kxWaitSemaphores(KxDevice, const KxSemaphoreWaitInfo*);      // values + stages

// Sparse resources
KX_API KxResult KX_CALL kxBindSparseMemory(KxDevice, uint32_t bindCount, const KxSparseMemoryBind*);
KX_API KxResult KX_CALL kxCmdUpdateBuffer(KxCommandBuffer, KxBuffer, uint64_t offset, uint64_t size, const void*);

// Multi-queue
typedef struct KxDeviceQueueInfo2 {
    KxStructureType sType;
    const void* pNext;
    KxQueueFlags flags;
    uint32_t queueFamilyIndex;
    uint32_t queueIndex;
} KxDeviceQueueInfo2;

KX_API KxResult KX_CALL kxGetDeviceQueue2(KxDevice, const KxDeviceQueueInfo2*, KxQueue*);
```

**CPU Implementation**:
- Timeline semaphore: `std::atomic<uint64_t>` + `std::condition_variable`
- Sparse binding: page-table emulation in `CpuDevice::bindSparseMemory()`
- Queue families: dedicated worker threads per family (graphics/compute/transfer)

**Acceptance Criteria**:
- Compute + graphics overlap measurable via timeline values
- Sparse texture streaming demo works (tile-based loading)

---

### Phase I — Descriptor Indexing & Bindless
**Duration**: 1 week | **Priority**: P2 | **Dependencies**: Phase B

**Extension Flags**:
```c
#define KX_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT     0x00000001
#define KX_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT               0x00000002
#define KX_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT             0x00000004
#define KX_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT   0x00000010
```

**CPU Bindless Emulation**:
```cpp
class CpuDescriptorBuffer {
    // Single large buffer holding all descriptor data
    struct Entry {
        uint64_t bufferAddress;      // for storage/uniform buffers
        uint64_t imageViewHandle;    // encoded image view + sampler
        uint32_t type;               // descriptor type enum
        uint32_t padding;
    };
    std::vector<Entry> buffer_;
    std::mutex mutex_;
    
    uint64_t allocate(const DescriptorInfo& info) {
        std::lock_guard lock(mutex_);
        buffer_.push_back(encode(info));
        return buffer_.size() - 1;  // index used as bindless handle
    }
};
```

**Shader Usage**:
```glsl
layout(set = 0, binding = 0) buffer DescriptorBuffer {
    DescriptorEntry entries[];
} descriptorBuffer;

layout(set = 0, binding = 1) uniform sampler2D samplers[];

void main() {
    uint idx = nonuniformEXT(materialIndex);
    DescriptorEntry entry = descriptorBuffer.entries[idx];
    // fetch texture via entry.imageViewHandle
}
```

**Acceptance Criteria**:
- 10,000 unique textures in single draw call
- No validation errors with `PARTIALLY_BOUND`
- Performance within 2× of native bindless

---

### Phase J — ARM64 & Apple Silicon
**Duration**: 1 week | **Priority**: P3 | **Dependencies**: Phase A

**Toolchains**:
```cmake
# cmake/toolchains/linux-arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)

# cmake/toolchains/macos-arm64.cmake
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_PROCESSOR arm64)
set(CMAKE_OSX_ARCHITECTURES arm64)
set(CMAKE_OSX_DEPLOYMENT_TARGET 13.0)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
```

**NEON Optimizations** (`SoftwareRasterizer.cpp`):
```cpp
// Vertex fetch: 4 vertices × 3 floats = 12 floats = 3× float32x4
float32x4_t v0 = vld1q_f32(&vertices[i*3]);
float32x4_t v1 = vld1q_f32(&vertices[i*3 + 4]);
float32x4_t v2 = vld1q_f32(&vertices[i*3 + 8]);

// Matrix multiply: 4×4 × 4×1 using FMA
float32x4_t row0 = vld1q_f32(&mvp[0]);
float32x4_t row1 = vld1q_f32(&mvp[4]);
float32x4_t row2 = vld1q_f32(&mvp[8]);
float32x4_t row3 = vld1q_f32(&mvp[12]);

float32x4_t x = vmulq_n_f32(row0, v0[0]);
x = vfmaq_n_f32(x, row1, v0[1]);
x = vfmaq_n_f32(x, row2, v0[2]);
x = vfmaq_n_f32(x, row3, v0[3]);
// x = mvp * vertex
```

**CI Matrix**:
```yaml
strategy:
  matrix:
    include:
      - os: ubuntu-latest
        arch: x64
      - os: ubuntu-latest
        arch: arm64
        docker: ubuntu:24.04-arm64
      - os: macos-latest
        arch: arm64
      - os: windows-latest
        arch: x64
```

**Acceptance Criteria**:
- All tests pass on ARM64 Linux (cross-compiled) and macOS ARM64 (native)
- NEON paths ≥ 2× scalar baseline on Cortex-A78 / M2

---

### Phase K — Vulkan GPU Backend
**Duration**: 4-6 weeks | **Priority**: P0 (post-B) | **Dependencies**: Phase B, C, D, E, F, G, H, I

**Module Responsibilities**:

| Module | Key Functions | Vulkan Mapping |
|--------|---------------|----------------|
| `VkShaderModule` | `compileToSPIRV()`, `getReflection()` | `spirv-tools::Module` → `VkShaderModuleCreateInfo` |
| `VkPipeline` | `createGraphics()`, `createCompute()` | `VkGraphicsPipelineCreateInfo` + dynamic state, `vkCreatePipelineCache` |
| `VkDescriptorSet` | `allocate()`, `update()`, `bind()` | `VkDescriptorSetLayout`, `VkDescriptorPool`, `VkDescriptorSet`, `VK_EXT_descriptor_buffer` |
| `VkMemory` | `allocate()`, `map()`, `bind()`, `getRequirements()` | `VkMemoryRequirements`, `vkAllocateMemory`, dedicated allocations, `VK_KHR_memory_dedicated` |
| `VkSwapchain` | `create()`, `acquire()`, `present()` | `vkCreateSwapchainKHR`, `vkAcquireNextImageKHR`, `vkQueuePresentKHR`, present modes |
| `VkSync` | `semaphore()`, `fence()`, `timelineSignal()`, `timelineWait()` | `VkSemaphore`, `VkFence`, `VK_KHR_timeline_semaphore`, `vkCmdWaitEvents`/`vkCmdSetEvent` |
| `VkCommandBuffer` | `begin()`, `cmdDraw()`, `cmdDispatch()`, `barrier()`, `renderPass()` | `vkCmdDraw*`, `vkCmdDispatch*`, `vkCmdPipelineBarrier2`, `vkCmdBeginRenderPass2` |

**Integration Flow**:
```cpp
// VkRuntime.cpp
std::unique_ptr<IDevice> VkRuntime::createDevice(PhysicalDevice pd, const DeviceCreateInfo& info) {
    VkInstance instance = createVkInstance(info.enabledExtensions);
    VkPhysicalDevice vkPd = selectPhysicalDevice(instance, pd);
    VkDevice device = createLogicalDevice(vkPd, info);
    
    return std::make_unique<VkDevice>(instance, vkPd, device, info);
}

// VkShaderModule.cpp
Result VkShaderModule::compileToSPIRV(const ShaderSourceCreateInfo& info, std::vector<uint32_t>* spirv) {
    spirv_tools::Module module(toSpirvContext(info.language));
    module.addSource(info.stage, info.pSource, info.sourceSize);
    module.setEntryPoint(info.pEntryPoint);
    return module.compile(spirv);
}
```

**Fallback Strategy**:
```cpp
Result BackendRegistry::loadPreferredBackend(Instance* out) {
    if (env == "vulkan" && tryLoadVulkanBackend(out)) return SUCCESS;
    if (env == "auto" && tryLoadVulkanBackend(out)) return SUCCESS;
    logWarning("Vulkan backend unavailable, falling back to CPU");
    return loadCpuBackend(out);
}
```

**Test Suite**:
- `09_vulkan_triangle` — pixel-perfect match vs `03_triangle`
- `10_vulkan_compute` — storage buffer readback correctness
- `11_vulkan_pbr` — material with 5 textures, descriptor indexing
- `12_vulkan_raytracing` — BVH + ray tracing (when `VK_KHR_ray_tracing` available)

**Acceptance Criteria**:
- 0 pixel difference vs CPU reference (exact match)
- ≥ 10× FPS on GTX 1060 / RX 580 / Intel Arc vs CPU
- Zero validation errors with `VK_LAYER_KHRONOS_validation`
- Graceful fallback to CPU on `VK_ERROR_DEVICE_LOST` / `VK_ERROR_INCOMPATIBLE_DRIVER`

---

## 4. Dependencies (Managed via CMake FetchContent)

| Dependency | Phase | Purpose | License | Version Policy |
|------------|-------|---------|---------|----------------|
| `spirv-tools` | K | IR → SPIR-V compilation, validation | Apache 2.0 | Pinned to `v2024.x` |
| `glslang` | C/K (opt) | GLSL/HLSL pre-validation | BSD-3 | Optional, off by default |
| `xcb` + `xcb-xfixes` | D | X11 surface + present | MIT | System package |
| `wayland-client` | D | Wayland surface + present | MIT | System package |
| `vulkan-headers` + `loader` | K | Vulkan API + ICD loading | Apache 2.0 | SDK 1.3.xxx |
| `SPIRV-Cross` | Future | SPIR-V → MSL/HLSL/DXIL | Apache 2.0 | When Metal/DX12 backends start |
| `shaderc` | Optional | Runtime GLSL→SPIR-V | Apache 2.0 | Alternative to spirv-tools |

**CMake FetchContent Example**:
```cmake
FetchContent_Declare(spirv_tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools
    GIT_TAG v2024.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spirv_tools)
target_link_libraries(vk_backend PRIVATE SPIRV-Tools-opt SPIRV-Tools-link)
```

---

## 5. Quality Gates (Definition of Done per Phase)

| Gate | Requirement |
|------|-------------|
| **Compile** | Zero warnings on `-Wall -Wextra -Wpedantic -Werror -Wconversion -Wshadow` |
| **Static Analysis** | `clang-tidy` (modernize, performance, readability, bugprone) clean |
| **Format** | `clang-format --dry-run --Werror` passes |
| **Unit Tests** | 100% pass, coverage ≥ 90% on new code |
| **Integration Tests** | All scenario tests pass on Windows + Linux |
| **Thread Safety** | ThreadSanitizer clean (10k iterations stress) |
| **Memory Safety** | AddressSanitizer clean (ASan + LSAN) |
| **Performance** | Benchmarks within 5% of baseline (no regressions) |
| **Documentation** | Doxygen for all public symbols; ADR for architectural decisions |
| **Packaging** | `make package` produces versioned `.zip`/`.tar.gz` with headers, libs, cmake config |

---

## 6. Risk Register & Mitigations

| ID | Risk | Probability | Impact | Mitigation |
|----|------|-------------|--------|------------|
| R1 | Scope creep across phases | High | Schedule slip | Strict phase gates; no new features mid-phase |
| R2 | SPIR-V compilation complexity | Medium | Phase K delay | Use `spirv-tools` library; unit test per SPIR-V instruction |
| R3 | Vulkan verbosity → bugs | High | Correctness issues | RAII wrappers (`VkHandle<T>`), `vk_check()`, codegen for boilerplate |
| R4 | CPU ray tracing too slow | Medium | Poor reference | BVH SAH + 4-ray packet SIMD; accept reference-level perf |
| R5 | Multi-thread races | High | Crashes/data corruption | TSan in CI; ownership model per command buffer; lock-free where possible |
| R6 | Driver bugs / flakiness | Medium | CI failures | Test on 3 GPU vendors; fallback to CPU on `DEVICE_LOST` |
| R7 | Bindless emulation overhead | Low | Perf gap | Measure; if >2× native, optimize descriptor buffer layout |
| R8 | ARM64 NEON correctness | Medium | Wrong rendering | Scalar fallback + exhaustive compare tests |

---

## 7. Decision Log (ADR References)

| ADR | Title | Status |
|-----|-------|--------|
| ADR-001 | Backend-agnostic interface in `runtime/` | Accepted (Phase B) |
| ADR-002 | SPIR-V export-only, no runtime interpreter | Accepted (Phase B) |
| ADR-003 | CPU backend = reference implementation | Accepted (Phase B) |
| ADR-004 | Extension-based feature flags (`KX_KHR_*`) | Accepted (Phase D+) |
| ADR-005 | Zero-cost abstraction: `IRuntime` pure virtual | Accepted (Phase B) |
| ADR-006 | NEON intrinsics in rasterizer hot paths | Accepted (Phase J) |
| ADR-007 | Tile-based work-stealing for parallel raster | Accepted (Phase E) |

---

## 8. Immediate Next Steps

1. **Create GitHub Issues** for each phase (A–K) with checklists
2. **Branch** `feature/phase-a-linux` → implement `PlatformLinux.cpp`, CI workflow
3. **PR Review** → merge to `main` with green CI
4. **Branch** `feature/phase-b-architecture` → extract `runtime/` interfaces, migrate `cpu_backend/`
5. **Parallel tracks** after B: C (shader stages) + D (swapchain) by different owners

---

*This roadmap is a living document. Update upon phase completion. All architectural decisions recorded as ADRs in `docs/arch/`.*