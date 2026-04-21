// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#include "arhi_internal.h"
#include <stdarg.h>
#include <stdio.h>

/* Log */
static RHILogLevel s_LogLevel = RHILogLevel_Off;
static RHILogCallback s_logCallback = nullptr;
static void* s_logUserData = nullptr;

RHILogLevel RHIGetLogLevel(void)
{
    return s_LogLevel;
}

void RHISetLogLevel(RHILogLevel level)
{
    s_LogLevel = level;
}

void RHISetLogCallback(RHILogCallback callback, void* userData)
{
    s_logCallback = callback;
    s_logUserData = userData;
}

static bool ShouldLog(RHILogLevel level)
{
    if (!s_logCallback || s_LogLevel == RHILogLevel_Off)
        return false;

    return level >= s_LogLevel;
}

void arhi_log(RHILogLevel level, const char* message)
{
    if (!ShouldLog(level))
        return;

    s_logCallback(level, message, s_logUserData);
}

void arhi_log_format(RHILogLevel level, const char* format, ...)
{
    if (!ShouldLog(level))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    s_logCallback(level, message, s_logUserData);
}

void arhi_log_info(const char* format, ...)
{
    if (!ShouldLog(RHILogLevel_Info))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    s_logCallback(RHILogLevel_Info, message, s_logUserData);
}

void arhi_log_warn(const char* format, ...)
{
    if (!ShouldLog(RHILogLevel_Warn))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    s_logCallback(RHILogLevel_Warn, message, s_logUserData);
}

void arhi_log_error(const char* format, ...)
{
    if (!ShouldLog(RHILogLevel_Error))
        return;

    char message[MAX_LOG_MESSAGE_SIZE];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    s_logCallback(RHILogLevel_Error, message, s_logUserData);
}

/* Factory */
bool IsRHIBackendSupport(RHIBackend backend)
{
    switch (backend)
    {
        case RHIBackend_Vulkan:
#if defined(ARHI_VULKAN)
            return Vulkan_IsSupported();
#else
            return false;
#endif

        case RHIBackend_D3D12:
#if defined(ARHI_D3D12)
            return D3D12_IsSupported();
#else
            return false;
#endif

        case RHIBackend_Metal:
#if defined(ARHI_Metal)
            return Metal_CreateFactory();
#else
            return false;
#endif

        default:
            return false;
    }
}

RHIFactory RHIFactoryCreate(ARHI_NULLABLE const RHIFactoryDesc* desc)
{
    RHIBackend backend = (desc != nullptr ? desc->preferredBackend : _RHIBackend_Default);
    if (backend == _RHIBackend_Default)
    {
        if (IsRHIBackendSupport(RHIBackend_D3D12))
        {
            backend = RHIBackend_D3D12;
        }
        else if (IsRHIBackendSupport(RHIBackend_Metal))
        {
            backend = RHIBackend_Metal;
        }
        else if (IsRHIBackendSupport(RHIBackend_Vulkan))
        {
            backend = RHIBackend_Vulkan;
        }
    }

    RHIFactory factory = nullptr;
    switch (backend)
    {
        case RHIBackend_Vulkan:
#if defined(ARHI_VULKAN)
            if (Vulkan_IsSupported())
            {
                factory = Vulkan_CreateFactory(desc);
            }
            break;
#else
            arhi_log_error("Vulkan is not supported");
            return nullptr;
#endif

        case RHIBackend_D3D12:
#if defined(ARHI_D3D12)
            if (D3D12_IsSupported())
            {
                factory = D3D12_CreateFactory(desc);
            }
            break;
#else
            arhi_log_error("D3D12 is not supported");
            return nullptr;
#endif
            break;

        case RHIBackend_Metal:
#if defined(ARHI_METAL)
            if (Metal_IsSupported())
            {
                factory = Metal_CreateFactory(desc);
            }
            break;
#else
            arhi_log_error("Metal is not supported");
            return nullptr;
#endif
            break;

        default:
            break;
    }

    return factory;
}

uint32_t RHIFactoryAddRef(RHIFactory factory)
{
    return factory->AddRef();
}
uint32_t RHIFactoryRelease(RHIFactory factory)
{
    return factory->Release();
}

RHIBackend RHIFactoryGetBackend(RHIFactory factory)
{
    return factory->GetBackend();
}

uint32_t RHIFactoryGetAdapterCount(RHIFactory factory)
{
    return factory->GetAdapterCount();
}

RHIAdapter RHIFactoryGetAdapter(RHIFactory factory, uint32_t index)
{
    return factory->GetAdapter(index);
}

RHIAdapter RHIFactoryGetBestAdapter(RHIFactory factory)
{
    RHIAdapter result = nullptr;
    uint32_t kind = (uint32_t)RHIAdapterType_Other + 1;

    for (uint32_t i = 0, count = factory->GetAdapterCount(); i < count; ++i)
    {
        RHIAdapter adapter = factory->GetAdapter(i);
        RHIAdapterType adapterType = adapter->GetType();
        if ((uint32_t)adapterType < kind)
        {
            result = adapter;
            kind = (uint32_t)adapterType;
        }
    }

    return result;
}

