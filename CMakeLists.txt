cmake_minimum_required(VERSION 3.24)
project(Kromax
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(KROMAX_SOVERSION 0)
set(KROMAX_VERSION "${project_VERSION}")

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(KX_BUILD_DRIVER_AMDGPU "Build the amdgpu hardware driver skeleton (disabled by default)" OFF)
option(KX_BUILD_EXAMPLES "Build examples" ON)
option(KX_BUILD_TESTS "Build tests" ON)
option(KX_DISABLE_EXCEPTIONS "Disable C++ exceptions in the internal implementation" OFF)
option(KX_DISABLE_RTTI "Disable RTTI in the internal implementation" OFF)

# ---------------------------------------------------------------------------
# C++ / C standard enforcement
# ---------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Output directories
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Include CMake modules
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(KxCompilerFlags)
include(KxSanitizers)

# ---------------------------------------------------------------------------
# Kromax core library
# ---------------------------------------------------------------------------
add_library(kromax
    include/kromax/kx_platform.h
    include/kromax/kx_core.h
    include/kromax/kx_display.h
    include/kromax/kx_debug.h
    include/kromax/kromax.h

    src/core/Expected.h
    src/core/Allocator.h
    src/core/Handle.h
    src/core/Object.h
    src/core/Dispatch.h
    src/core/Logging.h
    src/core/Logging.cpp
    src/core/Instance.h
    src/core/Instance.cpp
    src/core/PhysicalDevice.h
    src/core/PhysicalDevice.cpp
    src/core/Device.h
    src/core/Device.cpp
    src/core/DeviceDispatch.cpp
    src/core/DeviceDispatchPipeline.cpp
    src/core/DeviceDispatchDescriptor.cpp
    src/core/Queue.h
    src/core/Queue.cpp

    src/api/api.cpp
    src/api/api_dispatch.cpp
    src/api/api_instance.cpp
    src/api/api_device.cpp
    src/api/api_memory.cpp
    src/api/api_command.cpp
    src/api/api_display_core.cpp
    src/api/api_display_swapchain.cpp
    
    src/api/api_renderpass.cpp
    src/api/api_pipeline_create.cpp
    src/api/api_pipeline_commands.cpp
    src/api/api_pipeline_utils.cpp
    src/api/api_descriptor.cpp

    src/driver/IDriver.h
    src/driver/IDriverDevice.h
    src/driver/DriverRegistry.h
    src/driver/DriverRegistry.cpp

    src/driver/software/SoftwareDriver.h
    src/driver/software/SoftwareDriver.cpp
    src/driver/software/SoftwareDevice.h
    src/driver/software/SoftwareDevice.cpp
    src/driver/software/SoftwareDeviceMemory.cpp
    src/driver/software/SoftwareDeviceBuffer.cpp
    src/driver/software/SoftwareDeviceImage.cpp
    src/driver/software/SoftwareDeviceRenderPass.cpp
    src/driver/software/SoftwareDeviceCommand.cpp
    src/driver/software/SoftwareDeviceCopy.cpp
    src/driver/software/SoftwareDeviceDraw.cpp
    src/driver/software/SoftwareDeviceFormat.cpp
    src/driver/software/SoftwareDeviceMethods.cpp
    src/driver/software/SoftwareDeviceShader.cpp
    src/driver/software/SoftwareMemory.h
    src/driver/software/SoftwareMemory.cpp
    src/driver/software/SoftwareBuffer.h
    src/driver/software/SoftwareBuffer.cpp
    src/driver/software/SoftwareImage.h
    src/driver/software/SoftwareImage.cpp
    src/driver/software/SoftwareImageView.h
    src/driver/software/SoftwareImageView.cpp
    src/driver/software/SoftwareRenderPass.h
    src/driver/software/SoftwareRenderPass.cpp
    src/driver/software/SoftwareFramebuffer.h
    src/driver/software/SoftwareFramebuffer.cpp
    src/driver/software/SoftwareCommandBuffer.h
    src/driver/software/SoftwareCommandBuffer.cpp
    src/driver/software/SoftwarePipeline.h
    src/driver/software/SoftwarePipeline.cpp
    src/driver/software/SoftwarePipelineLayout.h
    src/driver/software/SoftwarePipelineLayout.cpp
    src/driver/software/SoftwareShaderModule.h
    src/driver/software/SoftwareShaderModule.cpp
    src/driver/software/SoftwareRasterizer.h
    src/driver/software/SoftwareRasterizerDraw.cpp
    src/driver/software/SoftwareRasterizerDrawIndexed.cpp
    src/driver/software/SoftwareRasterizerVertex.cpp
    src/driver/software/SoftwareRasterizerFragment.cpp
    src/driver/software/SoftwareRasterizerTriangle.cpp

    src/driver/software/SoftwareCommandPool.h
    src/driver/software/SoftwareCommandBufferCommands.h
    src/driver/software/SoftwareDescriptorSetLayout.h
    src/driver/software/SoftwareDescriptorPool.h
    src/driver/software/SoftwareDescriptorSet.h
    src/driver/software/SoftwareSampler.h

    src/driver/amdgpu/AmdGpuDriver.h

    src/display/IDisplayBackend.h
    src/display/offscreen/OffscreenBackend.h
    src/display/offscreen/OffscreenBackend.cpp
    src/display/offscreen/PpmWriter.h
    src/display/kms/KmsBackend.h
    src/display/kms/KmsBackend.cpp

    src/layers/ValidationLayer.h
    src/layers/ValidationLayer.cpp
)

target_include_directories(kromax PRIVATE
    src
    src/core
    src/api
    src/driver
    src/driver/software
    src/driver/amdgpu
    src/display
    src/display/offscreen
    src/display/kms
    src/layers
    include/kromax
)

# Internal C++ sources (compiled as C++20 by extension)
target_sources(kromax PRIVATE
    src/core/Logging.cpp
    src/core/Instance.cpp
    src/core/PhysicalDevice.cpp
    src/core/Device.cpp
    src/core/DeviceDispatch.cpp
    src/core/DeviceDispatchPipeline.cpp
    src/core/Queue.cpp
    src/api/api.cpp
    src/api/api_dispatch.cpp
    src/api/api_instance.cpp
    src/api/api_device.cpp
    src/api/api_memory.cpp
    src/api/api_command.cpp
    
    src/api/api_renderpass.cpp
    src/api/api_pipeline_create.cpp
    src/api/api_pipeline_commands.cpp
    src/api/api_pipeline_utils.cpp
    src/driver/DriverRegistry.cpp
    src/driver/software/SoftwareDriver.cpp
    src/driver/software/SoftwareDevice.cpp
    src/driver/software/SoftwareDeviceMemory.cpp
    src/driver/software/SoftwareDeviceBuffer.cpp
    src/driver/software/SoftwareDeviceImage.cpp
    src/driver/software/SoftwareDeviceRenderPass.cpp
    src/driver/software/SoftwareDeviceCommand.cpp
    src/driver/software/SoftwareDeviceCopy.cpp
    src/driver/software/SoftwareDeviceDraw.cpp
    src/driver/software/SoftwareDeviceFormat.cpp
    src/driver/software/SoftwareDeviceMethods.cpp
    src/driver/software/SoftwareDeviceShader.cpp
    src/driver/software/SoftwareDevicePipeline.cpp
    src/driver/software/SoftwareMemory.cpp
    src/driver/software/SoftwareBuffer.cpp
    src/driver/software/SoftwareImage.cpp
    src/driver/software/SoftwareImageView.cpp
    src/driver/software/SoftwareRenderPass.cpp
    src/driver/software/SoftwareFramebuffer.cpp
    src/driver/software/SoftwareCommandBuffer.cpp
    src/driver/software/SoftwarePipeline.cpp
    src/driver/software/SoftwarePipelineLayout.cpp
    src/driver/software/SoftwareShaderModule.cpp
    src/driver/software/SoftwareDescriptorSet.cpp
    src/driver/software/SoftwareRasterizerDraw.cpp
    src/driver/software/SoftwareRasterizerDrawIndexed.cpp
    src/driver/software/SoftwareRasterizerVertex.cpp
    src/driver/software/SoftwareRasterizerFragment.cpp
    src/driver/software/SoftwareRasterizerTriangle.cpp
    src/display/offscreen/OffscreenBackend.cpp
    src/display/kms/KmsBackend.cpp
    src/layers/ValidationLayer.cpp
)

# Compiler flags
kx_apply_compiler_flags(kromax)

# Sanitizers (optional, controlled via preset)
kx_apply_sanitizers(kromax)

# Optional amdgpu skeleton
if(KX_BUILD_DRIVER_AMDGPU)
    target_sources(kromax PRIVATE
        src/driver/amdgpu/AmdGpuDriver.cpp
    )
    target_compile_definitions(kromax PRIVATE KX_DRIVER_AMDGPU_ENABLED)
endif()

# libdrm for KMS backend (optional)
find_library(LIBDRM libdrm)
if(LIBDRM)
    target_link_libraries(kromax PRIVATE ${LIBDRM})
    target_compile_definitions(kromax PRIVATE KX_HAVE_LIBDRM)
endif()

# Version
set_target_properties(kromax PROPERTIES
    VERSION ${KROMAX_SOVERSION}
    SOVERSION ${KROMAX_SOVERSION}
)

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(KX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(KX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
include(KxInstall)
kx_install_targets(kromax)