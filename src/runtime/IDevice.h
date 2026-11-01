#ifndef KROMAX_SRC_RUNTIME_IDEVICE_H
#define KROMAX_SRC_RUNTIME_IDEVICE_H

/*
 * IDevice.h
 * Backend-agnostic device interface.
 * 
 * Interface Segregation: IDevice composes fine-grained factory interfaces.
 * Backends can implement the full IDevice or only the interfaces they need.
 */

#include "kx_core.h"
#include "Expected.h"

#include "IBufferFactory.h"
#include "IImageFactory.h"
#include "IMemoryManager.h"
#include "IShaderModuleFactory.h"
#include "IPipelineFactory.h"
#include "IDescriptorFactory.h"
#include "ICommandFactory.h"
#include "IRenderPassFactory.h"
#include "ISwapchainFactory.h"
#include "ISyncFactory.h"
#include "IQueueOperations.h"
#include "ISamplerFactory.h"
#include "IThreadPoolFactory.h"

#include <cstdint>

namespace kx {

// Forward declarations for opaque handles
class Buffer;
class Image;
class ImageView;
class ShaderModule;
class Pipeline;
class PipelineLayout;
class DescriptorSetLayout;
class DescriptorSet;
class DescriptorPool;
class CommandPool;
class CommandBuffer;
class RenderPass;
class Framebuffer;
class Swapchain;
class Semaphore;
class Fence;
class Queue;
class Sampler;
class ThreadPool;

/**
 * IDevice - Composite device interface
 * 
 * Backends should inherit from this interface to provide full device functionality.
 * For minimal backends, individual factory interfaces can be implemented separately.
 * 
 * SOLID: Interface Segregation Principle - clients depend only on interfaces they use.
 * The composite IDevice allows full-featured backends while fine-grained interfaces
 * allow minimal/test backends.
 */
class IDevice :
    public IBufferFactory,
    public IImageFactory,
    public IMemoryManager,
    public IShaderModuleFactory,
    public IPipelineFactory,
    public IDescriptorFactory,
    public ICommandFactory,
    public IRenderPassFactory,
    public ISwapchainFactory,
    public ISyncFactory,
    public IQueueOperations,
    public ISamplerFactory,
    public IThreadPoolFactory
{
public:
    virtual ~IDevice() = default;

    // Device-level operations
    virtual KxResult waitIdle() = 0;
};

} // namespace kx

#endif // KROMAX_SRC_RUNTIME_IDEVICE_H