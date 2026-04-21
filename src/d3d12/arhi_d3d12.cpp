// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ARHI_D3D12)
#include "arhi_internal.h"

#ifdef _GAMING_XBOX_SCARLETT
#   pragma warning(push)
#   pragma warning(disable: 5204 5249)
#   include <d3d12_xs.h>
#   pragma warning(pop)
#   include <d3dx12_xs.h>
#elif (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
#   pragma warning(push)
#   pragma warning(disable: 5204)
#   include <d3d12_x.h>
#   pragma warning(pop)
#else
#   include <directx/d3d12.h>
#   include <directx/d3d12video.h>
//#   include <directx/d3dx12_resource_helpers.h>
//#   include <directx/d3dx12_pipeline_state_stream.h>
#   include <directx/d3dx12_check_feature_support.h>
//#   include <dcomp.h>
#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//#   include "ThirdParty/microsoft.ui.xaml.media.dxinterop.h"
#else
#   include <windows.ui.xaml.media.dxinterop.h> // WinRT
#endif
#   define PPV_ARGS(x) IID_PPV_ARGS(&x)
#endif

#define VHR(hr) do \
{ \
  if (FAILED(hr)) { \
    arhi_log_error("[%s()] HRESULT error detected (0x%lX)", __FUNCTION__, hr); \
    ARHI_DEBUG_BREAK(); \
    ExitProcess(1); \
  } \
} while(0)

#define D3D12_LOG_ERROR(hr, message) arhi_log_error("%s, result: 0x%08X", message, hr);

#include <wrl/client.h>

#include <dxgi1_6.h>

#if defined(_DEBUG) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#include <dxgidebug.h>
// Declare debug guids to avoid linking with "dxguid.lib"
static constexpr IID DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, {0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8} };
static constexpr IID DXGI_DEBUG_DXGI = { 0x25cddaa4, 0xb1c6, 0x47e1, {0xac, 0x3e, 0x98, 0x87, 0x5b, 0x5a, 0x2e, 0x2a} };
#endif

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "D3D12MemAlloc.h"

//#define ALIMER_USE_PIX3

#if defined(ALIMER_USE_PIX3)
#include <pix3.h>
#else
// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix.h>
#endif

using Microsoft::WRL::ComPtr;

static_assert(sizeof(GPUAddress) == sizeof(D3D12_GPU_VIRTUAL_ADDRESS), "GPUAddress mismatch");

static_assert(sizeof(RHIViewport) == sizeof(D3D12_VIEWPORT), "Viewport mismatch");
static_assert(offsetof(RHIViewport, x) == offsetof(D3D12_VIEWPORT, TopLeftX), "Layout mismatch");
static_assert(offsetof(RHIViewport, y) == offsetof(D3D12_VIEWPORT, TopLeftY), "Layout mismatch");
static_assert(offsetof(RHIViewport, width) == offsetof(D3D12_VIEWPORT, Width), "Layout mismatch");
static_assert(offsetof(RHIViewport, height) == offsetof(D3D12_VIEWPORT, Height), "Layout mismatch");
static_assert(offsetof(RHIViewport, minDepth) == offsetof(D3D12_VIEWPORT, MinDepth), "Layout mismatch");
static_assert(offsetof(RHIViewport, maxDepth) == offsetof(D3D12_VIEWPORT, MaxDepth), "Layout mismatch");

static_assert(sizeof(RHIDispatchIndirectCommand) == sizeof(D3D12_DISPATCH_ARGUMENTS), "DispatchIndirectCommand mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountX) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountX), "Layout mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountY) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountY), "Layout mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountZ) == offsetof(D3D12_DISPATCH_ARGUMENTS, ThreadGroupCountZ), "Layout mismatch");

static_assert(sizeof(RHIDrawIndexedIndirectCommand) == sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), "DrawIndexedIndirectCommand mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, indexCount) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, IndexCountPerInstance), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, instanceCount) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, InstanceCount), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, firstIndex) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, StartIndexLocation), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, baseVertex) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, BaseVertexLocation), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, firstInstance) == offsetof(D3D12_DRAW_INDEXED_ARGUMENTS, StartInstanceLocation), "DrawIndexedIndirectCommand layout mismatch");

