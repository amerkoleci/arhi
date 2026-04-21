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
typedef struct RHIFactoryImpl*              RHIFactory;
typedef struct RHIAdapterImpl*              RHIAdapter;
typedef struct RHISurfaceSourceImpl*        RHISurfaceSource;
typedef struct RHISurfaceImpl*              RHISurface;

/* Types */
typedef uint64_t GPUAddress;

/* Constants */
#define ARHI_VERSION_MAJOR  1
#define ARHI_VERSION_MINOR	0
#define ARHI_VERSION_PATCH	0

#define RHI_MAX_ADAPTER_NAME_SIZE  (256u)
#define RHI_MAX_COLOR_ATTACHMENTS (8u)
#define RHI_MAX_VERTEX_BUFFER_BINDINGS (8u)
#define RHI_WHOLE_SIZE (UINT64_MAX)
#define RHI_LOD_CLAMP_NONE (1000.0F)

/* Enums */
typedef enum RHILogLevel {
    RHILogLevel_Off = 0,
    RHILogLevel_Trace = 1,
    RHILogLevel_Debug = 2,
    RHILogLevel_Info = 3,
    RHILogLevel_Warn = 4,
    RHILogLevel_Error = 5,
    RHILogLevel_Fatal = 6,

    RHILogLevel_Count,
    _RHILogLevel_Force32 = 0x7FFFFFFF
} RHILogLevel;

