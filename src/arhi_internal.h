// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#pragma once

#include "arhi.h"
#include <algorithm>
#include <atomic>

/* Compiler defines */
#if defined(__clang__)
#define ARHI_FORCE_INLINE inline __attribute__((__always_inline__))
#define ARHI_LIKELY(x) __builtin_expect(!!(x), 1)
#define ARHI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ARHI_UNREACHABLE() __builtin_unreachable()
#define ARHI_DEBUG_BREAK() __builtin_trap()
// CLANG ENABLE/DISABLE WARNING DEFINITION
#define ARHI_DISABLE_WARNINGS() \
    _Pragma("clang diagnostic push")\
	_Pragma("clang diagnostic ignored \"-Wall\"") \
	_Pragma("clang diagnostic ignored \"-Wextra\"") \
	_Pragma("clang diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-completeness\"") \
    _Pragma("clang diagnostic ignored \"-Wnullability-extension\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-function\"") \
    _Pragma("clang diagnostic ignored \"-Wtypedef-redefinition\"") \
    _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")

#define ARHI_ENABLE_WARNINGS() _Pragma("clang diagnostic pop")
#elif defined(__GNUC__) || defined(__GNUG__)
#define ARHI_FORCE_INLINE inline __attribute__((__always_inline__))
#define ARHI_LIKELY(x) __builtin_expect(!!(x), 1)
#define ARHI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define ARHI_UNREACHABLE() __builtin_unreachable()
#define ARHI_DEBUG_BREAK() __builtin_trap()
// GCC ENABLE/DISABLE WARNING DEFINITION
#	define ARHI_DISABLE_WARNINGS() \
	_Pragma("GCC diagnostic push") \
	_Pragma("GCC diagnostic ignored \"-Wall\"") \
	_Pragma("GCC diagnostic ignored \"-Wextra\"") \
	_Pragma("GCC diagnostic ignored \"-Wtautological-compare\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-function\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-field-initializers\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-result\"")

#define ARHI_ENABLE_WARNINGS() _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define ARHI_FORCE_INLINE __forceinline
#define ARHI_LIKELY(x) (x)
#define ARHI_UNLIKELY(x) (x)
#define ARHI_UNREACHABLE() __assume(false)
#define ARHI_DEBUG_BREAK() __debugbreak()
#define ARHI_DISABLE_WARNINGS() __pragma(warning(push, 0))
#define ARHI_ENABLE_WARNINGS() __pragma(warning(pop))
#endif

#define ARHI_STRINGIZE_HELPER(X) #X
#define ARHI_STRINGIZE(X) ARHI_STRINGIZE_HELPER(X)
#define ARHI_UNUSED(x) (void)(x)

// Macro for determining size of arrays.
#if defined(_MSC_VER)
#   define ARHI_COUNT_OF(arr) _countof(arr)
#else
#   define ARHI_COUNT_OF(arr) (sizeof(arr) / sizeof(arr[0]))
#endif

// Always turn on asserts in Debug mode
#if defined(_DEBUG) && !defined(ARHI_ENABLE_ASSERTS)
#define ARHI_ENABLE_ASSERTS
#endif

#ifdef ARHI_ENABLE_ASSERTS
#   include <assert.h>
#   define ARHI_ASSERT(c) assert(c)
#else
#   define ARHI_ASSERT(...) ((void)0)
#endif

#define SAFE_RELEASE(obj) do \
{ \
  if ((obj)) { \
    (obj)->Release(); \
    (obj) = nullptr; \
  } \
} while(0)

#if defined(_WIN32)
// Use the C++ standard templated min/max
#define NOMINMAX
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP
#define NOMCX
#define NOSERVICE
#define NOHELP
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define MAX_LOG_MESSAGE_SIZE        1024
_ARHI_EXTERN void arhi_log(RHILogLevel level, const char* message);
_ARHI_EXTERN void arhi_log_format(RHILogLevel level, const char* format, ...);
_ARHI_EXTERN void arhi_log_info(const char* format, ...);
_ARHI_EXTERN void arhi_log_warn(const char* format, ...);
_ARHI_EXTERN void arhi_log_error(const char* format, ...);

// Forward declaration for native handle
struct IUnknown;
struct ANativeWindow;
struct wl_display;
struct wl_surface;