static_assert(sizeof(RHIDrawIndirectCommand) == sizeof(D3D12_DRAW_ARGUMENTS), "DrawIndirectCommand mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, vertexCount) == offsetof(D3D12_DRAW_ARGUMENTS, VertexCountPerInstance), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, instanceCount) == offsetof(D3D12_DRAW_ARGUMENTS, InstanceCount), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, firstVertex) == offsetof(D3D12_DRAW_ARGUMENTS, StartVertexLocation), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, firstInstance) == offsetof(D3D12_DRAW_ARGUMENTS, StartInstanceLocation), "DrawIndirectCommand layout mismatch");

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
using PFN_CREATE_DXGI_FACTORY2 = decltype(&CreateDXGIFactory2);
using PFN_DXGI_GET_DEBUG_INTERFACE1 = decltype(&DXGIGetDebugInterface1);

const GUID CLSID_D3D12SDKConfiguration_Alimer = { 0x7cda6aca, 0xa03e, 0x49c8, {0x94, 0x58, 0x03, 0x34, 0xd2, 0x0e, 0x07, 0xce} };
const GUID CLSID_D3D12DeviceFactory_Alimer = { 0x114863bf, 0xc386, 0x4aee, {0xb3, 0x9d, 0x8f, 0x0b, 0xbb, 0x06, 0x29, 0x55} };
const GUID CLSID_D3D12Debug_Alimer = { 0xf2352aeb, 0xdd84, 0x49fe, {0xb9, 0x7b, 0xa9, 0xdc, 0xfd, 0xcc, 0x1b, 0x4f} };
const GUID CLSID_D3D12DeviceRemovedExtendedData_Alimer = { 0x4a75bbc4, 0x9ff4, 0x4ad8, {0x9f, 0x18, 0xab, 0xae, 0x84, 0xdc, 0x5f, 0xf2} };

/* pix3 */
void WINAPI PIXBeginEventOnCommandListFn(ID3D12GraphicsCommandList * commandList, UINT64 color, _In_ PCSTR formatString);
void WINAPI PIXEndEventOnCommandListFn(ID3D12GraphicsCommandList * commandList);
void WINAPI PIXSetMarkerOnCommandListFn(ID3D12GraphicsCommandList * commandList, UINT64 color, _In_ PCSTR formatString);
//void WINAPI PIXBeginEventOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);
//void WINAPI PIXEndEventOnCommandQueue(ID3D12CommandQueue* commandQueue);
//void WINAPI PIXSetMarkerOnCommandQueue(ID3D12CommandQueue* commandQueue, UINT64 color, _In_ PCSTR formatString);
using PFN_PIXBeginEventOnCommandList = decltype(&PIXBeginEventOnCommandListFn);
using PFN_PIXEndEventOnCommandList = decltype(&PIXEndEventOnCommandListFn);
using PFN_PIXSetMarkerOnCommandList = decltype(&PIXSetMarkerOnCommandListFn);

struct D3D12_State {
    HMODULE lib_dxgi = nullptr;
    HMODULE lib_d3d12 = nullptr;
    HMODULE lib_WinPixEventRuntime = nullptr;

    PFN_CREATE_DXGI_FACTORY2 CreateDXGIFactory2 = nullptr;

#if defined(_DEBUG)
    PFN_DXGI_GET_DEBUG_INTERFACE1 DXGIGetDebugInterface1 = nullptr;
#endif

    PFN_D3D12_CREATE_DEVICE D3D12CreateDevice = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE D3D12GetDebugInterface = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE D3D12SerializeVersionedRootSignature = nullptr;

    PFN_PIXBeginEventOnCommandList PIXBeginEventOnCommandList = nullptr;
    PFN_PIXEndEventOnCommandList PIXEndEventOnCommandList = nullptr;
    PFN_PIXSetMarkerOnCommandList PIXSetMarkerOnCommandList = nullptr;

    ComPtr<ID3D12DeviceFactory> deviceFactory;

    ~D3D12_State()
    {
        deviceFactory.Reset();

        if (lib_d3d12)
        {
            FreeLibrary(lib_d3d12);
            lib_d3d12 = nullptr;
        }

        if (lib_dxgi)
        {
            FreeLibrary(lib_dxgi);
            lib_dxgi = nullptr;
        }

        if (lib_WinPixEventRuntime)
        {
            FreeLibrary(lib_WinPixEventRuntime);
            lib_WinPixEventRuntime = nullptr;
        }
    }
} d3d12_state;

#define dxgi_CreateDXGIFactory2 d3d12_state.CreateDXGIFactory2

#if defined(_DEBUG)
#define dxgi_DXGIGetDebugInterface1 d3d12_state.DXGIGetDebugInterface1
#endif /* defined(_DEBUG) */

#define d3d12_D3D12CreateDevice d3d12_state.D3D12CreateDevice
#define d3d12_D3D12GetDebugInterface d3d12_state.D3D12GetDebugInterface
#define d3d12_D3D12SerializeVersionedRootSignature d3d12_state.D3D12SerializeVersionedRootSignature
#else
#define dxgi_CreateDXGIFactory2 CreateDXGIFactory2
#define d3d12_D3D12CreateDevice D3D12CreateDevice
#define d3d12_D3D12GetDebugInterface D3D12GetDebugInterface
#define d3d12_D3D12SerializeVersionedRootSignature D3D12SerializeVersionedRootSignature
#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */

bool D3D12_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;
    // Linux:  const char* libName = "libdxvk_dxgi.so";

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
    d3d12_state.lib_dxgi = LoadLibraryExW(L"dxgi.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    d3d12_state.lib_d3d12 = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

    if (d3d12_state.lib_dxgi == nullptr || d3d12_state.lib_d3d12 == nullptr)
    {
        return false;
    }

    d3d12_state.CreateDXGIFactory2 = (PFN_CREATE_DXGI_FACTORY2)GetProcAddress(d3d12_state.lib_dxgi, "CreateDXGIFactory2");
    if (d3d12_state.CreateDXGIFactory2 == nullptr)
    {
        return false;
    }

#if defined(_DEBUG)
    d3d12_state.DXGIGetDebugInterface1 = (PFN_DXGI_GET_DEBUG_INTERFACE1)GetProcAddress(d3d12_state.lib_dxgi, "DXGIGetDebugInterface1");
#endif

    // Use new D3D12GetInterface and agility SDK
    static PFN_D3D12_GET_INTERFACE func_D3D12GetInterface = (PFN_D3D12_GET_INTERFACE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12GetInterface");
    if (func_D3D12GetInterface)
    {
        ComPtr<ID3D12SDKConfiguration> sdkConfig;
        if (SUCCEEDED(func_D3D12GetInterface(CLSID_D3D12SDKConfiguration_Alimer, IID_PPV_ARGS(sdkConfig.GetAddressOf()))))
        {
            ComPtr<ID3D12SDKConfiguration1> sdkConfig1 = nullptr;
            if (SUCCEEDED(sdkConfig.As(&sdkConfig1)))
            {
                uint32_t agilitySdkVersion = D3D12_SDK_VERSION;
                const char* agilitySdkPath = ".\\D3D12\\"; // D3D12SDKPath;
                if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, agilitySdkPath, IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()))))
                {
                    func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()));
                }
                else if (SUCCEEDED(sdkConfig1->CreateDeviceFactory(agilitySdkVersion, ".\\", IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()))))
                {
                    func_D3D12GetInterface(CLSID_D3D12DeviceFactory_Alimer, IID_PPV_ARGS(d3d12_state.deviceFactory.GetAddressOf()));
                }
            }
        }
    }

    if (!d3d12_state.deviceFactory)
    {
        d3d12_state.D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12CreateDevice");
        if (!d3d12_state.D3D12CreateDevice)
        {
            return false;
        }

        d3d12_state.D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12GetDebugInterface");
        d3d12_state.D3D12SerializeVersionedRootSignature = (PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE)GetProcAddress(d3d12_state.lib_d3d12, "D3D12SerializeVersionedRootSignature");
        if (!d3d12_state.D3D12SerializeVersionedRootSignature) {
            return false;
        }
    }

    // Try to load PIX (WinPixEventRuntime.dll)
    d3d12_state.lib_WinPixEventRuntime = LoadLibraryW(L"WinPixEventRuntime.dll");
    if (d3d12_state.lib_WinPixEventRuntime != nullptr)
    {
        d3d12_state.PIXBeginEventOnCommandList = (PFN_PIXBeginEventOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXBeginEventOnCommandList");
        d3d12_state.PIXEndEventOnCommandList = (PFN_PIXEndEventOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXEndEventOnCommandList");
        d3d12_state.PIXSetMarkerOnCommandList = (PFN_PIXSetMarkerOnCommandList)GetProcAddress(d3d12_state.lib_WinPixEventRuntime, "PIXSetMarkerOnCommandList");
    }
#endif

    ComPtr<IDXGIFactory4> dxgiFactory;
    if (FAILED(dxgi_CreateDXGIFactory2(0, PPV_ARGS(dxgiFactory))))
    {
        return false;
    }

    ComPtr<IDXGIAdapter1> dxgiAdapter;
    bool foundCompatibleDevice = false;
    for (uint32_t i = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, dxgiAdapter.ReleaseAndGetAddressOf()); ++i)
    {
        DXGI_ADAPTER_DESC1 adapterDesc;
        VHR(dxgiAdapter->GetDesc1(&adapterDesc));

        // Don't select the Basic Render Driver adapter.
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the actual device.
        if (d3d12_state.deviceFactory != nullptr)
        {
            if (SUCCEEDED(d3d12_state.deviceFactory->CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
            {
                foundCompatibleDevice = true;
                break;
            }
        }
        else
        {
            if (SUCCEEDED(d3d12_D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
            {
                foundCompatibleDevice = true;
                break;
            }
        }
    }

    if (foundCompatibleDevice)
    {
        available = true;
        return true;
    }

    available = false;
    return false;
}

RHIFactoryImpl* D3D12_CreateFactory(const RHIFactoryDesc* desc)
{
    ARHI_UNUSED(desc);
    return nullptr;
}

#endif /* defined(ARHI_D3D12) */