typedef enum RHIPixelFormat {
    RHIPixelFormat_Undefined = 0,
    // 8-bit formats
    RHIPixelFormat_R8Unorm,
    RHIPixelFormat_R8Snorm,
    RHIPixelFormat_R8Uint,
    RHIPixelFormat_R8Sint,
    // 16-bit formats
    RHIPixelFormat_R16Unorm,
    RHIPixelFormat_R16Snorm,
    RHIPixelFormat_R16Uint,
    RHIPixelFormat_R16Sint,
    RHIPixelFormat_R16Float,
    RHIPixelFormat_RG8Unorm,
    RHIPixelFormat_RG8Snorm,
    RHIPixelFormat_RG8Uint,
    RHIPixelFormat_RG8Sint,
    // Packed 16-Bit formats
    RHIPixelFormat_B5G6R5Unorm,
    RHIPixelFormat_BGR5A1Unorm,
    RHIPixelFormat_BGRA4Unorm,
    // 32-bit formats
    RHIPixelFormat_R32Uint,
    RHIPixelFormat_R32Sint,
    RHIPixelFormat_R32Float,
    RHIPixelFormat_RG16Unorm,
    RHIPixelFormat_RG16Snorm,
    RHIPixelFormat_RG16Uint,
    RHIPixelFormat_RG16Sint,
    RHIPixelFormat_RG16Float,
    RHIPixelFormat_RGBA8Unorm,
    RHIPixelFormat_RGBA8UnormSrgb,
    RHIPixelFormat_RGBA8Snorm,
    RHIPixelFormat_RGBA8Uint,
    RHIPixelFormat_RGBA8Sint,
    RHIPixelFormat_BGRA8Unorm,
    RHIPixelFormat_BGRA8UnormSrgb,
    // Packed 32-Bit Pixel Formats
    RHIPixelFormat_RGB10A2Unorm,
    RHIPixelFormat_RGB10A2Uint,
    RHIPixelFormat_RG11B10Ufloat,
    RHIPixelFormat_RGB9E5Ufloat,
    // 64-bit formats
    RHIPixelFormat_RG32Uint,
    RHIPixelFormat_RG32Sint,
    RHIPixelFormat_RG32Float,
    RHIPixelFormat_RGBA16Unorm,
    RHIPixelFormat_RGBA16Snorm,
    RHIPixelFormat_RGBA16Uint,
    RHIPixelFormat_RGBA16Sint,
    RHIPixelFormat_RGBA16Float,
    // 128-bit formats
    RHIPixelFormat_RGBA32Uint,
    RHIPixelFormat_RGBA32Sint,
    RHIPixelFormat_RGBA32Float,
    // Depth-stencil formats
    RHIPixelFormat_Depth16Unorm,
    RHIPixelFormat_Depth24UnormStencil8,
    RHIPixelFormat_Depth32Float,
    RHIPixelFormat_Depth32FloatStencil8,
    // BC compressed formats
    RHIPixelFormat_BC1RGBAUnorm,
    RHIPixelFormat_BC1RGBAUnormSrgb,
    RHIPixelFormat_BC2RGBAUnorm,
    RHIPixelFormat_BC2RGBAUnormSrgb,
    RHIPixelFormat_BC3RGBAUnorm,
    RHIPixelFormat_BC3RGBAUnormSrgb,
    RHIPixelFormat_BC4RUnorm,
    RHIPixelFormat_BC4RSnorm,
    RHIPixelFormat_BC5RGUnorm,
    RHIPixelFormat_BC5RGSnorm,
    RHIPixelFormat_BC6HRGBUfloat,
    RHIPixelFormat_BC6HRGBFloat,
    RHIPixelFormat_BC7RGBAUnorm,
    RHIPixelFormat_BC7RGBAUnormSrgb,
    // ETC2/EAC compressed formats
    RHIPixelFormat_ETC2RGB8Unorm,
    RHIPixelFormat_ETC2RGB8UnormSrgb,
    RHIPixelFormat_ETC2RGB8A1Unorm,
    RHIPixelFormat_ETC2RGB8A1UnormSrgb,
    RHIPixelFormat_ETC2RGBA8Unorm,
    RHIPixelFormat_ETC2RGBA8UnormSrgb,
    RHIPixelFormat_EACR11Unorm,
    RHIPixelFormat_EACR11Snorm,
    RHIPixelFormat_EACRG11Unorm,
    RHIPixelFormat_EACRG11Snorm,
    // ASTC compressed formats
    RHIPixelFormat_ASTC4x4Unorm,
    RHIPixelFormat_ASTC4x4UnormSrgb,
    RHIPixelFormat_ASTC5x4Unorm,
    RHIPixelFormat_ASTC5x4UnormSrgb,
    RHIPixelFormat_ASTC5x5Unorm,
    RHIPixelFormat_ASTC5x5UnormSrgb,
    RHIPixelFormat_ASTC6x5Unorm,
    RHIPixelFormat_ASTC6x5UnormSrgb,
    RHIPixelFormat_ASTC6x6Unorm,
    RHIPixelFormat_ASTC6x6UnormSrgb,
    RHIPixelFormat_ASTC8x5Unorm,
    RHIPixelFormat_ASTC8x5UnormSrgb,
    RHIPixelFormat_ASTC8x6Unorm,
    RHIPixelFormat_ASTC8x6UnormSrgb,
    RHIPixelFormat_ASTC8x8Unorm,
    RHIPixelFormat_ASTC8x8UnormSrgb,
    RHIPixelFormat_ASTC10x5Unorm,
    RHIPixelFormat_ASTC10x5UnormSrgb,
    RHIPixelFormat_ASTC10x6Unorm,
    RHIPixelFormat_ASTC10x6UnormSrgb,
    RHIPixelFormat_ASTC10x8Unorm,
    RHIPixelFormat_ASTC10x8UnormSrgb,
    RHIPixelFormat_ASTC10x10Unorm,
    RHIPixelFormat_ASTC10x10UnormSrgb,
    RHIPixelFormat_ASTC12x10Unorm,
    RHIPixelFormat_ASTC12x10UnormSrgb,
    RHIPixelFormat_ASTC12x12Unorm,
    RHIPixelFormat_ASTC12x12UnormSrgb,
    // ASTC HDR compressed formats
    RHIPixelFormat_ASTC4x4HDR,
    RHIPixelFormat_ASTC5x4HDR,
    RHIPixelFormat_ASTC5x5HDR,
    RHIPixelFormat_ASTC6x5HDR,
    RHIPixelFormat_ASTC6x6HDR,
    RHIPixelFormat_ASTC8x5HDR,
    RHIPixelFormat_ASTC8x6HDR,
    RHIPixelFormat_ASTC8x8HDR,
    RHIPixelFormat_ASTC10x5HDR,
    RHIPixelFormat_ASTC10x6HDR,
    RHIPixelFormat_ASTC10x8HDR,
    RHIPixelFormat_ASTC10x10HDR,
    RHIPixelFormat_ASTC12x10HDR,
    RHIPixelFormat_ASTC12x12HDR,

    // MultiAspect format
    //RHIPixelFormat_R8BG8Biplanar420Unorm,
    //RHIPixelFormat_R10X6BG10X6Biplanar420Unorm,

    _RHIPixelFormat_Count,
    _RHIPixelFormat_Force32 = 0x7FFFFFFF
} RHIPixelFormat;