enum class TextureLayout : uint8_t
{
    Undefined,
    CopySource,
    CopyDest,
    ResolveSource,
    ResolveDest,
    ShaderResource,
    UnorderedAccess,
    RenderTarget,
    DepthWrite,
    DepthRead,

    Present,
    ShadingRateSurface,
};

class RHIObject
{
protected:
    RHIObject() = default;
    virtual ~RHIObject() = default;

public:
    // Non-copyable and non-movable
    RHIObject(const RHIObject&) = delete;
    RHIObject& operator=(const RHIObject&) = delete;
    RHIObject(RHIObject&&) = delete;
    RHIObject& operator=(RHIObject&&) = delete;

    virtual uint32_t AddRef()
    {
        return ++refCount;
    }

    virtual uint32_t Release()
    {
        uint32_t newCount = --refCount;
        if (newCount == 0) {
            delete this;
        }
        return newCount;
    }

    virtual void SetLabel([[maybe_unused]] const char* label)
    {}

private:
    std::atomic_uint32_t refCount = 1;
};

struct GPUBuffer : public RHIObject
{
    //GPUBufferDesc desc;
    GPUAddress address = 0;
};

struct GPUTexture : public RHIObject
{
    //GPUTextureDesc desc;
};

struct GPUSampler : public RHIObject
{

};

struct GPUQueryHeap : public RHIObject
{

};

struct GPUBindGroupLayoutImpl : public RHIObject
{

};

struct GPUBindGroupImpl : public RHIObject
{

};

struct GPUPipelineLayoutImpl : public RHIObject
{

};

struct GPUComputePipeline : public RHIObject
{

};

struct GPURenderPipelineImpl : public RHIObject
{

};

struct GPUCommandEncoder : public RHIObject
{
    virtual void EndEncoding() = 0;
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;
};

struct GPUComputePassEncoder : public GPUCommandEncoder
{
    virtual void SetPipeline(GPUComputePipeline* pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    virtual void DispatchIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
};

struct GPURenderPassEncoder : public GPUCommandEncoder
{
    //virtual void SetViewport(const GPUViewport* viewport) = 0;
    //virtual void SetViewports(uint32_t viewportCount, const GPUViewport* viewports) = 0;
    //virtual void SetScissorRect(const GPUScissorRect* scissorRect) = 0;
    //virtual void SetScissorRects(uint32_t scissorCount, const GPUScissorRect* scissorRects) = 0;
    virtual void SetBlendColor(const RHIColor* color) = 0;
    //virtual void SetStencilReference(uint32_t reference) = 0;

    virtual void SetVertexBuffer(uint32_t slot, GPUBuffer* buffer, uint64_t offset) = 0;
    virtual void SetIndexBuffer(GPUBuffer* buffer, RHIIndexType type, uint64_t offset) = 0;
    //virtual void SetPipeline(GPURenderPipeline pipeline) = 0;
    virtual void SetPushConstants(uint32_t pushConstantIndex, const void* data, uint32_t size) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) = 0;
    virtual void DrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;
    virtual void DrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset) = 0;

    virtual void MultiDrawIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;
    virtual void MultiDrawIndexedIndirect(GPUBuffer* indirectBuffer, uint64_t indirectBufferOffset, uint32_t maxDrawCount, GPUBuffer* drawCountBuffer = nullptr, uint64_t drawCountBufferOffset = 0) = 0;

    virtual void SetShadingRate(GPUShadingRate rate) = 0;
};

struct GPUCommandBuffer : public RHIObject
{
    virtual void PushDebugGroup(const char* groupLabel) const = 0;
    virtual void PopDebugGroup() const = 0;
    virtual void InsertDebugMarker(const char* markerLabel) const = 0;

    //virtual GPUAcquireSurfaceResult AcquireSurfaceTexture(GPUSurface* surface, GPUTexture** surfaceTexture) = 0;
    //virtual GPUComputePassEncoder* BeginComputePass(const GPUComputePassDesc& desc) = 0;
    //virtual GPURenderPassEncoder* BeginRenderPass(const GPURenderPassDesc& desc) = 0;
};