/* Adapter */
void RHIAdapterGetInfo(RHIAdapter adapter, RHIAdapterInfo* info)
{
    ARHI_ASSERT(info);

    adapter->GetInfo(info);
}

/* SurfaceHandle */
ARHI_API RHISurfaceSource RHISurfaceSourceCreateFromWin32(void* hwnd)
{
#if defined(_WIN32)
    if (!IsWindow(static_cast<HWND>(hwnd)))
    {
        arhi_log_error("Win32: Invalid vulkan hwnd handle");
        return nullptr;
    }

    RHISurfaceSource handle = new RHISurfaceSourceImpl();
    handle->type = RHISurfaceSourceType::WindowsHWND;
    handle->hwnd = static_cast<HWND>(hwnd);
    return handle;
#else
    ARHI_UNUSED(hwnd);
    arhi_log_error("Win32 surface source is not supported on this platform");
    return nullptr;
#endif
}

RHISurfaceSource RHISurfaceSourceCreateFromAndroid(void* window)
{
    RHISurfaceSource handle = new RHISurfaceSourceImpl();
    handle->type = RHISurfaceSourceType::AndroidWindow;
    handle->androidWindow = static_cast<ANativeWindow*>(window);
    return handle;
}

RHISurfaceSource RHISurfaceSourceCreateFromMetalLayer(void* metalLayer)
{
    RHISurfaceSource handle = new RHISurfaceSourceImpl();
    handle->type = RHISurfaceSourceType::MetalLayer;
    handle->metalLayer = metalLayer;
    return handle;
}

RHISurfaceSource RHISurfaceSourceCreateFromXlib(void* display, uint64_t window)
{
    RHISurfaceSource handle = new RHISurfaceSourceImpl();
    handle->type = RHISurfaceSourceType::XlibWindow;
    handle->xDisplay = display;
    handle->xWindow = window;
    return handle;
}

RHISurfaceSource RHISurfaceSourceCreateFromWayland(void* display, void* surface)
{
    RHISurfaceSource handle = new RHISurfaceSourceImpl();
    handle->type = RHISurfaceSourceType::WaylandSurface;
    handle->waylandDisplay = static_cast<wl_display*>(display);
    handle->waylandSurface = static_cast<wl_surface*>(surface);
    return handle;
}

void RHISurfaceSourceDestroy(RHISurfaceSource source)
{
    delete source;
}

/* Surface */
RHISurface RHISurfaceCreate(RHIFactory factory, RHISurfaceSource source)
{
    ARHI_ASSERT(factory);
    ARHI_ASSERT(source);

    return factory->CreateSurface(source);
}

uint32_t RHISurfaceAddRef(RHISurface surface)
{
    return surface->AddRef();
}

uint32_t RHISurfaceRelease(RHISurface surface)
{
    return surface->Release();
}

/* Other */
struct RHIVertexFormatInfo
{
    RHIVertexFormat format;
    uint32_t byteSize;
    uint32_t componentCount;
};

static const RHIVertexFormatInfo kVertexFormatTable[] = {
    { RHIVertexFormat_Undefined,           0, 0 },
    { RHIVertexFormat_UByte,               1, 1 },
    { RHIVertexFormat_UByte2,              2, 2 },
    { RHIVertexFormat_UByte4,              4, 4 },
    { RHIVertexFormat_Byte,                1, 1 },
    { RHIVertexFormat_Byte2,               2, 2 },
    { RHIVertexFormat_Byte4,               4, 4 },
    { RHIVertexFormat_UByteNormalized,     1, 1 },
    { RHIVertexFormat_UByte2Normalized,    2, 2 },
    { RHIVertexFormat_UByte4Normalized,    4, 4 },
    { RHIVertexFormat_ByteNormalized,      2, 2 },
    { RHIVertexFormat_Byte2Normalized,     2, 2 },
    { RHIVertexFormat_Byte4Normalized,     4, 4 },

    { RHIVertexFormat_UShort,              2, 1 },
    { RHIVertexFormat_UShort2,             4, 2 },
    { RHIVertexFormat_UShort4,             8, 4 },
    { RHIVertexFormat_Short,               4, 1 },
    { RHIVertexFormat_Short2,              4, 2 },
    { RHIVertexFormat_Short4,              8, 4 },
    { RHIVertexFormat_UShortNormalized,    2, 1 },
    { RHIVertexFormat_UShort2Normalized,   4, 2 },
    { RHIVertexFormat_UShort4Normalized,   8, 4 },
    { RHIVertexFormat_ShortNormalized,     2, 1 },
    { RHIVertexFormat_Short2Normalized,    4, 2 },
    { RHIVertexFormat_Short4Normalized,    8, 4 },

    { RHIVertexFormat_Half,                2, 1 },
    { RHIVertexFormat_Half2,               4, 2 },
    { RHIVertexFormat_Half4,               8, 4 },
    { RHIVertexFormat_Float,               4, 1 },
    { RHIVertexFormat_Float2,              8, 2 },
    { RHIVertexFormat_Float3,              12, 3 },
    { RHIVertexFormat_Float4,              16, 4 },

    { RHIVertexFormat_UInt,                4, 1 },
    { RHIVertexFormat_UInt2,               8, 2 },
    { RHIVertexFormat_UInt3,               12, 3 },
    { RHIVertexFormat_UInt4,               16, 4 },

    { RHIVertexFormat_Int,                 4, 1 },
    { RHIVertexFormat_Int2,                8, 2 },
    { RHIVertexFormat_Int3,                12, 3 },
    { RHIVertexFormat_Int4,                16, 4 },

    { RHIVertexFormat_Unorm10_10_10_2, 4, 4 },
    { RHIVertexFormat_Unorm8x4BGRA, 4, 4 }
    //{VertexFormat::RG11B10Float,   32, 4,  VertexFormatKind::Float},
    //{VertexFormat::RGB9E5Float,   32, 4, VertexFormatKind::Float},
};