typedef enum RHIMemoryType {
    /// CPU no access, GPU read/write
    RHIMemoryType_Private,
    /// CPU write, GPU read
    RHIMemoryType_Upload,
    /// CPU read, GPU write
    RHIMemoryType_Readback,

    _RHIMemoryType_Count,
    _RHIMemoryType_Force32 = 0x7FFFFFFF
} RHIMemoryType;

typedef enum GPUTextureAspect {
    GPUTextureAspect_All = 0,
    GPUTextureAspect_DepthOnly = 1,
    GPUTextureAspect_StencilOnly = 2,

    _GPUTextureAspect_Count,
    _GPUTextureAspect_Force32 = 0x7FFFFFFF
} GPUTextureAspect;

typedef enum RHIBackend {
    _RHIBackend_Default = 0,
    RHIBackend_Vulkan,
    RHIBackend_D3D12,
    RHIBackend_Metal,

    _RHIBackend_Count,
    _RHIBackend_Force32 = 0x7FFFFFFF
} RHIBackend;

typedef enum RHIValidationMode {
    RHIValidationMode_Disabled = 0,
    RHIValidationMode_Enabled,
    RHIValidationMode_Verbose,
    RHIValidationMode_Gpu,

    _RHIValidationMode_Count,
    _RHIValidationMode_Force32 = 0x7FFFFFFF
} RHIValidationMode;

typedef enum RHIQueueType {
    RHIQueueType_Graphics = 0,
    RHIQueueType_Compute,
    RHIQueueType_Copy,

    _RHIQueueType_Count,
    _RHIQueueType_Force32 = 0x7FFFFFFF
} RHIQueueType;

typedef enum RHIVertexFormat {
    RHIVertexFormat_Undefined = 0,
    RHIVertexFormat_UByte,
    RHIVertexFormat_UByte2,
    RHIVertexFormat_UByte4,
    RHIVertexFormat_Byte,
    RHIVertexFormat_Byte2,
    RHIVertexFormat_Byte4,
    RHIVertexFormat_UByteNormalized,
    RHIVertexFormat_UByte2Normalized,
    RHIVertexFormat_UByte4Normalized,
    RHIVertexFormat_ByteNormalized,
    RHIVertexFormat_Byte2Normalized,
    RHIVertexFormat_Byte4Normalized,
    RHIVertexFormat_UShort,
    RHIVertexFormat_UShort2,
    RHIVertexFormat_UShort4,
    RHIVertexFormat_Short,
    RHIVertexFormat_Short2,
    RHIVertexFormat_Short4,
    RHIVertexFormat_UShortNormalized,
    RHIVertexFormat_UShort2Normalized,
    RHIVertexFormat_UShort4Normalized,
    RHIVertexFormat_ShortNormalized,
    RHIVertexFormat_Short2Normalized,
    RHIVertexFormat_Short4Normalized,
    RHIVertexFormat_Half,
    RHIVertexFormat_Half2,
    RHIVertexFormat_Half4,
    RHIVertexFormat_Float,
    RHIVertexFormat_Float2,
    RHIVertexFormat_Float3,
    RHIVertexFormat_Float4,
    RHIVertexFormat_UInt,
    RHIVertexFormat_UInt2,
    RHIVertexFormat_UInt3,
    RHIVertexFormat_UInt4,
    RHIVertexFormat_Int,
    RHIVertexFormat_Int2,
    RHIVertexFormat_Int3,
    RHIVertexFormat_Int4,
    RHIVertexFormat_Unorm10_10_10_2,
    RHIVertexFormat_Unorm8x4BGRA,

    _RHIVertexFormat_Count,
    _RHIVertexFormat_Force32 = 0x7FFFFFFF
} RHIVertexFormat;

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

