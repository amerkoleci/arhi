// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#ifndef ARHI_H_
#define ARHI_H_

#if defined(ARHI_SHARED_LIBRARY)
#    if defined(_WIN32)
#        if defined(ARHI_IMPLEMENTATION)
#            define _ARHI_EXPORT __declspec(dllexport)
#        else
#            define _ARHI_EXPORT __declspec(dllimport)
#        endif
#    else
#        if defined(ARHI_IMPLEMENTATION)
#            define _ARHI_EXPORT __attribute__((visibility("default")))
#        else
#            define _ARHI_EXPORT
#        endif
#    endif
#else
#    define _ARHI_EXPORT
#endif

#ifdef __cplusplus
#    define _ARHI_EXTERN extern "C"
#else
#    define _ARHI_EXTERN extern
#endif

#define ARHI_API _ARHI_EXTERN _ARHI_EXPORT

#if !defined(ARHI_NULLABLE)
#define ARHI_NULLABLE
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Types */
typedef uint32_t Flags;
typedef uint32_t Bool32;

/* Forward declarations */
typedef struct GPUFactory                   GPUFactory;
typedef struct GPUAdapter                   GPUAdapter;
typedef struct GPUSurfaceHandle             GPUSurfaceHandle;
typedef struct GPUSurface                   GPUSurface;
typedef struct GPUDevice                    GPUDevice;
typedef struct GPUCommandQueue              GPUCommandQueue;
typedef struct GPUCommandBuffer             GPUCommandBuffer;
typedef struct GPUComputePassEncoder        GPUComputePassEncoder;
typedef struct GPURenderPassEncoder         GPURenderPassEncoder;
typedef struct GPUBuffer                    GPUBuffer;
typedef struct GPUTexture                   GPUTexture;
typedef struct GPUSampler                   GPUSampler;
typedef struct GPUQueryHeap                 GPUQueryHeap;
typedef struct GPUBindGroupLayoutImpl* GPUBindGroupLayout;
typedef struct GPUBindGroupImpl* GPUBindGroup;
typedef struct GPUPipelineLayoutImpl* GPUPipelineLayout;
typedef struct GPUComputePipeline           GPUComputePipeline;
typedef struct GPURenderPipelineImpl* GPURenderPipeline;

/* Types */
typedef uint64_t GPUDeviceAddress;

/* Constants */
#define GPU_MAX_ADAPTER_NAME_SIZE  (256u)
#define GPU_MAX_COLOR_ATTACHMENTS (8u)
#define GPU_MAX_VERTEX_BUFFER_BINDINGS (8u)
#define GPU_WHOLE_SIZE (UINT64_MAX)
#define GPU_LOD_CLAMP_NONE (1000.0F)

/* Enums */


typedef enum LogLevel {
    LogLevel_Off = 0,
    LogLevel_Trace = 1,
    LogLevel_Debug = 2,
    LogLevel_Info = 3,
    LogLevel_Warn = 4,
    LogLevel_Error = 5,
    LogLevel_Fatal = 6,

    LogLevel_Count,
    _LogLevel_Force32 = 0x7FFFFFFF
} LogLevel;