struct GPUCommandQueue : public RHIObject
{
    virtual RHIQueueType GetType() const = 0;

    virtual void WaitIdle() = 0;
    ///virtual GPUCommandBuffer* AcquireCommandBuffer(const GPUCommandBufferDesc* desc) = 0;
    virtual void Submit(uint32_t numCommandBuffers, GPUCommandBuffer** commandBuffers) = 0;
};

struct GPUDevice : public RHIObject
{
    virtual bool HasFeature(RHIFeature feature) const = 0;
    virtual GPUCommandQueue* GetQueue(RHIQueueType type) = 0;
    virtual void WaitIdle() = 0;
    virtual uint64_t CommitFrame() = 0;

    virtual uint64_t GetTimestampFrequency() const = 0;

    /* Resource creation */
    //virtual GPUBuffer* CreateBuffer(const GPUBufferDesc& desc, const void* pInitialData) = 0;
    //virtual GPUTexture* CreateTexture(const GPUTextureDesc& desc, const GPUTextureData* pInitialData) = 0;
    //virtual GPUSampler* CreateSampler(const GPUSamplerDesc& desc) = 0;
    //virtual GPUBindGroupLayoutImpl* CreateBindGroupLayout(const GPUBindGroupLayoutDesc& desc) = 0;
    //virtual GPUPipelineLayoutImpl* CreatePipelineLayout(const GPUPipelineLayoutDesc& desc) = 0;
    //virtual GPUComputePipeline* CreateComputePipeline(const GPUComputePipelineDesc& desc) = 0;
    //virtual GPURenderPipelineImpl* CreateRenderPipeline(const GPURenderPipelineDesc& desc) = 0;
    //virtual GPUQueryHeap* CreateQueryHeap(const GPUQueryHeapDesc& desc) = 0;
};

enum class RHISurfaceSourceType : uint8_t
{
    Invalid,
    AndroidWindow,
    MetalLayer,
    WindowsHWND,
    IDCompositionVisual,
    SwapChainPanel,
    SurfaceHandle,
    WaylandSurface,
    XlibWindow,
};

struct RHISurfaceSourceImpl
{
    RHISurfaceSourceType type = RHISurfaceSourceType::Invalid;

    // MetalLayer
    void* metalLayer = nullptr;
    // ANativeWindow
    ANativeWindow* androidWindow = nullptr;

    // Wayland
    wl_display* waylandDisplay = nullptr;
    wl_surface* waylandSurface = nullptr;
    // Xlib
    void* xDisplay = nullptr;
    uint64_t xWindow = 0;

#if defined(_WIN32)
    // WindowsHwnd
    HWND hwnd = nullptr;
#endif

    // IDCompositionVisual/SwapChainPanel
    //IUnknown* idCompositionVisualOrSwapChainPanel = nullptr;
    // SurfaceHandle
    //void* surfaceHandle = nullptr;
};

struct RHISurfaceImpl : public RHIObject
{
    //virtual void GetCapabilities(GPUAdapter* adapter, GPUSurfaceCapabilities* capabilities) const = 0;
    //virtual bool Configure(const GPUSurfaceConfig* config_) = 0;
    //virtual void Unconfigure() = 0;

    //GPUSurfaceConfig config;
};

struct RHIAdapterImpl : public RHIObject
{
    virtual RHIAdapterType GetType() const = 0;
    virtual void GetInfo(RHIAdapterInfo* info) const = 0;
    //virtual void GetLimits(GPUAdapterLimits* limits) const = 0;
    //virtual bool HasFeature(GPUFeature feature) const = 0;
    //virtual GPUDevice* CreateDevice(const GPUDeviceDesc& desc) = 0;
};

struct RHIFactoryImpl : public RHIObject
{
public:
    virtual ~RHIFactoryImpl() = default;

    virtual RHIBackend GetBackend() const = 0;
    virtual uint32_t GetAdapterCount() const = 0;
    virtual RHIAdapter GetAdapter(uint32_t index) const = 0;
    virtual RHISurface CreateSurface(RHISurfaceSource source) = 0;
};

namespace
{
    /// Check if inV is a power of 2
    template <typename T>
    constexpr bool IsPowerOf2(T value)
    {
        return (value & (value - 1)) == 0;
    }