typedef enum RHIIndexType {
    RHIIndexType_Uint16 = 0,
    RHIIndexType_Uint32 = 1,

    _RHIIndexType_Count,
    _RHIIndexType_Force32 = 0x7FFFFFFF
} RHIIndexType;

typedef enum RHICompareFunction {
    RHICompareFunction_Undefined = 0,
    RHICompareFunction_Never,
    RHICompareFunction_Less,
    RHICompareFunction_Equal,
    RHICompareFunction_LessEqual,
    RHICompareFunction_Greater,
    RHICompareFunction_NotEqual,
    RHICompareFunction_GreaterEqual,
    RHICompareFunction_Always,

    _RHICompareFunction_Count,
    _RHICompareFunction_Force32 = 0x7FFFFFFF
} RHICompareFunction;

typedef enum RHIBlendFactor {
    RHIBlendFactor_Zero,
    RHIBlendFactor_One,
    RHIBlendFactor_SourceColor,
    RHIBlendFactor_OneMinusSourceColor,
    RHIBlendFactor_SourceAlpha,
    RHIBlendFactor_OneMinusSourceAlpha,
    RHIBlendFactor_DestinationColor,
    RHIBlendFactor_OneMinusDestinationColor,
    RHIBlendFactor_DestinationAlpha,
    RHIBlendFactor_OneMinusDestinationAlpha,
    RHIBlendFactor_SourceAlphaSaturated,
    RHIBlendFactor_BlendColor,
    RHIBlendFactor_OneMinusBlendColor,
    RHIBlendFactor_BlendAlpha,
    RHIBlendFactor_OneMinusBlendAlpha,
    RHIBlendFactor_Source1Color,
    RHIBlendFactor_OneMinusSource1Color,
    RHIBlendFactor_Source1Alpha,
    RHIBlendFactor_OneMinusSource1Alpha,

    _RHIBlendFactor_Count,
    _RHIBlendFactor_Force32 = 0x7FFFFFFF
} RHIBlendFactor;

typedef enum RHIBlendOperation {
    RHIBlendOperation_Add,
    RHIBlendOperation_Subtract,
    RHIBlendOperation_ReverseSubtract,
    RHIBlendOperation_Min,
    RHIBlendOperation_Max,

    _RHIBlendOperation_Count,
    _RHIBlendOperation_Force32 = 0x7FFFFFFF
} RHIBlendOperation;

typedef enum RHIStencilOperation {
    RHIStencilOperation_Undefined = 0,
    RHIStencilOperation_Keep,
    RHIStencilOperation_Zero,
    RHIStencilOperation_Replace,
    RHIStencilOperation_IncrementClamp,
    RHIStencilOperation_DecrementClamp,
    RHIStencilOperation_Invert,
    RHIStencilOperation_IncrementWrap,
    RHIStencilOperation_DecrementWrap,

    _RHIStencilOperation_Count,
    _RHIStencilOperation_Force32 = 0x7FFFFFFF
} RHIStencilOperation;

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