typedef enum PixelFormat {
    PixelFormat_Undefined = 0,
    // 8-bit formats
    PixelFormat_R8Unorm,
    PixelFormat_R8Snorm,
    PixelFormat_R8Uint,
    PixelFormat_R8Sint,
    // 16-bit formats
    PixelFormat_R16Unorm,
    PixelFormat_R16Snorm,
    PixelFormat_R16Uint,
    PixelFormat_R16Sint,
    PixelFormat_R16Float,
    PixelFormat_RG8Unorm,
    PixelFormat_RG8Snorm,
    PixelFormat_RG8Uint,
    PixelFormat_RG8Sint,
    // Packed 16-Bit formats
    PixelFormat_B5G6R5Unorm,
    PixelFormat_BGR5A1Unorm,
    PixelFormat_BGRA4Unorm,
    // 32-bit formats
    PixelFormat_R32Uint,
    PixelFormat_R32Sint,
    PixelFormat_R32Float,
    PixelFormat_RG16Unorm,
    PixelFormat_RG16Snorm,
    PixelFormat_RG16Uint,
    PixelFormat_RG16Sint,
    PixelFormat_RG16Float,
    PixelFormat_RGBA8Unorm,
    PixelFormat_RGBA8UnormSrgb,
    PixelFormat_RGBA8Snorm,
    PixelFormat_RGBA8Uint,
    PixelFormat_RGBA8Sint,
    PixelFormat_BGRA8Unorm,
    PixelFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    PixelFormat_RGB10A2Unorm,
    PixelFormat_RGB10A2Uint,
    PixelFormat_RG11B10Ufloat,
    PixelFormat_RGB9E5Ufloat,
    // 64-bit formats
    PixelFormat_RG32Uint,
    PixelFormat_RG32Sint,
    PixelFormat_RG32Float,
    PixelFormat_RGBA16Unorm,
    PixelFormat_RGBA16Snorm,
    PixelFormat_RGBA16Uint,
    PixelFormat_RGBA16Sint,
    PixelFormat_RGBA16Float,
    // 128-bit formats
    PixelFormat_RGBA32Uint,
    PixelFormat_RGBA32Sint,
    PixelFormat_RGBA32Float,
    // Depth-stencil formats
    PixelFormat_Depth16Unorm,
    PixelFormat_Depth24UnormStencil8,
    PixelFormat_Depth32Float,
    PixelFormat_Depth32FloatStencil8,
    // BC compressed formats
    PixelFormat_BC1RGBAUnorm,
    PixelFormat_BC1RGBAUnormSrgb,
    PixelFormat_BC2RGBAUnorm,
    PixelFormat_BC2RGBAUnormSrgb,
    PixelFormat_BC3RGBAUnorm,
    PixelFormat_BC3RGBAUnormSrgb,
    PixelFormat_BC4RUnorm,
    PixelFormat_BC4RSnorm,
    PixelFormat_BC5RGUnorm,
    PixelFormat_BC5RGSnorm,
    PixelFormat_BC6HRGBUfloat,
    PixelFormat_BC6HRGBFloat,
    PixelFormat_BC7RGBAUnorm,
    PixelFormat_BC7RGBAUnormSrgb,
    // ETC2/EAC compressed formats
    PixelFormat_ETC2RGB8Unorm,
    PixelFormat_ETC2RGB8UnormSrgb,
    PixelFormat_ETC2RGB8A1Unorm,
    PixelFormat_ETC2RGB8A1UnormSrgb,
    PixelFormat_ETC2RGBA8Unorm,
    PixelFormat_ETC2RGBA8UnormSrgb,
    PixelFormat_EACR11Unorm,
    PixelFormat_EACR11Snorm,
    PixelFormat_EACRG11Unorm,
    PixelFormat_EACRG11Snorm,
    // ASTC compressed formats
    PixelFormat_ASTC4x4Unorm,
    PixelFormat_ASTC4x4UnormSrgb,
    PixelFormat_ASTC5x4Unorm,
    PixelFormat_ASTC5x4UnormSrgb,
    PixelFormat_ASTC5x5Unorm,
    PixelFormat_ASTC5x5UnormSrgb,
    PixelFormat_ASTC6x5Unorm,
    PixelFormat_ASTC6x5UnormSrgb,
    PixelFormat_ASTC6x6Unorm,
    PixelFormat_ASTC6x6UnormSrgb,
    PixelFormat_ASTC8x5Unorm,
    PixelFormat_ASTC8x5UnormSrgb,
    PixelFormat_ASTC8x6Unorm,
    PixelFormat_ASTC8x6UnormSrgb,
    PixelFormat_ASTC8x8Unorm,
    PixelFormat_ASTC8x8UnormSrgb,
    PixelFormat_ASTC10x5Unorm,
    PixelFormat_ASTC10x5UnormSrgb,
    PixelFormat_ASTC10x6Unorm,
    PixelFormat_ASTC10x6UnormSrgb,
    PixelFormat_ASTC10x8Unorm,
    PixelFormat_ASTC10x8UnormSrgb,
    PixelFormat_ASTC10x10Unorm,
    PixelFormat_ASTC10x10UnormSrgb,
    PixelFormat_ASTC12x10Unorm,
    PixelFormat_ASTC12x10UnormSrgb,
    PixelFormat_ASTC12x12Unorm,
    PixelFormat_ASTC12x12UnormSrgb,
    // ASTC HDR compressed formats
    PixelFormat_ASTC4x4HDR,
    PixelFormat_ASTC5x4HDR,
    PixelFormat_ASTC5x5HDR,
    PixelFormat_ASTC6x5HDR,
    PixelFormat_ASTC6x6HDR,
    PixelFormat_ASTC8x5HDR,
    PixelFormat_ASTC8x6HDR,
    PixelFormat_ASTC8x8HDR,
    PixelFormat_ASTC10x5HDR,
    PixelFormat_ASTC10x6HDR,
    PixelFormat_ASTC10x8HDR,
    PixelFormat_ASTC10x10HDR,
    PixelFormat_ASTC12x10HDR,
    PixelFormat_ASTC12x12HDR,

    // MultiAspect format
    //PixelFormat_R8BG8Biplanar420Unorm,
    //PixelFormat_R10X6BG10X6Biplanar420Unorm,

    _PixelFormat_Count,
    _PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum GPUMemoryType {
    /// CPU no access, GPU read/write
    GPUMemoryType_Private,
    /// CPU write, GPU read
    GPUMemoryType_Upload,
    /// CPU read, GPU write
    GPUMemoryType_Readback,

    GPUMemoryType_Count,
    _GPUMemoryType_Force32 = 0x7FFFFFFF
} GPUMemoryType;

typedef enum GPUTextureAspect {
    GPUTextureAspect_All = 0,
    GPUTextureAspect_DepthOnly = 1,
    GPUTextureAspect_StencilOnly = 2,

    _GPUTextureAspect_Count,
    _GPUTextureAspect_Force32 = 0x7FFFFFFF
} GPUTextureAspect;

typedef enum GPUBackendType {
    GPUBackendType_Undefined = 0,
    GPUBackendType_Null,
    GPUBackendType_Vulkan,
    GPUBackendType_D3D12,
    GPUBackendType_Metal,
    GPUBackendType_WebGPU,

    _GPUBackendType_Count,
    _GPUBackendType_Force32 = 0x7FFFFFFF
} GPUBackendType;

typedef enum GPUValidationMode {
    GPUValidationMode_Disabled = 0,
    GPUValidationMode_Enabled,
    GPUValidationMode_Verbose,
    GPUValidationMode_GPU,

    _GPUValidationMode_Count,
    _GPUValidationMode_Force32 = 0x7FFFFFFF
} GPUValidationMode;

typedef enum GPUCommandQueueType {
    GPUCommandQueueType_Graphics = 0,
    GPUCommandQueueType_Compute,
    GPUCommandQueueType_Copy,
    //GPUCommandQueueType_VideoDecode,
    //GPUCommandQueueType_VideoEncode,

    _GPUCommandQueueType_Count,
    _GPUCommandQueueType_Force32 = 0x7FFFFFFF
} GPUCommandQueueType;

typedef enum GPUVertexFormat {
    GPUVertexFormat_Undefined = 0,
    GPUVertexFormat_UByte,
    GPUVertexFormat_UByte2,
    GPUVertexFormat_UByte4,
    GPUVertexFormat_Byte,
    GPUVertexFormat_Byte2,
    GPUVertexFormat_Byte4,
    GPUVertexFormat_UByteNormalized,
    GPUVertexFormat_UByte2Normalized,
    GPUVertexFormat_UByte4Normalized,
    GPUVertexFormat_ByteNormalized,
    GPUVertexFormat_Byte2Normalized,
    GPUVertexFormat_Byte4Normalized,
    GPUVertexFormat_UShort,
    GPUVertexFormat_UShort2,
    GPUVertexFormat_UShort4,
    GPUVertexFormat_Short,
    GPUVertexFormat_Short2,
    GPUVertexFormat_Short4,
    GPUVertexFormat_UShortNormalized,
    GPUVertexFormat_UShort2Normalized,
    GPUVertexFormat_UShort4Normalized,
    GPUVertexFormat_ShortNormalized,
    GPUVertexFormat_Short2Normalized,
    GPUVertexFormat_Short4Normalized,
    GPUVertexFormat_Half,
    GPUVertexFormat_Half2,
    GPUVertexFormat_Half4,
    GPUVertexFormat_Float,
    GPUVertexFormat_Float2,
    GPUVertexFormat_Float3,
    GPUVertexFormat_Float4,
    GPUVertexFormat_UInt,
    GPUVertexFormat_UInt2,
    GPUVertexFormat_UInt3,
    GPUVertexFormat_UInt4,
    GPUVertexFormat_Int,
    GPUVertexFormat_Int2,
    GPUVertexFormat_Int3,
    GPUVertexFormat_Int4,
    GPUVertexFormat_Unorm10_10_10_2,
    GPUVertexFormat_Unorm8x4BGRA,

    _GPUVertexFormat_Count,
    _GPUVertexFormat_Force32 = 0x7FFFFFFF
} GPUVertexFormat;

typedef enum GPUTextureDimension {
    /// Undefined - default to 2D texture.
    GPUTextureDimension_Undefined = 0,
    /// One-dimensional Texture.
    GPUTextureDimension_1D = 1,
    /// Two-dimensional Texture.
    GPUTextureDimension_2D = 2,
    /// Three-dimensional Texture.
    GPUTextureDimension_3D = 3,
    /// Cubemap Texture.
    GPUTextureDimension_Cube = 4,

    _GPUTextureDimension_Force32 = 0x7FFFFFFF
} GPUTextureDimension;

typedef enum GPUIndexType {
    GPUIndexType_Uint16 = 0,
    GPUIndexType_Uint32 = 1,

    _GPUIndexType_Count,
    _GPUIndexType_Force32 = 0x7FFFFFFF
} GPUIndexType;

typedef enum GPUCompareFunction {
    GPUCompareFunction_Undefined = 0,
    GPUCompareFunction_Never,
    GPUCompareFunction_Less,
    GPUCompareFunction_Equal,
    GPUCompareFunction_LessEqual,
    GPUCompareFunction_Greater,
    GPUCompareFunction_NotEqual,
    GPUCompareFunction_GreaterEqual,
    GPUCompareFunction_Always,

    _GPUCompareFunction_Count,
    _GPUCompareFunction_Force32 = 0x7FFFFFFF
} GPUCompareFunction;

typedef enum GPUBlendFactor {
    GPUBlendFactor_Undefined = 0,
    GPUBlendFactor_Zero,
    GPUBlendFactor_One,
    GPUBlendFactor_SourceColor,
    GPUBlendFactor_OneMinusSourceColor,
    GPUBlendFactor_SourceAlpha,
    GPUBlendFactor_OneMinusSourceAlpha,
    GPUBlendFactor_DestinationColor,
    GPUBlendFactor_OneMinusDestinationColor,
    GPUBlendFactor_DestinationAlpha,
    GPUBlendFactor_OneMinusDestinationAlpha,
    GPUBlendFactor_SourceAlphaSaturated,
    GPUBlendFactor_BlendColor,
    GPUBlendFactor_OneMinusBlendColor,
    GPUBlendFactor_BlendAlpha,
    GPUBlendFactor_OneMinusBlendAlpha,
    GPUBlendFactor_Source1Color,
    GPUBlendFactor_OneMinusSource1Color,
    GPUBlendFactor_Source1Alpha,
    GPUBlendFactor_OneMinusSource1Alpha,

    _GPUBlendFactor_Count,
    _GPUBlendFactor_Force32 = 0x7FFFFFFF
} GPUBlendFactor;

typedef enum GPUBlendOperation {
    GPUBlendOperation_Undefined = 0,
    GPUBlendOperation_Add,
    GPUBlendOperation_Subtract,
    GPUBlendOperation_ReverseSubtract,
    GPUBlendOperation_Min,
    GPUBlendOperation_Max,

    _GPUBlendOperation_Count,
    _GPUBlendOperation_Force32 = 0x7FFFFFFF
} GPUBlendOperation;

typedef enum GPUStencilOperation {
    GPUStencilOperation_Undefined = 0,
    GPUStencilOperation_Keep,
    GPUStencilOperation_Zero,
    GPUStencilOperation_Replace,
    GPUStencilOperation_IncrementClamp,
    GPUStencilOperation_DecrementClamp,
    GPUStencilOperation_Invert,
    GPUStencilOperation_IncrementWrap,
    GPUStencilOperation_DecrementWrap,

    _GPUStencilOperation_Count,
    _GPUStencilOperation_Force32 = 0x7FFFFFFF
} GPUStencilOperation;

typedef enum GPULoadAction {
    GPULoadAction_Undefined = 0,
    GPULoadAction_Discard,
    GPULoadAction_Load,
    GPULoadAction_Clear,

    _GPULoadAction_Count,
    _GPULoadAction_Force32 = 0x7FFFFFFF
} GPULoadAction;

typedef enum GPUStoreAction {
    GPUStoreAction_Undefined = 0,
    GPUStoreAction_Discard,
    GPUStoreAction_Store,

    _GPUStoreAction_Count,
    _GPUStoreAction_Force32 = 0x7FFFFFFF
} GPUStoreAction;

typedef enum GPUPresentMode {
    GPUPresentMode_Undefined = 0,
    GPUPresentMode_Fifo,
    GPUPresentMode_FifoRelaxed,
    GPUPresentMode_Immediate,
    GPUPresentMode_Mailbox,

    _GPUPresentMode_Count,
    _GPUPresentMode_Force32 = 0x7FFFFFFF
} GPUPresentMode;

typedef enum GPUShaderStage {
    GPUShaderStage_Undefined,
    GPUShaderStage_Vertex,
    GPUShaderStage_Hull,
    GPUShaderStage_Domain,
    GPUShaderStage_Fragment,
    GPUShaderStage_Compute,
    GPUShaderStage_Amplification,
    GPUShaderStage_Mesh,

    _GPUShaderStage_Count,
    _GPUShaderStage_Force32 = 0x7FFFFFFF
} GPUShaderStage;

typedef enum GPUVertexStepMode {
    GPUVertexStepMode_Vertex = 0,
    GPUVertexStepMode_Instance = 1,

    _GPUVertexStepMode_Force32 = 0x7FFFFFFF
} GPUVertexStepMode;

typedef enum GPUFillMode {
    _GPUFillMode_Default = 0,
    GPUFillMode_Solid,
    GPUFillMode_Wireframe,

    _GPUFillMode_Force32 = 0x7FFFFFFF
} GPUFillMode;

typedef enum GPUCullMode {
    _GPUCullMode_Default = 0,
    GPUCullMode_None,
    GPUCullMode_Front,
    GPUCullMode_Back,

    _GPUCullMode_Force32 = 0x7FFFFFFF
} GPUCullMode;

typedef enum GPUFrontFace {
    _GPUFrontFace_Default = 0,
    GPUFrontFace_Clockwise,
    GPUFrontFace_CounterClockwise,

    _GPUFrontFace_Force32 = 0x7FFFFFFF
} GPUFrontFace;

typedef enum GPUDepthClipMode {
    _GPUDepthClipMode_Default = 0,
    GPUDepthClipMode_Clip,
    GPUDepthClipMode_Clamp,

    _GPUDepthClipMode_Force32 = 0x7FFFFFFF
} GPUDepthClipMode;

typedef enum GPUSamplerMinMagFilter {
    GPUSamplerMinMagFilter_Nearest = 0,
    GPUSamplerMinMagFilter_Linear = 1,

    _GPUSamplerMinMagFilter_Force32 = 0x7FFFFFFF
} GPUSamplerMinMagFilter;

typedef enum GPUSamplerMipFilter {
    GPUSamplerMipFilter_Nearest = 0,
    GPUSamplerMipFilter_Linear = 1,

    _GPUSamplerMipFilter_Force32 = 0x7FFFFFFF
} GPUSamplerMipFilter;

typedef enum GPUSamplerAddressMode {
    GPUSamplerAddressMode_ClampToEdge = 0,
    GPUSamplerAddressMode_MirrorClampToEdge = 1,
    GPUSamplerAddressMode_Repeat = 2,
    GPUSamplerAddressMode_MirrorRepeat = 3,

    _GPUSamplerAddressMode_Force32 = 0x7FFFFFFF
} GPUSamplerAddressMode;

typedef enum GPUPrimitiveTopology {
    GPUPrimitiveTopology_Undefined = 0,
    GPUPrimitiveTopology_TriangleList,
    GPUPrimitiveTopology_PointList,
    GPUPrimitiveTopology_LineList,
    GPUPrimitiveTopology_LineStrip,
    GPUPrimitiveTopology_TriangleStrip,
    GPUPrimitiveTopology_PatchList,

    _GPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} GPUPrimitiveTopology;

typedef enum GPUShadingRate {
    GPUShadingRate_1X1,	// Default/full shading rate
    GPUShadingRate_1X2,
    GPUShadingRate_2X1,
    GPUShadingRate_2X2,
    GPUShadingRate_2X4,
    GPUShadingRate_4X2,
    GPUShadingRate_4X4,

    _GPUShadingRate_Count,
    _GPUShadingRate_Force32 = 0x7FFFFFFF
} GPUShadingRate;

typedef enum GPUAcquireSurfaceResult {
    /// Everything is good and we can render this frame
    GPUAcquireSurfaceResult_SuccessOptimal = 0,
    /// Still OK - the surface can present the frame, but in a suboptimal way. The surface may need reconfiguration.
    GPUAcquireSurfaceResult_SuccessSuboptimal,
    /// A timeout was encountered while trying to acquire the next frame.
    GPUAcquireSurfaceResult_Timeout,
    /// The underlying surface has changed, and therefore the swap chain must be updated.
    GPUAcquireSurfaceResult_Outdated,
    /// The swap chain has been lost and needs to be recreated.
    GPUAcquireSurfaceResult_Lost,
    /// There is no more memory left to allocate a new frame.
    GPUAcquireSurfaceResult_OutOfMemory,
    /// Acquiring a texture failed with a generic error. Check error callbacks for more information.
    GPUAcquireSurfaceResult_Other,

    _GPUAcquireSurfaceResult_Force32 = 0x7FFFFFFF
} GPUAcquireSurfaceResult;

typedef enum GPUAdapterVendor {
    /// Adapter vendor is unknown
    GPUAdapterVendor_Unknown = 0,

    /// Adapter vendor is NVIDIA
    GPUAdapterVendor_NVIDIA,

    /// Adapter vendor is AMD
    GPUAdapterVendor_AMD,

    /// Adapter vendor is Intel
    GPUAdapterVendor_Intel,

    /// Adapter vendor is ARM
    GPUAdapterVendor_ARM,

    /// Adapter vendor is Qualcomm
    GPUAdapterVendor_Qualcomm,

    /// Adapter vendor is Imagination Technologies
    GPUAdapterVendor_ImgTech,

    /// Adapter vendor is Microsoft (software rasterizer)
    GPUAdapterVendor_MSFT,

    /// Adapter vendor is Apple
    GPUAdapterVendor_Apple,

    /// Adapter vendor is Mesa (software rasterizer)
    GPUAdapterVendor_Mesa,

    /// Adapter vendor is Broadcom (Raspberry Pi)
    GPUAdapterVendor_Broadcom,

    _GPUAdapterVendor_Count,
    _GPUAdapterVendor_Force32 = 0x7FFFFFFF
} GPUAdapterVendor;

typedef enum GPUAdapterType {
    GPUAdapterType_DiscreteGpu,
    GPUAdapterType_IntegratedGpu,
    GPUAdapterType_VirtualGpu,
    GPUAdapterType_Cpu,
    GPUAdapterType_Other,

    _GPUAdapterType_Force32 = 0x7FFFFFFF
} GPUAdapterType;

typedef enum GPUShaderModel {
    GPUShaderModel_6_0,
    GPUShaderModel_6_1,
    GPUShaderModel_6_2,
    GPUShaderModel_6_3,
    GPUShaderModel_6_4,
    GPUShaderModel_6_5,
    GPUShaderModel_6_6,
    GPUShaderModel_6_7,
    GPUShaderModel_6_8,
    GPUShaderModel_6_9,
    GPUShaderModel_Highest = GPUShaderModel_6_9,

    _GPUShaderModel_Force32 = 0x7FFFFFFF
} GPUShaderModel;

typedef enum GPUConservativeRasterizationTier {
    GPUConservativeRasterizationTier_NotSupported = 0,
    GPUConservativeRasterizationTier_1 = 1,
    GPUConservativeRasterizationTier_2 = 2,
    GPUConservativeRasterizationTier_3 = 3,

    _GPUConservativeRasterizationTier_Force32 = 0x7FFFFFFF
} GPUConservativeRasterizationTier;

typedef enum GPUVariableRateShadingTier {
    GPUVariableRateShadingTier_NotSupported = 0,
    GPUVariableRateShadingTier_1 = 1,
    GPUVariableRateShadingTier_2 = 2,

    _GPUVariableRateShadingTier_Force32 = 0x7FFFFFFF
} GPUVariableRateShadingTier;

typedef enum GPURayTracingTier {
    GPURayTracingTier_NotSupported = 0,
    GPURayTracingTier_1 = 1,
    GPURayTracingTier_2 = 2,

    _GPURayTracingTier_Force32 = 0x7FFFFFFF
} GPURayTracingTier;

typedef enum GPUMeshShaderTier {
    GPUMeshShaderTier_NotSupported = 0,
    GPUMeshShaderTier_1 = 1,

    _GPUMeshShaderTier_Force32 = 0x7FFFFFFF
} GPUMeshShaderTier;

typedef enum GPUFeature {
    GPUFeature_TimestampQuery,
    GPUFeature_PipelineStatisticsQuery,
    GPUFeature_TextureCompressionBC,
    GPUFeature_TextureCompressionETC2,
    GPUFeature_TextureCompressionASTC,
    GPUFeature_TextureCompressionASTC_HDR,
    GPUFeature_IndirectFirstInstance,
    GPUFeature_DualSourceBlending,
    GPUFeature_ShaderFloat16,
    GPUFeature_MultiDrawIndirect,

    GPUFeature_SamplerMirrorClampToEdge,
    GPUFeature_SamplerClampToBorder,
    GPUFeature_SamplerMinMax,

    GPUFeature_Tessellation,
    GPUFeature_DepthBoundsTest,
    GPUFeature_GPUUploadHeapSupported,
    GPUFeature_CopyQueueTimestampQuery,
    GPUFeature_CacheCoherentUMA,
    GPUFeature_ShaderOutputViewportIndex,
    GPUFeature_Predication,

    _GPUFeature_Force32 = 0x7FFFFFFF
} GPUFeature;

/* Flags/Bitmask Enums */
typedef uint32_t GPUBufferUsage;
static const GPUBufferUsage GPUBufferUsage_None = 0;
static const GPUBufferUsage GPUBufferUsage_Vertex = (1 << 0);
static const GPUBufferUsage GPUBufferUsage_Index = (1 << 1);
/// Supports Constant buffer access.
static const GPUBufferUsage GPUBufferUsage_Constant = (1 << 2);
static const GPUBufferUsage GPUBufferUsage_ShaderRead = (1 << 3);
static const GPUBufferUsage GPUBufferUsage_ShaderWrite = (1 << 4);
/// Supports indirect buffer access for indirect draw/dispatch.
static const GPUBufferUsage GPUBufferUsage_Indirect = (1 << 5);
/// Supports predication access for conditional rendering.
static const GPUBufferUsage GPUBufferUsage_Predication = (1 << 6);
/// Supports ray tracing acceleration structure usage.
static const GPUBufferUsage GPUBufferUsage_RayTracing = (1 << 7);

typedef uint32_t GPUTextureUsage;
static const GPUTextureUsage GPUTextureUsage_None = 0;
static const GPUTextureUsage GPUTextureUsage_ShaderRead = (1 << 0);
static const GPUTextureUsage GPUTextureUsage_ShaderWrite = (1 << 1);
static const GPUTextureUsage GPUTextureUsage_RenderTarget = (1 << 2);
static const GPUTextureUsage GPUTextureUsage_Transient = (1 << 3);
static const GPUTextureUsage GPUTextureUsage_ShadingRate = (1 << 4);
/// Supports shared handle usage.
static const GPUTextureUsage GPUTextureUsage_Shared = (1 << 5);

typedef uint32_t GPUColorWriteMask;
static const GPUColorWriteMask GPUColorWriteMask_None = 0x0000000000000000;
static const GPUColorWriteMask GPUColorWriteMask_Red = 0x0000000000000001;
static const GPUColorWriteMask GPUColorWriteMask_Green = 0x0000000000000002;
static const GPUColorWriteMask GPUColorWriteMask_Blue = 0x0000000000000004;
static const GPUColorWriteMask GPUColorWriteMask_Alpha = 0x0000000000000008;
static const GPUColorWriteMask GPUColorWriteMask_All = 0x000000000000000F /* Red | Green | Blue | Alpha */;

/* Structs */
typedef struct RHIColor {
    float r;
    float g;
    float b;
    float a;
} RHIColor;

typedef struct RHIViewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
} RHIViewport;

typedef struct RHIScissorRect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} RHIScissorRect;

typedef struct RHIFactoryDesc {
    GPUBackendType preferredBackend;
    GPUValidationMode validationMode;
} RHIFactoryDesc;

/* Indirect Commands Structs */
typedef struct RHIDispatchIndirectCommand {
    uint32_t groupCountX;
    uint32_t groupCountY;
    uint32_t groupCountZ;
} RHIDispatchIndirectCommand;

typedef struct RHIDrawIndexedIndirectCommand {
    uint32_t    indexCount;
    uint32_t    instanceCount;
    uint32_t    firstIndex;
    int32_t     baseVertex;
    uint32_t    firstInstance;
} RHIDrawIndexedIndirectCommand;

typedef struct RHIDrawIndirectCommand {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
} RHIDrawIndirectCommand;

#endif /* ARHI_H_ */