static_assert(
    sizeof(kVertexFormatTable) / sizeof(RHIVertexFormatInfo) == size_t(_RHIVertexFormat_Count),
    "The format info table doesn't have the right number of elements"
    );

enum class KnownGPUAdapterVendor
{
    AMD = 0x01002,
    NVIDIA = 0x010DE,
    INTEL = 0x08086,
    ARM = 0x013B5,
    QUALCOMM = 0x05143,
    IMGTECH = 0x01010,
    MSFT = 0x01414,
    APPLE = 0x0106B,
    MESA = 0x10005,
    BROADCOM = 0x014e4
};

static const RHIVertexFormatInfo& GetVertexFormatInfo(RHIVertexFormat format)
{
    if (format >= _RHIVertexFormat_Count)
        return kVertexFormatTable[0]; // Undefined

    const RHIVertexFormatInfo& info = kVertexFormatTable[format];
    ARHI_ASSERT(info.format == format);
    return info;
}

uint32_t RHIVertexFormatGetByteSize(RHIVertexFormat format)
{
    const RHIVertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.byteSize;
}

uint32_t RHIVertexFormatGetComponentCount(RHIVertexFormat format)
{
    const RHIVertexFormatInfo& formatInfo = GetVertexFormatInfo(format);
    return formatInfo.componentCount;
}

RHIAdapterVendor RHIAdapterVendorFromID(uint32_t vendorId)
{
    switch (vendorId)
    {
        case (uint32_t)KnownGPUAdapterVendor::AMD:
            return RHIAdapterVendor_AMD;
        case (uint32_t)KnownGPUAdapterVendor::NVIDIA:
            return RHIAdapterVendor_NVIDIA;
        case (uint32_t)KnownGPUAdapterVendor::INTEL:
            return RHIAdapterVendor_Intel;
        case (uint32_t)KnownGPUAdapterVendor::ARM:
            return RHIAdapterVendor_ARM;
        case (uint32_t)KnownGPUAdapterVendor::QUALCOMM:
            return RHIAdapterVendor_Qualcomm;
        case (uint32_t)KnownGPUAdapterVendor::IMGTECH:
            return RHIAdapterVendor_ImgTech;
        case (uint32_t)KnownGPUAdapterVendor::MSFT:
            return RHIAdapterVendor_MSFT;
        case (uint32_t)KnownGPUAdapterVendor::APPLE:
            return RHIAdapterVendor_Apple;
        case (uint32_t)KnownGPUAdapterVendor::MESA:
            return RHIAdapterVendor_Mesa;
        case (uint32_t)KnownGPUAdapterVendor::BROADCOM:
            return RHIAdapterVendor_Broadcom;

        default:
            return RHIAdapterVendor_Unknown;
    }
}

uint32_t RHIAdapterVendorToID(RHIAdapterVendor vendor)
{
    switch (vendor)
    {
        case RHIAdapterVendor_AMD:
            return (uint32_t)KnownGPUAdapterVendor::AMD;
        case RHIAdapterVendor_NVIDIA:
            return (uint32_t)KnownGPUAdapterVendor::NVIDIA;
        case RHIAdapterVendor_Intel:
            return (uint32_t)KnownGPUAdapterVendor::INTEL;
        case RHIAdapterVendor_ARM:
            return (uint32_t)KnownGPUAdapterVendor::ARM;
        case RHIAdapterVendor_Qualcomm:
            return (uint32_t)KnownGPUAdapterVendor::QUALCOMM;
        case RHIAdapterVendor_ImgTech:
            return (uint32_t)KnownGPUAdapterVendor::IMGTECH;
        case RHIAdapterVendor_MSFT:
            return (uint32_t)KnownGPUAdapterVendor::MSFT;
        case RHIAdapterVendor_Apple:
            return (uint32_t)KnownGPUAdapterVendor::APPLE;
        case RHIAdapterVendor_Mesa:
            return (uint32_t)KnownGPUAdapterVendor::MESA;
        case RHIAdapterVendor_Broadcom:
            return (uint32_t)KnownGPUAdapterVendor::BROADCOM;

        default:
            return 0;
    }
}