typedef enum RHIShaderStage {
    RHIShaderStage_Undefined,
    RHIShaderStage_Vertex,
    RHIShaderStage_Fragment,
    RHIShaderStage_Compute,
    RHIShaderStage_Mesh,
    RHIShaderStage_Amplification,

    _RHIShaderStage_Count,
    _RHIShaderStage_Force32 = 0x7FFFFFFF
} RHIShaderStage;

typedef enum RHIVertexStepMode {
    RHIVertexStepMode_Vertex = 0,
    RHIVertexStepMode_Instance = 1,

    _RHIVertexStepMode_Force32 = 0x7FFFFFFF
} RHIVertexStepMode;

typedef enum RHIFillMode {
    RHIFillMode_Solid,
    RHIFillMode_Wireframe,

    _RHIFillMode_Force32 = 0x7FFFFFFF
} RHIFillMode;

typedef enum RHICullMode {
    _RHICullMode_Default = 0,
    RHICullMode_None,
    RHICullMode_Front,
    RHICullMode_Back,

    _RHICullMode_Force32 = 0x7FFFFFFF
} RHICullMode;

typedef enum RHIFrontFace {
    RHIFrontFace_CounterClockwise,
    RHIFrontFace_Clockwise,

    _RHIFrontFace_Force32 = 0x7FFFFFFF
} RHIFrontFace;

typedef enum RHIDepthClipMode {
    RHIDepthClipMode_Clip,
    RHIDepthClipMode_Clamp,

    _RHIDepthClipMode_Force32 = 0x7FFFFFFF
} RHIDepthClipMode;

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

typedef enum RHIAdapterVendor {
    /// Adapter vendor is unknown
    RHIAdapterVendor_Unknown = 0,

    /// Adapter vendor is NVIDIA
    RHIAdapterVendor_NVIDIA,

    /// Adapter vendor is AMD
    RHIAdapterVendor_AMD,

    /// Adapter vendor is Intel
    RHIAdapterVendor_Intel,

    /// Adapter vendor is ARM
    RHIAdapterVendor_ARM,

    /// Adapter vendor is Qualcomm
    RHIAdapterVendor_Qualcomm,

    /// Adapter vendor is Imagination Technologies
    RHIAdapterVendor_ImgTech,

    /// Adapter vendor is Microsoft (software rasterizer)
    RHIAdapterVendor_MSFT,

    /// Adapter vendor is Apple
    RHIAdapterVendor_Apple,

    /// Adapter vendor is Mesa (software rasterizer)
    RHIAdapterVendor_Mesa,

    /// Adapter vendor is Broadcom (Raspberry Pi)
    RHIAdapterVendor_Broadcom,

    _RHIAdapterVendor_Count,
    _RHIAdapterVendor_Force32 = 0x7FFFFFFF
} RHIAdapterVendor;

typedef enum RHIAdapterType {
    RHIAdapterType_DiscreteGpu,
    RHIAdapterType_IntegratedGpu,
    RHIAdapterType_VirtualGpu,
    RHIAdapterType_Cpu,
    RHIAdapterType_Other,

    _RHIAdapterType_Force32 = 0x7FFFFFFF
} RHIAdapterType;

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

typedef enum RHIConservativeRasterizationTier {
    RHIConservativeRasterizationTier_NotSupported = 0,
    RHIConservativeRasterizationTier_1 = 1,
    RHIConservativeRasterizationTier_2 = 2,
    RHIConservativeRasterizationTier_3 = 3,

    _RHIConservativeRasterizationTier_Force32 = 0x7FFFFFFF
} RHIConservativeRasterizationTier;

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