    template <typename T>
    inline T AlignUp(T val, T alignment)
    {
        ARHI_ASSERT(IsPowerOf2(alignment));
        return (val + alignment - 1) & ~(alignment - 1);
    }


    // Returns smallest power of 2 greater or equal to v.
    static inline uint32_t NextPow2(uint32_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v++;
        return v;
    }

    static inline uint64_t NextPow2(uint64_t v)
    {
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        v |= v >> 32;
        v++;
        return v;
    }

    namespace string
    {
        inline void copy_safe(char* dst, size_t dstSize, const char* src)
        {
            if (!dst || !src || dstSize == 0)
            {
                return;
            }

            // Copy characters from src to dst until either (dstSize - 1) is exhausted or we hit a null terminator in src.
            while (dstSize > 1 && *src)
            {
                *dst++ = *src++;
                --dstSize;
            }
            // Fill the rest of dst with null characters to ensure null-termination.
            while (dstSize > 0)
            {
                *dst++ = 0;
                --dstSize;
            }
        }
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t mipLevelCount) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount;
    }

    constexpr uint32_t CalculateSubresource(uint32_t mipLevel, uint32_t arrayLayer, uint32_t planeSlice, uint32_t mipLevelCount, uint32_t arrayLayers) noexcept
    {
        return mipLevel + arrayLayer * mipLevelCount + planeSlice * mipLevelCount * arrayLayers;
    }

    inline uint32_t GetMipLevelCount(uint32_t width, uint32_t height, uint32_t depth = 1u, uint32_t minDimension = 1u, uint32_t requiredAlignment = 1u)
    {
        uint32_t mips = 1;
        while (width > minDimension || height > minDimension || depth > minDimension)
        {
            width = std::max(minDimension, width >> 1u);
            height = std::max(minDimension, height >> 1u);
            depth = std::max(minDimension, depth >> 1u);
            if (
                AlignUp(width, requiredAlignment) != width ||
                AlignUp(height, requiredAlignment) != height ||
                AlignUp(depth, requiredAlignment) != depth
                )
                break;
            mips++;
        }
        return mips;
    }

    //inline bool BlendEnabled(const GPURenderPipelineColorAttachmentDesc* state)
    //{
    //    return
    //        state->colorBlendOperation != GPUBlendOperation_Add
    //        || state->destColorBlendFactor != GPUBlendFactor_Zero
    //        || state->srcColorBlendFactor != GPUBlendFactor_One
    //        || state->alphaBlendOperation != GPUBlendOperation_Add
    //        || state->destAlphaBlendFactor != GPUBlendFactor_Zero
    //        || state->srcAlphaBlendFactor != GPUBlendFactor_One;
    //}
    //
    //inline bool StencilTestEnabled(const GPUDepthStencilState& depthStencil)
    //{
    //    return depthStencil.backFace.compareFunction != GPUCompareFunction_Always
    //        || depthStencil.backFace.failOperation != GPUStencilOperation_Keep
    //        || depthStencil.backFace.depthFailOperation != GPUStencilOperation_Keep
    //        || depthStencil.backFace.passOperation != GPUStencilOperation_Keep
    //        || depthStencil.frontFace.compareFunction != GPUCompareFunction_Always
    //        || depthStencil.frontFace.failOperation != GPUStencilOperation_Keep
    //        || depthStencil.frontFace.depthFailOperation != GPUStencilOperation_Keep
    //        || depthStencil.frontFace.passOperation != GPUStencilOperation_Keep;
    //}
}

#if defined(ARHI_VULKAN)
_ARHI_EXTERN bool Vulkan_IsSupported(void);
_ARHI_EXTERN RHIFactoryImpl* Vulkan_CreateFactory(const RHIFactoryDesc* desc);
#endif

#if defined(ARHI_D3D12)
_ARHI_EXTERN bool D3D12_IsSupported(void);
_ARHI_EXTERN RHIFactoryImpl* D3D12_CreateFactory(const RHIFactoryDesc* desc);
#endif

#if defined(ARHI_METAL)
_ARHI_EXTERN bool Metal_IsSupported(void);
_ARHI_EXTERN RHIFactoryImpl* Metal_CreateFactory(const RHIFactoryDesc* desc);
#endif