typedef enum RHIFeature {
    RHIFeature_TimestampQuery,
    RHIFeature_PipelineStatisticsQuery,
    RHIFeature_TextureCompressionBC,
    RHIFeature_TextureCompressionETC2,
    RHIFeature_TextureCompressionASTC,
    RHIFeature_TextureCompressionASTC_HDR,
    RHIFeature_IndirectFirstInstance,
    RHIFeature_DualSourceBlending,
    RHIFeature_ShaderFloat16,
    RHIFeature_MultiDrawIndirect,

    RHIFeature_SamplerMirrorClampToEdge,
    RHIFeature_SamplerClampToBorder,
    RHIFeature_SamplerMinMax,

    RHIFeature_Tessellation,
    RHIFeature_DepthBoundsTest,
    RHIFeature_GPUUploadHeapSupported,
    RHIFeature_CopyQueueTimestampQuery,
    RHIFeature_CacheCoherentUMA,
    RHIFeature_ShaderOutputViewportIndex,
    RHIFeature_Predication,

    _RHIFeature_Force32 = 0x7FFFFFFF
} RHIFeature;

/* Flags/Bitmask Enums */
typedef uint32_t RHIBufferUsage;
static const RHIBufferUsage GPUBufferUsage_None = 0;
static const RHIBufferUsage GPUBufferUsage_Vertex = (1 << 0);
static const RHIBufferUsage GPUBufferUsage_Index = (1 << 1);
/// Supports Constant buffer access.
static const RHIBufferUsage GPUBufferUsage_Constant = (1 << 2);
static const RHIBufferUsage GPUBufferUsage_ShaderRead = (1 << 3);
static const RHIBufferUsage GPUBufferUsage_ShaderWrite = (1 << 4);
/// Supports indirect buffer access for indirect draw/dispatch.
static const RHIBufferUsage GPUBufferUsage_Indirect = (1 << 5);
/// Supports predication access for conditional rendering.
static const RHIBufferUsage GPUBufferUsage_Predication = (1 << 6);
/// Supports ray tracing acceleration structure usage.
static const RHIBufferUsage GPUBufferUsage_RayTracing = (1 << 7);

typedef uint32_t RHITextureUsage;
static const RHITextureUsage GPUTextureUsage_None = 0;
static const RHITextureUsage GPUTextureUsage_ShaderRead = (1 << 0);
static const RHITextureUsage GPUTextureUsage_ShaderWrite = (1 << 1);
static const RHITextureUsage GPUTextureUsage_RenderTarget = (1 << 2);
static const RHITextureUsage GPUTextureUsage_Transient = (1 << 3);
static const RHITextureUsage GPUTextureUsage_ShadingRate = (1 << 4);
/// Supports shared handle usage.
static const RHITextureUsage GPUTextureUsage_Shared = (1 << 5);

typedef uint32_t RHIColorWriteMask;
static const RHIColorWriteMask GPUColorWriteMask_None = 0x0000000000000000;
static const RHIColorWriteMask GPUColorWriteMask_Red = 0x0000000000000001;
static const RHIColorWriteMask GPUColorWriteMask_Green = 0x0000000000000002;
static const RHIColorWriteMask GPUColorWriteMask_Blue = 0x0000000000000004;
static const RHIColorWriteMask GPUColorWriteMask_Alpha = 0x0000000000000008;
static const RHIColorWriteMask GPUColorWriteMask_All = 0x000000000000000F /* Red | Green | Blue | Alpha */;

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


typedef struct RHILimits {
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    uint32_t maxTextureDimensionCube;
    uint32_t maxTextureArrayLayers;
    uint32_t maxBindGroups;
    uint32_t maxConstantBufferBindingSize;
    uint32_t maxStorageBufferBindingSize;
    uint32_t minConstantBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment;
    uint32_t maxPushConstantsSize;
    uint64_t maxBufferSize;
    uint32_t maxColorAttachments;
    uint32_t maxViewports;
    float    viewportBoundsMin;
    float    viewportBoundsMax;

    uint32_t maxComputeWorkgroupStorageSize;
    uint32_t maxComputeInvocationsPerWorkgroup;
    uint32_t maxComputeWorkgroupSizeX;
    uint32_t maxComputeWorkgroupSizeY;
    uint32_t maxComputeWorkgroupSizeZ;
    uint32_t maxComputeWorkgroupsPerDimension;

    /* Highest supported shader model */
    GPUShaderModel shaderModel;

    /* ConservativeRasterization tier */
    RHIConservativeRasterizationTier conservativeRasterizationTier;

    /* VariableRateShading tier */
    GPUVariableRateShadingTier variableShadingRateTier;
    uint32_t variableShadingRateImageTileSize;
    Bool32 isAdditionalVariableShadingRatesSupported;

    /* Ray tracing */
    GPURayTracingTier rayTracingTier;
    uint32_t rayTracingShaderGroupIdentifierSize;
    uint32_t rayTracingShaderTableAlignment;
    uint32_t rayTracingShaderTableMaxStride;
    uint32_t rayTracingShaderRecursionMaxDepth;
    uint32_t rayTracingMaxGeometryCount;
    uint32_t rayTracingScratchAlignment;

    /* Mesh shader */
    GPUMeshShaderTier meshShaderTier;
} RHILimits;

typedef struct RHIAdapterInfo {
    char deviceName[RHI_MAX_ADAPTER_NAME_SIZE];
    uint16_t driverVersion[4];
    const char* driverDescription;
    RHIAdapterType adapterType;
    RHIAdapterVendor vendor;
    uint32_t vendorID;
    uint32_t deviceID;
} RHIAdapterInfo;

typedef struct RHIFactoryDesc {
    RHIBackend preferredBackend;
    RHIValidationMode validationMode;
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

/* Log */
typedef void (*RHILogCallback)(RHILogLevel level, const char* message, void* userData);
ARHI_API RHILogLevel RHIGetLogLevel(void);
ARHI_API void RHISetLogLevel(RHILogLevel level);
ARHI_API void RHISetLogCallback(RHILogCallback callback, void* userData);

/* Factory */
ARHI_API bool IsRHIBackendSupport(RHIBackend backend);
ARHI_API RHIFactory RHIFactoryCreate(ARHI_NULLABLE const RHIFactoryDesc* desc);
ARHI_API uint32_t RHIFactoryAddRef(RHIFactory factory);
ARHI_API uint32_t RHIFactoryRelease(RHIFactory factory);
ARHI_API RHIBackend RHIFactoryGetBackend(RHIFactory factory);
ARHI_API uint32_t RHIFactoryGetAdapterCount(RHIFactory factory);
ARHI_API RHIAdapter RHIFactoryGetAdapter(RHIFactory factory, uint32_t index);
ARHI_API RHIAdapter RHIFactoryGetBestAdapter(RHIFactory factory);

/* Adapter */
ARHI_API void RHIAdapterGetInfo(RHIAdapter adapter, RHIAdapterInfo* info);

/* SurfaceHandle */
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromWin32(void* hwnd);
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromAndroid(void* window);
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromMetalLayer(void* metalLayer);
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromXlib(void* display, uint64_t window);
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromWayland(void* display, void* surface);
ARHI_API void RHISurfaceSourceDestroy(RHISurfaceSource source);

/* Surface */
ARHI_API RHISurface RHISurfaceCreate(RHIFactory factory, RHISurfaceSource source);
ARHI_API uint32_t RHISurfaceAddRef(RHISurface surface);
ARHI_API uint32_t RHISurfaceRelease(RHISurface surface);

/* Other */
ARHI_API uint32_t RHIVertexFormatGetByteSize(RHIVertexFormat format);
ARHI_API uint32_t RHIVertexFormatGetComponentCount(RHIVertexFormat format);
ARHI_API RHIAdapterVendor RHIAdapterVendorFromID(uint32_t vendorId);
ARHI_API uint32_t RHIAdapterVendorToID(RHIAdapterVendor vendor);

#endif /* ARHI_H_ */
