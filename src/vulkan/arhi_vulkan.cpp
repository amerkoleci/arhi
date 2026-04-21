// Copyright (c) Amer Koleci and Contributors.
// Licensed under the MIT License (MIT). See LICENSE in the repository root for more information.

#if defined(ARHI_VULKAN)
#include "arhi_internal.h"
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#if defined(_WIN32)
#include <vulkan/vulkan_win32.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#elif defined(__APPLE__)
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#else
typedef struct xcb_connection_t xcb_connection_t;
typedef uint32_t xcb_window_t;
typedef uint32_t xcb_visualid_t;

//#include <vulkan/vulkan_xlib.h>
#include <vulkan/vulkan_xcb.h>

struct wl_display;
struct wl_surface;
#include <vulkan/vulkan_wayland.h>
#endif

ARHI_DISABLE_WARNINGS()
#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"
//#include "spirv_reflect.h"
ARHI_ENABLE_WARNINGS()

#if !defined(_WIN32)
#include <dlfcn.h>
#endif

#include <inttypes.h>
#include <algorithm>
#include <vector>
#include <deque>
#include <mutex>

static_assert(sizeof(GPUAddress) == sizeof(VkDeviceAddress), "GPUAddress mismatch");

static_assert(sizeof(RHIViewport) == sizeof(VkViewport), "RHIViewport mismatch");
static_assert(offsetof(RHIViewport, x) == offsetof(VkViewport, x), "RHIViewport layout mismatch");
static_assert(offsetof(RHIViewport, y) == offsetof(VkViewport, y), "RHIViewport layout mismatch");
static_assert(offsetof(RHIViewport, width) == offsetof(VkViewport, width), "RHIViewport layout mismatch");
static_assert(offsetof(RHIViewport, height) == offsetof(VkViewport, height), "RHIViewport layout mismatch");
static_assert(offsetof(RHIViewport, minDepth) == offsetof(VkViewport, minDepth), "RHIViewport layout mismatch");
static_assert(offsetof(RHIViewport, maxDepth) == offsetof(VkViewport, maxDepth), "RHIViewport layout mismatch");

static_assert(sizeof(RHIScissorRect) == sizeof(VkRect2D), "RHIScissorRect mismatch");
static_assert(offsetof(RHIScissorRect, x) == offsetof(VkRect2D, offset.x), "RHIScissorRect layout mismatch");
static_assert(offsetof(RHIScissorRect, y) == offsetof(VkRect2D, offset.y), "RHIScissorRect layout mismatch");
static_assert(offsetof(RHIScissorRect, width) == offsetof(VkRect2D, extent.width), "RHIScissorRect layout mismatch");
static_assert(offsetof(RHIScissorRect, height) == offsetof(VkRect2D, extent.height), "RHIScissorRect layout mismatch");

static_assert(sizeof(RHIDispatchIndirectCommand) == sizeof(VkDispatchIndirectCommand), "DispatchIndirectCommand mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountX) == offsetof(VkDispatchIndirectCommand, x), "DispatchIndirectCommand layout mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountY) == offsetof(VkDispatchIndirectCommand, y), "DispatchIndirectCommand layout mismatch");
static_assert(offsetof(RHIDispatchIndirectCommand, groupCountZ) == offsetof(VkDispatchIndirectCommand, z), "DispatchIndirectCommand layout mismatch");

static_assert(sizeof(RHIDrawIndexedIndirectCommand) == sizeof(VkDrawIndexedIndirectCommand), "DrawIndexedIndirectCommand mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, indexCount) == offsetof(VkDrawIndexedIndirectCommand, indexCount), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, instanceCount) == offsetof(VkDrawIndexedIndirectCommand, instanceCount), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, firstIndex) == offsetof(VkDrawIndexedIndirectCommand, firstIndex), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, baseVertex) == offsetof(VkDrawIndexedIndirectCommand, vertexOffset), "DrawIndexedIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndexedIndirectCommand, firstInstance) == offsetof(VkDrawIndexedIndirectCommand, firstInstance), "DrawIndexedIndirectCommand layout mismatch");

static_assert(sizeof(RHIDrawIndirectCommand) == sizeof(VkDrawIndirectCommand), "DrawIndirectCommand mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, vertexCount) == offsetof(VkDrawIndirectCommand, vertexCount), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, instanceCount) == offsetof(VkDrawIndirectCommand, instanceCount), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, firstVertex) == offsetof(VkDrawIndirectCommand, firstVertex), "DrawIndirectCommand layout mismatch");
static_assert(offsetof(RHIDrawIndirectCommand, firstInstance) == offsetof(VkDrawIndirectCommand, firstInstance), "DrawIndirectCommand layout mismatch");

inline const char* VkResultToString(VkResult result)
{
    switch (result)
    {
#define STR(r)   \
	case VK_##r: \
		return #r
        STR(NOT_READY);
        STR(TIMEOUT);
        STR(EVENT_SET);
        STR(EVENT_RESET);
        STR(INCOMPLETE);
        STR(ERROR_OUT_OF_HOST_MEMORY);
        STR(ERROR_OUT_OF_DEVICE_MEMORY);
        STR(ERROR_INITIALIZATION_FAILED);
        STR(ERROR_DEVICE_LOST);
        STR(ERROR_MEMORY_MAP_FAILED);
        STR(ERROR_LAYER_NOT_PRESENT);
        STR(ERROR_EXTENSION_NOT_PRESENT);
        STR(ERROR_FEATURE_NOT_PRESENT);
        STR(ERROR_INCOMPATIBLE_DRIVER);
        STR(ERROR_TOO_MANY_OBJECTS);
        STR(ERROR_FORMAT_NOT_SUPPORTED);
        STR(ERROR_SURFACE_LOST_KHR);
        STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
        STR(SUBOPTIMAL_KHR);
        STR(ERROR_OUT_OF_DATE_KHR);
        STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
        STR(ERROR_VALIDATION_FAILED_EXT);
        STR(ERROR_INVALID_SHADER_NV);
#undef STR
        default:
            return "UNKNOWN_ERROR";
    }
}

#if defined(_DEBUG)
/// Helper macro to test the result of Vulkan calls which can return an error.
#define VK_CHECK(x) \
	do \
	{ \
		VkResult err = x; \
		if (err < 0) \
		{ \
			arhi_log_error("Detected Vulkan error: %s", VkResultToString(err)); \
		} \
	} while (0)
#else
#define VK_CHECK(x) (void)(x)
#endif
#define VK_LOG_ERROR(result, message) arhi_log_error("Vulkan: %s, error: %s", message, VkResultToString(result));

// Declare function pointers
static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;

#define VULKAN_GLOBAL_FUNCTION(name) static PFN_##name name = nullptr;
#include "arhi_vulkan_funcs.h"

namespace
{
    template<typename MainT, typename NewT>
    inline void PnextChainPushFront(MainT* mainStruct, NewT* newStruct)
    {
        newStruct->pNext = mainStruct->pNext;
        mainStruct->pNext = newStruct;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        ARHI_UNUSED(pUserData);

        const char* messageTypeStr = "General";

        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            messageTypeStr = "Validation";
        else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            messageTypeStr = "Performance";

        // Log debug messge
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            arhi_log_warn("Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            arhi_log_error("Vulkan - %s: %s", messageTypeStr, pCallbackData->pMessage);
#if defined(_DEBUG)
            ARHI_DEBUG_BREAK();
#endif
        }

        return VK_FALSE;
    }

    bool ValidateLayers(const std::vector<const char*>& required, const std::vector<VkLayerProperties>& available)
    {
        for (auto layer : required)
        {
            bool found = false;
            for (auto& available_layer : available)
            {
                if (strcmp(available_layer.layerName, layer) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                arhi_log_warn("Validation Layer '%s' not found", layer);
                return false;
            }
        }

        return true;
    }
}

/* Forward declarations */
struct VulkanRHIAdapter;
struct VulkanRHIFactory;


struct VulkanPhysicalDeviceExtensions final
{
    // Core 1.3
    bool maintenance4;
    bool dynamicRendering;
    bool synchronization2;
    bool extendedDynamicState;
    bool extendedDynamicState2;
    bool pipelineCreationCacheControl;
    bool formatFeatureFlags2;

    // Core 1.4
    bool pushDescriptor;

    // Extensions
    bool swapchain;
    bool memoryBudget;
    bool AMD_device_coherent_memory;
    bool EXT_memory_priority;
    bool deferredHostOperations;
    bool portabilitySubset;
    bool depthClipEnable;
    bool textureCompressionAstcHdr;
    bool shaderViewportIndexLayer;
    bool conservativeRasterization;

    bool externalMemory;
    bool externalSemaphore;
    bool externalFence;

    bool maintenance5;
    bool maintenance6;
    bool accelerationStructure;
    bool raytracingPipeline;
    bool rayQuery;
    bool fragmentShadingRate;
    bool meshShader;
    bool conditionalRendering;
    struct
    {
        bool queue;
        bool decode_queue;
        bool decode_h264;
        bool decode_h265;
        bool encode_queue;
        bool encode_h264;
        bool encode_h265;
    } video;
    bool win32_full_screen_exclusive;
};

struct VulkanQueueFamilyIndices final
{
    uint32_t queueFamilyCount = 0;
    uint32_t familyIndices[_RHIQueueType_Count] = {};
    uint32_t queueIndices[_RHIQueueType_Count] = {};
    uint32_t counts[_RHIQueueType_Count] = {};

    uint32_t timestampValidBits = 0;

    std::vector<uint32_t> queueOffsets;
    std::vector<std::vector<float>> queuePriorities;

    VulkanQueueFamilyIndices()
    {
        for (auto& index : familyIndices)
        {
            index = VK_QUEUE_FAMILY_IGNORED;
        }
    }

    bool IsComplete() const
    {
        return familyIndices[RHIQueueType_Graphics] != VK_QUEUE_FAMILY_IGNORED;
    }
};

struct VulkanRHISurface final : public RHISurfaceImpl
{
    VulkanRHIFactory* factory = nullptr;
    //VulkanDevice* device = nullptr;
    VkSurfaceKHR handle = VK_NULL_HANDLE;
    //VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    //VkExtent2D swapchainExtent = {};
    //uint32_t backBufferIndex = 0;
    //std::vector<VulkanTexture*> backbufferTextures;
    //std::mutex locker;
    //size_t swapChainAcquireSemaphoreIndex = 0;
    //std::vector<VkSemaphore> swapchainAcquireSemaphores;
    //std::vector<VkSemaphore> swapchainReleaseSemaphores;
    //mutable std::vector<PixelFormat> supportedFormats;
    //mutable std::vector<GPUPresentMode> supportedPresentModes;

    ~VulkanRHISurface() override;
};

struct VulkanRHIAdapter final : public RHIAdapterImpl
{
    VulkanRHIFactory* factory = nullptr;
    bool debugUtils = false;

    VulkanRHIAdapter(VulkanRHIFactory* factory_, VkPhysicalDevice handle_);
    VkPhysicalDevice handle = nullptr;
    VulkanPhysicalDeviceExtensions extensions{};
    VulkanQueueFamilyIndices queueFamilyIndices;
    RHIAdapterType adapterType = RHIAdapterType_Other;
    bool synchronization2 = false;
    bool dynamicRendering = false;
    std::string driverDescription;
    bool supportsDepth32Stencil8 = false;
    bool supportsDepth24Stencil8 = false;
    bool supportsStencil8 = false;
    RHILimits limits{};

    // Features
    VkPhysicalDeviceFeatures2 features2 = {};
    VkPhysicalDeviceVulkan11Features features11 = {};
    VkPhysicalDeviceVulkan12Features features12 = {};
    VkPhysicalDeviceVulkan13Features features13 = {};
    VkPhysicalDeviceVulkan14Features features14 = {};

    // Core 1.3
    VkPhysicalDeviceMaintenance4Features maintenance4Features = {};
    VkPhysicalDeviceMaintenance4Properties maintenance4Properties = {};
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {};
    VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT extendedDynamicState2Features = {};

    // Core 1.4
    VkPhysicalDeviceMaintenance5Features maintenance5Features = {};
    VkPhysicalDeviceMaintenance6Features maintenance6Features = {};
    VkPhysicalDeviceMaintenance6Properties maintenance6Properties = {};
    VkPhysicalDevicePushDescriptorProperties pushDescriptorProps = {};

    // Extensions
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures{};
    VkPhysicalDevicePerformanceQueryFeaturesKHR performanceQueryFeatures{};
    VkPhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures = {};
    VkPhysicalDeviceTextureCompressionASTCHDRFeatures astcHdrFeatures{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures{};
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures{};

    // Properties
    VkPhysicalDeviceProperties2 properties2 = {};
    VkPhysicalDeviceVulkan11Properties properties11 = {};
    VkPhysicalDeviceVulkan12Properties properties12 = {};
    VkPhysicalDeviceVulkan13Properties properties13 = {};
    VkPhysicalDeviceVulkan14Properties properties14 = {};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerFilterMinmaxProperties = {};
    VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProperties = {};
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProps = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProperties = {};
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties = {};
    VkPhysicalDeviceMemoryProperties2 memoryProperties2 = {};

    bool Init();
    bool IsDepthStencilFormatSupported(VkFormat format) const;

    RHIAdapterType GetType() const override { return adapterType; }
    void GetInfo(RHIAdapterInfo* info) const override;
};

struct VulkanRHIFactory final : public RHIFactoryImpl
{
    bool debugUtils = false;
    bool xcbSurface = false;
    bool xlibSurface = false;
    bool waylandSurface = false;

    VkInstance handle = nullptr;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    std::vector<VulkanRHIAdapter*> adapters;

#define VULKAN_INSTANCE_FUNCTION(func) PFN_##func func;
#include "arhi_vulkan_funcs.h"

    ~VulkanRHIFactory() override;
    bool Init(const RHIFactoryDesc* desc);
    VulkanPhysicalDeviceExtensions QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice);
    VulkanQueueFamilyIndices QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue);
    bool GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);

    RHIBackend GetBackend() const override { return RHIBackend_Vulkan; }
    uint32_t GetAdapterCount() const override { return static_cast<uint32_t>(adapters.size()); }
    RHIAdapter GetAdapter(uint32_t index) const override;
    RHISurface CreateSurface(RHISurfaceSource source) override;
};

/* VulkanRHISurface */
VulkanRHISurface::~VulkanRHISurface()
{
    //for (size_t i = 0; i < backbufferTextures.size(); ++i)
    //{
    //    backbufferTextures[i]->Release();
    //}
    //
    //const uint64_t frameCount = device->frameCount;
    //device->destroyMutex.lock();
    //
    //for (size_t i = 0; i < backbufferTextures.size(); ++i)
    //{
    //    device->destroyedSemaphores.push_back(std::make_pair(swapchainAcquireSemaphores[i], frameCount));
    //    device->destroyedSemaphores.push_back(std::make_pair(swapchainReleaseSemaphores[i], frameCount));
    //}
    //
    //if (swapchain)
    //{
    //    device->destroyedSwapchains.push_back(std::make_pair(swapchain, frameCount));
    //    swapchain = VK_NULL_HANDLE;
    //}

    if (handle != VK_NULL_HANDLE)
    {
        factory->vkDestroySurfaceKHR(factory->handle, handle, nullptr);
        //device->destroyedSurfaces.push_back(std::make_pair(handle, frameCount));
        handle = VK_NULL_HANDLE;
    }

    //device->destroyMutex.unlock();
    //
    //backBufferIndex = 0;
    //backbufferTextures.clear();
    //swapchainExtent = {};
    //SAFE_RELEASE(device);
}

/* VulkanRHIAdapter */
VulkanRHIAdapter::VulkanRHIAdapter(VulkanRHIFactory* factory_, VkPhysicalDevice handle_)
    : factory(factory_)
    , debugUtils(factory_->debugUtils)
    , handle(handle_)
{

}

bool VulkanRHIAdapter::Init()
{
    extensions = factory->QueryPhysicalDeviceExtensions(handle);
    queueFamilyIndices = factory->QueryQueueFamilies(handle, extensions.video.queue);

    // Get current base properties
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    factory->vkGetPhysicalDeviceProperties2(handle, &properties2);

    VkBaseOutStructure* featureChainCurrent{ nullptr };
    auto addToFeatureChain = [&featureChainCurrent](auto* next) {
        auto n = reinterpret_cast<VkBaseOutStructure*>(next);
        featureChainCurrent->pNext = n;
        featureChainCurrent = n;
        };

    VkBaseOutStructure* propertiesChainCurrent{ nullptr };
    auto addToPropertiesChain = [&propertiesChainCurrent](auto* next) {
        auto n = reinterpret_cast<VkBaseOutStructure*>(next);
        propertiesChainCurrent->pNext = n;
        propertiesChainCurrent = n;
        };

    // Features
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

    properties11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
    properties12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;

    featureChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&features2);
    propertiesChainCurrent = reinterpret_cast<VkBaseOutStructure*>(&properties2);

    addToFeatureChain(&features11);
    addToFeatureChain(&features12);
    addToPropertiesChain(&properties11);
    addToPropertiesChain(&properties12);

    if (properties2.properties.apiVersion >= VK_API_VERSION_1_3)
    {
        features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        properties13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;

        addToFeatureChain(&features13);
        addToPropertiesChain(&properties13);
    }

    if (properties2.properties.apiVersion >= VK_API_VERSION_1_4)
    {
        features14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
        properties14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES;

        addToFeatureChain(&features14);
        addToPropertiesChain(&properties14);
    }

    // Properties
    samplerFilterMinmaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES;
    depthStencilResolveProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;

    addToPropertiesChain(&samplerFilterMinmaxProperties);
    addToPropertiesChain(&depthStencilResolveProperties);

    // Core in 1.3
    if (properties2.properties.apiVersion < VK_API_VERSION_1_3)
    {
        if (extensions.maintenance4)
        {
            maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES;
            maintenance4Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES;

            addToFeatureChain(&maintenance4Features);
            addToPropertiesChain(&maintenance4Properties);
        }

        if (extensions.dynamicRendering)
        {
            dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
            addToFeatureChain(&dynamicRenderingFeatures);
        }

        if (extensions.synchronization2)
        {
            synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
            addToFeatureChain(&synchronization2Features);
        }

        if (extensions.extendedDynamicState)
        {
            extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
            addToFeatureChain(&extendedDynamicStateFeatures);
        }

        if (extensions.extendedDynamicState2)
        {
            extendedDynamicState2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
            addToFeatureChain(&extendedDynamicState2Features);
        }

        if (extensions.textureCompressionAstcHdr)
        {
            astcHdrFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES;
            addToFeatureChain(&astcHdrFeatures);
        }
    }
    else
    {
        // Core in 1.4
        if (properties2.properties.apiVersion < VK_API_VERSION_1_4)
        {
            if (extensions.maintenance5)
            {
                maintenance5Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES;
                addToFeatureChain(&maintenance5Features);
            }

            if (extensions.maintenance6)
            {
                maintenance6Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES;
                maintenance6Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_PROPERTIES;

                addToFeatureChain(&maintenance6Features);
                addToPropertiesChain(&maintenance6Properties);
            }

            if (extensions.pushDescriptor)
            {
                pushDescriptorProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES;
                addToPropertiesChain(&pushDescriptorProps);
            }
        }
    }

    if (extensions.conservativeRasterization)
    {
        conservativeRasterizationProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONSERVATIVE_RASTERIZATION_PROPERTIES_EXT;
        addToPropertiesChain(&conservativeRasterizationProps);
    }

    if (extensions.depthClipEnable)
    {
        depthClipEnableFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_CLIP_ENABLE_FEATURES_EXT;
        addToFeatureChain(&depthClipEnableFeatures);
    }

    if (extensions.accelerationStructure)
    {
        ARHI_ASSERT(extensions.deferredHostOperations);

        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

        addToFeatureChain(&accelerationStructureFeatures);
        addToPropertiesChain(&accelerationStructureProperties);

        if (extensions.raytracingPipeline)
        {
            rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

            addToFeatureChain(&rayTracingPipelineFeatures);
            addToPropertiesChain(&rayTracingPipelineProperties);
        }

        if (extensions.rayQuery)
        {
            rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
            addToFeatureChain(&rayQueryFeatures);
        }
    }

    if (extensions.fragmentShadingRate)
    {
        fragmentShadingRateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
        fragmentShadingRateProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_PROPERTIES_KHR;

        addToFeatureChain(&fragmentShadingRateFeatures);
        addToPropertiesChain(&fragmentShadingRateProperties);
    }

    if (extensions.meshShader)
    {
        meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
        meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;

        addToFeatureChain(&meshShaderFeatures);
        addToPropertiesChain(&meshShaderProperties);
    }

    if (extensions.conditionalRendering)
    {
        conditionalRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CONDITIONAL_RENDERING_FEATURES_EXT;
        addToFeatureChain(&conditionalRenderingFeatures);
    }

    factory->vkGetPhysicalDeviceFeatures2(handle, &features2);
    factory->vkGetPhysicalDeviceProperties2(handle, &properties2);

    synchronization2 = features13.synchronization2 == VK_TRUE || synchronization2Features.synchronization2 == VK_TRUE;
    dynamicRendering = features13.dynamicRendering == VK_TRUE || dynamicRenderingFeatures.dynamicRendering == VK_TRUE;

    memoryProperties2 = {};
    memoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    factory->vkGetPhysicalDeviceMemoryProperties2(handle, &memoryProperties2);

    switch (properties2.properties.deviceType)
    {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            adapterType = RHIAdapterType_IntegratedGpu;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            adapterType = RHIAdapterType_DiscreteGpu;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            adapterType = RHIAdapterType_VirtualGpu;
            break;

        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            adapterType = RHIAdapterType_Cpu;
            break;
        default:
            adapterType = RHIAdapterType_Other;
            break;
    }

    driverDescription = properties12.driverName;
    if (properties12.driverInfo[0] != '\0')
    {
        driverDescription += std::string(": ") + properties12.driverInfo;
    }

    // The environment can request to various options for depth-stencil formats that could be
    // unavailable. Override the decision if it is not applicable.
    supportsDepth32Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT);
    supportsDepth24Stencil8 = IsDepthStencilFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT);
    supportsStencil8 = IsDepthStencilFormatSupported(VK_FORMAT_S8_UINT);

    // Init limits
    limits.maxTextureDimension1D = properties2.properties.limits.maxImageDimension1D;
    limits.maxTextureDimension2D = properties2.properties.limits.maxImageDimension2D;
    limits.maxTextureDimension3D = properties2.properties.limits.maxImageDimension3D;
    limits.maxTextureDimensionCube = properties2.properties.limits.maxImageDimensionCube;
    limits.maxTextureArrayLayers = properties2.properties.limits.maxImageArrayLayers;
    limits.maxBindGroups = properties2.properties.limits.maxBoundDescriptorSets;
    limits.maxConstantBufferBindingSize = properties2.properties.limits.maxUniformBufferRange;
    limits.maxStorageBufferBindingSize = properties2.properties.limits.maxStorageBufferRange;
    limits.minConstantBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minUniformBufferOffsetAlignment;
    limits.minStorageBufferOffsetAlignment = (uint32_t)properties2.properties.limits.minStorageBufferOffsetAlignment;
    limits.maxPushConstantsSize = properties2.properties.limits.maxPushConstantsSize;
    [[maybe_unused]] const uint32_t maxPushDescriptors = pushDescriptorProps.maxPushDescriptors;
    limits.maxBufferSize = properties13.maxBufferSize;
    limits.maxColorAttachments = properties2.properties.limits.maxColorAttachments;
    limits.maxViewports = properties2.properties.limits.maxViewports;
    limits.viewportBoundsMin = properties2.properties.limits.viewportBoundsRange[0];
    limits.viewportBoundsMax = properties2.properties.limits.viewportBoundsRange[1];

    /* Compute */
    limits.maxComputeWorkgroupStorageSize = properties2.properties.limits.maxComputeSharedMemorySize;
    limits.maxComputeInvocationsPerWorkgroup = properties2.properties.limits.maxComputeWorkGroupInvocations;

    limits.maxComputeWorkgroupSizeX = properties2.properties.limits.maxComputeWorkGroupSize[0];
    limits.maxComputeWorkgroupSizeY = properties2.properties.limits.maxComputeWorkGroupSize[1];
    limits.maxComputeWorkgroupSizeZ = properties2.properties.limits.maxComputeWorkGroupSize[2];

    limits.maxComputeWorkgroupsPerDimension = std::min({
        properties2.properties.limits.maxComputeWorkGroupCount[0],
        properties2.properties.limits.maxComputeWorkGroupCount[1],
        properties2.properties.limits.maxComputeWorkGroupCount[2],
        }
        );

    // Based on https://docs.vulkan.org/guide/latest/hlsl.html#_shader_model_coverage
    limits.shaderModel = GPUShaderModel_6_0;
    if (features11.multiview)
        limits.shaderModel = GPUShaderModel_6_1;
    if (features12.shaderFloat16 || features2.features.shaderInt16)
        limits.shaderModel = GPUShaderModel_6_2;
    if (extensions.accelerationStructure)
        limits.shaderModel = GPUShaderModel_6_3;
    if (limits.variableShadingRateTier >= GPUVariableRateShadingTier_2)
        limits.shaderModel = GPUShaderModel_6_4;
    //if (m_Desc.isMeshShaderSupported || m_Desc.rayTracingTier >= 2)
    //    m_Desc.shaderModel = 65;
    //if (m_Desc.isShaderAtomicsI64Supported)
    //    m_Desc.shaderModel = 66;
    //if (features.features.shaderStorageImageMultisample)
    //    m_Desc.shaderModel = 67;

    limits.conservativeRasterizationTier = RHIConservativeRasterizationTier_NotSupported;
    if (extensions.conservativeRasterization)
    {
        limits.conservativeRasterizationTier = RHIConservativeRasterizationTier_1;

        if (conservativeRasterizationProps.primitiveOverestimationSize < 1.0f / 2.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
            limits.conservativeRasterizationTier = RHIConservativeRasterizationTier_2;
        if (conservativeRasterizationProps.primitiveOverestimationSize <= 1.0 / 256.0f && conservativeRasterizationProps.degenerateTrianglesRasterized)
            limits.conservativeRasterizationTier = RHIConservativeRasterizationTier_3;
    }

    limits.variableShadingRateTier = GPUVariableRateShadingTier_NotSupported;
    if (extensions.fragmentShadingRate)
    {
        if (fragmentShadingRateFeatures.pipelineFragmentShadingRate == VK_TRUE)
        {
            limits.variableShadingRateTier = GPUVariableRateShadingTier_1;
        }

        if (fragmentShadingRateFeatures.primitiveFragmentShadingRate && fragmentShadingRateFeatures.attachmentFragmentShadingRate)
        {
            limits.variableShadingRateTier = GPUVariableRateShadingTier_2;
        }

        const auto& tileExtent = fragmentShadingRateProperties.minFragmentShadingRateAttachmentTexelSize;
        limits.variableShadingRateImageTileSize = std::max(tileExtent.width, tileExtent.height);
        limits.isAdditionalVariableShadingRatesSupported = fragmentShadingRateProperties.maxFragmentSize.height > 2 || fragmentShadingRateProperties.maxFragmentSize.width > 2;
    }

    // Ray tracing
    limits.rayTracingTier = GPURayTracingTier_NotSupported;
    if (features12.bufferDeviceAddress == VK_TRUE
        && accelerationStructureFeatures.accelerationStructure == VK_TRUE
        && rayTracingPipelineFeatures.rayTracingPipeline == VK_TRUE)
    {
        limits.rayTracingTier = GPURayTracingTier_1;

        if (rayQueryFeatures.rayQuery == VK_TRUE)
        {
            limits.rayTracingTier = GPURayTracingTier_2;
        }

        //if (OpacityMicromapFeatures.micromap)
        //    m_Desc.tiers.rayTracing++;

        limits.rayTracingShaderGroupIdentifierSize = rayTracingPipelineProperties.shaderGroupHandleSize;
        limits.rayTracingShaderTableAlignment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
        limits.rayTracingShaderTableMaxStride = rayTracingPipelineProperties.maxShaderGroupStride;
        limits.rayTracingShaderRecursionMaxDepth = rayTracingPipelineProperties.maxRayRecursionDepth;
        limits.rayTracingMaxGeometryCount = (uint32_t)accelerationStructureProperties.maxGeometryCount;
        limits.rayTracingScratchAlignment = accelerationStructureProperties.minAccelerationStructureScratchOffsetAlignment;
    }

    // Mesh shader
    limits.meshShaderTier = GPUMeshShaderTier_NotSupported;
    if (meshShaderFeatures.meshShader == VK_TRUE && meshShaderFeatures.taskShader == VK_TRUE)
    {
        limits.meshShaderTier = GPUMeshShaderTier_1;
    }

    return true;
}

void VulkanRHIAdapter::GetInfo(RHIAdapterInfo* info) const
{
    memset(info, 0, sizeof(RHIAdapterInfo));

    string::copy_safe(info->deviceName, sizeof(info->deviceName), properties2.properties.deviceName);
    info->vendor = RHIAdapterVendorFromID(properties2.properties.vendorID);
    info->vendorID = properties2.properties.vendorID;
    info->deviceID = properties2.properties.deviceID;

    uint32_t versionRaw = properties2.properties.driverVersion;

    switch (info->vendor)
    {
        case RHIAdapterVendor_NVIDIA:
            info->driverVersion[0] = static_cast<uint16_t>((versionRaw >> 22) & 0x3FF);
            info->driverVersion[1] = static_cast<uint16_t>((versionRaw >> 14) & 0x0FF);
            info->driverVersion[2] = static_cast<uint16_t>((versionRaw >> 6) & 0x0FF);
            info->driverVersion[3] = static_cast<uint16_t>(versionRaw & 0x003F);
            break;

        case RHIAdapterVendor_Intel:
#if defined(_WIN32)
            // Windows Vulkan driver releases together with D3D driver, so they share the same
            // version. But only CCC.DDDD is encoded in 32-bit driverVersion.
            info->driverVersion[0] = static_cast<uint16_t>(versionRaw >> 14);
            info->driverVersion[1] = static_cast<uint16_t>(versionRaw & 0x3FFF);
            break;
#endif

        default:
            // Use Vulkan driver conversions for other vendors
            info->driverVersion[0] = static_cast<uint16_t>(versionRaw >> 22);
            info->driverVersion[1] = static_cast<uint16_t>((versionRaw >> 12) & 0x3FF);
            info->driverVersion[2] = static_cast<uint16_t>(versionRaw & 0xFFF);
            break;
    }

    info->driverDescription = driverDescription.c_str();
    info->adapterType = adapterType;
}

bool VulkanRHIAdapter::IsDepthStencilFormatSupported(VkFormat format) const
{
    ARHI_ASSERT(format == VK_FORMAT_D16_UNORM_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_S8_UINT);

    VkFormatProperties2 props = {};
    props.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
    factory->vkGetPhysicalDeviceFormatProperties2(handle, format, &props);
    return props.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
}

/* VulkanRHIFactory */
VulkanRHIFactory::~VulkanRHIFactory()
{
    if (debugUtilsMessenger != VK_NULL_HANDLE)
    {
        vkDestroyDebugUtilsMessengerEXT(handle, debugUtilsMessenger, nullptr);
        debugUtilsMessenger = VK_NULL_HANDLE;
    }

    if (handle != VK_NULL_HANDLE)
    {
        vkDestroyInstance(handle, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

bool VulkanRHIFactory::Init(const RHIFactoryDesc* desc)
{
    uint32_t instanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr));
    std::vector<VkLayerProperties> availableInstanceLayers(instanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&instanceLayerCount, availableInstanceLayers.data()));

    uint32_t extensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableInstanceExtensions(extensionCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableInstanceExtensions.data()));

    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

    for (auto& availableExtension : availableInstanceExtensions)
    {
        if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
        {
            debugUtils = true;
            instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0)
        {
            instanceExtensions.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_xcb_surface") == 0)
        {
            xcbSurface = true;
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_xlib_surface") == 0)
        {
            xlibSurface = true;
        }
        else if (strcmp(availableExtension.extensionName, "VK_KHR_wayland_surface") == 0)
        {
            waylandSurface = true;
        }
    }

    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    // Enable surface extensions depending on os
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
    instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__APPLE__)
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    // https://vulkan.lunarg.com/doc/view/1.3.280.0/windows/synchronization2_layer.html
    // https://vulkan.lunarg.com/doc/view/latest/windows/shader_object_layer.html
    for (auto& availableLayer : availableInstanceLayers)
    {
        if (strcmp(availableLayer.layerName, "VK_LAYER_KHRONOS_synchronization2") == 0)
        {
            instanceLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
            break;
        }
    }
#else
    if (instance->xcbSurface)
    {
        instanceExtensions.push_back("VK_KHR_xcb_surface");
    }
    else
    {
        ALIMER_ASSERT(instance->xlibSurface);
        instanceExtensions.push_back("VK_KHR_xlib_surface");
    }

    if (instance->waylandSurface)
    {
        instanceExtensions.push_back("VK_KHR_wayland_surface");
    }
#endif

    const RHIValidationMode validationMode = (desc != nullptr) ? desc->validationMode : RHIValidationMode_Disabled;

    if (validationMode != RHIValidationMode_Disabled)
    {
        // Determine the optimal validation layers to enable that are necessary for useful debugging
        std::vector<const char*> optimalValidationLyers = { "VK_LAYER_KHRONOS_validation" };
        if (ValidateLayers(optimalValidationLyers, availableInstanceLayers))
        {
            instanceLayers.insert(instanceLayers.end(), optimalValidationLyers.begin(), optimalValidationLyers.end());
        }

    }

    bool validationFeatures = false;
    if (validationMode == RHIValidationMode_Gpu)
    {
        uint32_t layerInstanceExtensionCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, nullptr));
        std::vector<VkExtensionProperties> availableLayerInstanceExtensions(layerInstanceExtensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties("VK_LAYER_KHRONOS_validation", &layerInstanceExtensionCount, availableLayerInstanceExtensions.data()));

        for (auto& availableExtension : availableLayerInstanceExtensions)
        {
            if (strcmp(availableExtension.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0)
            {
                validationFeatures = true;
                instanceExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
            }
        }
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "Alimer";
    appInfo.engineVersion = VK_MAKE_VERSION(ARHI_VERSION_MAJOR, ARHI_VERSION_MINOR, ARHI_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
#if defined(__APPLE__)
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    createInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    createInfo.ppEnabledLayerNames = instanceLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{};

    if (validationMode != RHIValidationMode_Disabled && debugUtils)
    {
        debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            ;
        debugUtilsCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            ;

        if (validationMode == RHIValidationMode_Verbose)
        {
            debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
            debugUtilsCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

            debugUtilsCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }

        debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;
        createInfo.pNext = &debugUtilsCreateInfo;
    }

    VkValidationFeaturesEXT validationFeaturesInfo = { VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
    if (validationMode == RHIValidationMode_Gpu && validationFeatures)
    {
        static const VkValidationFeatureEnableEXT enable_features[3] = {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
            VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
        };
        validationFeaturesInfo.enabledValidationFeatureCount = 3;
        validationFeaturesInfo.pEnabledValidationFeatures = enable_features;
        PnextChainPushFront(&createInfo, &validationFeaturesInfo);
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &handle);
    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create Vulkan instance.");
        return false;
    }

#define VULKAN_INSTANCE_FUNCTION(fn) fn = (PFN_##fn)vkGetInstanceProcAddr(handle, #fn);
#include "arhi_vulkan_funcs.h"

    if (validationMode != RHIValidationMode_Disabled && debugUtils)
    {
        result = vkCreateDebugUtilsMessengerEXT(handle, &debugUtilsCreateInfo, nullptr, &debugUtilsMessenger);
        if (result != VK_SUCCESS)
        {
            VK_LOG_ERROR(result, "Could not create debug utils messenger");
        }
    }

#if defined(_DEBUG)
    arhi_log_info("Created VkInstance with version: %d.%d.%d",
        VK_VERSION_MAJOR(appInfo.apiVersion),
        VK_VERSION_MINOR(appInfo.apiVersion),
        VK_VERSION_PATCH(appInfo.apiVersion)
    );

    if (createInfo.enabledLayerCount)
    {
        arhi_log_info("Enabled %d Validation Layers:", createInfo.enabledLayerCount);

        for (uint32_t i = 0; i < createInfo.enabledLayerCount; ++i)
        {
            arhi_log_info("	\t%s", createInfo.ppEnabledLayerNames[i]);
        }
    }

    arhi_log_info("Enabled %d Instance Extensions:", createInfo.enabledExtensionCount);
    for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i)
    {
        arhi_log_info("	\t%s", createInfo.ppEnabledExtensionNames[i]);
    }
#endif

    // Enumerate physical device and detect best one.
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, nullptr));
    if (physicalDeviceCount == 0)
    {
        //alimerLogDebug(LogCategory_GPU, "Vulkan: Failed to find GPUs with Vulkan support");
        return false;
    }

    adapters.reserve(physicalDeviceCount);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(handle, &physicalDeviceCount, physicalDevices.data()));

    for (VkPhysicalDevice physicalDevice : physicalDevices)
    {
        // We require minimum 1.2
        VkPhysicalDeviceProperties2 physicalDeviceProperties = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        vkGetPhysicalDeviceProperties2(physicalDevice, &physicalDeviceProperties);
        if (physicalDeviceProperties.properties.apiVersion < VK_API_VERSION_1_2)
        {
            continue;
        }

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures);

        if (physicalDeviceFeatures.features.robustBufferAccess != VK_TRUE
            || physicalDeviceFeatures.features.fullDrawIndexUint32 != VK_TRUE
            || physicalDeviceFeatures.features.depthClamp != VK_TRUE
            || physicalDeviceFeatures.features.depthBiasClamp != VK_TRUE
            || physicalDeviceFeatures.features.fragmentStoresAndAtomics != VK_TRUE
            || physicalDeviceFeatures.features.imageCubeArray != VK_TRUE
            || physicalDeviceFeatures.features.independentBlend != VK_TRUE
            || physicalDeviceFeatures.features.sampleRateShading != VK_TRUE
            || physicalDeviceFeatures.features.shaderClipDistance != VK_TRUE
            || physicalDeviceFeatures.features.occlusionQueryPrecise != VK_TRUE)
        {
            continue;
        }

        VulkanPhysicalDeviceExtensions extensions = QueryPhysicalDeviceExtensions(physicalDevice);
        if (!extensions.swapchain)
        {
            continue;
        }

        VulkanQueueFamilyIndices queueFamilyIndices = QueryQueueFamilies(physicalDevice, extensions.video.queue);
        if (!queueFamilyIndices.IsComplete())
        {
            continue;
        }

#if defined(TODO)
        if (options && options->compatibleSurface != nullptr)
        {
            VulkanSurface* surface = static_cast<VulkanSurface*>(options->compatibleSurface);
            VkBool32 presentSupport = false;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
                physicalDevice,
                queueFamilyIndices.familyIndices[GPUCommandQueueType_Graphics],
                surface->handle,
                &presentSupport
            );

            // Present family not found, we cannot create SwapChain
            if (result != VK_SUCCESS || presentSupport == VK_FALSE)
            {
                continue;
            }
        }
#endif // defined(TODO)


        VulkanRHIAdapter* adapter = new VulkanRHIAdapter(this, physicalDevice);
        if (!adapter->Init())
        {
            delete adapter;
            continue;
        }

        adapters.push_back(adapter);
    }

    return true;
}

RHIAdapter VulkanRHIFactory::GetAdapter(uint32_t index) const
{
    if (index >= adapters.size())
        return nullptr;

    return adapters[index];
}

RHISurface VulkanRHIFactory::CreateSurface(RHISurfaceSource source)
{
    VkResult result = VK_SUCCESS;
    VkSurfaceKHR vk_surface = VK_NULL_HANDLE;

    switch (source->type)
    {
#if defined(_WIN32)
        case RHISurfaceSourceType::WindowsHWND:
        {
            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
            surfaceCreateInfo.hwnd = source->hwnd;

            result = vkCreateWin32SurfaceKHR(handle, &surfaceCreateInfo, nullptr, &vk_surface);
        }
        break;

#elif defined(__ANDROID__)
        case RHISurfaceSourceType::AndroidWindow:
        {
            VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.window = (ANativeWindow*)source->androidWindow;

            result = vkCreateAndroidSurfaceKHR(handle, &surfaceCreateInfo, nullptr, &vk_surface);
        }
        break;
#elif defined(__APPLE__)
        case RHISurfaceSourceType::MetalLayer:
        {
            VkMetalSurfaceCreateInfoEXT surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            surfaceCreateInfo.pLayer = source->metalLayer;

            result = vkCreateMetalSurfaceEXT(handle, &surfaceCreateInfo, nullptr, &vk_surface);
        }
        break;
#else
        case RHISurfaceSourceType::XlibWindow:
        {
            //if (xcb_surface)
            //{
            //    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
            //    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            //    surfaceCreateInfo.connection = xlibXcb.GetXCBConnection(static_cast<Display*>(surface->GetXDisplay()));
            //    surfaceCreateInfo.window = (xcb_window_t)window;
            //
            //    result = vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            //}
            //else
            //{
            //    VkXlibSurfaceCreateInfoKHR  surfaceCreateInfo = {};
            //    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            //    surfaceCreateInfo.dpy = static_cast<Display*>(surface->GetXDisplay());
            //    surfaceCreateInfo.window = surface->GetXWindow());
            //
            //    result = vkCreateXlibSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &swapChain->vkSurface);
            //}
        }
        break;

        case RHISurfaceSourceType::WaylandSurface:
        {
            VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.display = source->waylandDisplay;
            surfaceCreateInfo.surface = source->waylandSurface;

            result = vkCreateWaylandSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &vk_surface);
        }
        break;
#endif
    }

    if (result != VK_SUCCESS)
    {
        VK_LOG_ERROR(result, "Failed to create surface");
        return nullptr;
    }

    if (vk_surface == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    VulkanRHISurface* surface = new VulkanRHISurface();
    surface->factory = this;
    surface->handle = vk_surface;
    return surface;
}

VulkanPhysicalDeviceExtensions VulkanRHIFactory::QueryPhysicalDeviceExtensions(VkPhysicalDevice physicalDevice)
{
    uint32_t count = 0;
    VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    if (result != VK_SUCCESS)
        return {};

    std::vector<VkExtensionProperties> vk_extensions(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, vk_extensions.data());

    VulkanPhysicalDeviceExtensions extensions{};

    for (uint32_t i = 0; i < count; ++i)
    {
        // Core in 1.3
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_4_EXTENSION_NAME) == 0)
        {
            extensions.maintenance4 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
        {
            extensions.dynamicRendering = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
        {
            extensions.synchronization2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
        {
            extensions.extendedDynamicState = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0)
        {
            extensions.extendedDynamicState2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0)
        {
            extensions.pipelineCreationCacheControl = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) == 0)
        {
            extensions.formatFeatureFlags2 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0)
        {
            extensions.pushDescriptor = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            extensions.swapchain = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0)
        {
            extensions.memoryBudget = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME) == 0)
        {
            extensions.AMD_device_coherent_memory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0)
        {
            extensions.EXT_memory_priority = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
        {
            extensions.deferredHostOperations = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, "VK_KHR_portability_subset") == 0) // VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
        {
            extensions.portabilitySubset = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_DEPTH_CLIP_ENABLE_EXTENSION_NAME) == 0)
        {
            extensions.depthClipEnable = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME) == 0)
        {
            extensions.textureCompressionAstcHdr = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
        {
            extensions.shaderViewportIndexLayer = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) == 0)
        {
            extensions.conservativeRasterization = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_5_EXTENSION_NAME) == 0)
        {
            extensions.maintenance5 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_MAINTENANCE_6_EXTENSION_NAME) == 0)
        {
            extensions.maintenance6 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
        {
            extensions.accelerationStructure = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
        {
            extensions.raytracingPipeline = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
        {
            extensions.rayQuery = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME) == 0)
        {
            extensions.fragmentShadingRate = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
        {
            extensions.meshShader = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME) == 0)
        {
            extensions.conditionalRendering = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H264_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_h264 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_DECODE_H265_EXTENSION_NAME) == 0)
        {
            extensions.video.decode_h265 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_queue = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_H264_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_h264 = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_VIDEO_ENCODE_H265_EXTENSION_NAME) == 0)
        {
            extensions.video.encode_h265 = true;
        }

#if defined(_WIN32)
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalMemory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalSemaphore = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_WIN32_EXTENSION_NAME) == 0)
        {
            extensions.externalFence = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0)
        {
            extensions.win32_full_screen_exclusive = true;
        }
#else
        if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalMemory = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalSemaphore = true;
        }
        else if (strcmp(vk_extensions[i].extensionName, VK_KHR_EXTERNAL_FENCE_FD_EXTENSION_NAME) == 0)
        {
            extensions.externalFence = true;
        }
#endif
    }

    VkPhysicalDeviceProperties2 properties2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    vkGetPhysicalDeviceProperties2(physicalDevice, &properties2);

    // Core 1.4
    if (properties2.properties.apiVersion >= VK_API_VERSION_1_4)
    {
        extensions.maintenance5 = true;
        extensions.maintenance6 = true;
        extensions.pushDescriptor = true;
    }

    // Core 1.3
    if (properties2.properties.apiVersion >= VK_API_VERSION_1_3)
    {
        extensions.maintenance4 = true;
        extensions.dynamicRendering = true;
        extensions.synchronization2 = true;
        extensions.extendedDynamicState = true;
        extensions.extendedDynamicState2 = true;
        extensions.pipelineCreationCacheControl = true;
        extensions.formatFeatureFlags2 = true;
    }

    return extensions;

}

VulkanQueueFamilyIndices VulkanRHIFactory::QueryQueueFamilies(VkPhysicalDevice physicalDevice, bool supportsVideoQueue)
{

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilies(queueFamilyCount);
    std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo(queueFamilyCount);
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;

        if (supportsVideoQueue)
        {
            queueFamilies[i].pNext = &queueFamiliesVideo[i];
            queueFamiliesVideo[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_VIDEO_PROPERTIES_KHR;
        }
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyCount, queueFamilies.data());

    VulkanQueueFamilyIndices indices;
    indices.queueFamilyCount = queueFamilyCount;
    indices.queueOffsets.resize(queueFamilyCount);
    indices.queuePriorities.resize(queueFamilyCount);

    const auto FindVacantQueue = [&](RHIQueueType type, VkQueueFlags requiredFlags, VkQueueFlags ignoreFlags, float priority) -> bool
        {
            for (uint32_t familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++)
            {
                if ((queueFamilies[familyIndex].queueFamilyProperties.queueFlags & ignoreFlags) != 0)
                    continue;

                // A graphics queue candidate must support present for us to select it.
                if ((requiredFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
                {
                    bool supported = GetPresentationSupport(physicalDevice, familyIndex);
                    if (!supported)
                        continue;
                }

                // A video decode queue candidate must support VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR or VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                if ((requiredFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR) == 0 &&
                        (videoCodecOperations & VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR) == 0)
                    {
                        continue;
                    }
                }

#if defined(RHI_VIDEO_ENCODE)
                // A video decode queue candidate must support VK_VIDEO_CODEC_OPERATION_DECODE_H264_BIT_KHR or VK_VIDEO_CODEC_OPERATION_DECODE_H265_BIT_KHR
                if ((required & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) != 0)
                {
                    VkVideoCodecOperationFlagsKHR videoCodecOperations = queueFamiliesVideo[familyIndex].videoCodecOperations;

                    if ((videoCodecOperations & VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_EXT) == 0 &&
                        (videoCodecOperations & VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_EXT) == 0)
                    {
                        continue;
                    }
                }
#endif

                if (queueFamilies[familyIndex].queueFamilyProperties.queueCount &&
                    (queueFamilies[familyIndex].queueFamilyProperties.queueFlags & requiredFlags) == requiredFlags)
                {
                    indices.familyIndices[type] = familyIndex;
                    queueFamilies[familyIndex].queueFamilyProperties.queueCount--;
                    indices.queueIndices[type] = indices.queueOffsets[familyIndex]++;
                    indices.queuePriorities[familyIndex].push_back(priority);
                    return true;
                }
            }

            return false;
        };

    if (!FindVacantQueue(RHIQueueType_Graphics, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0, 0.5f))
    {
        arhi_log_error("Vulkan: Could not find suitable graphics queue.");
        return indices;
    }

    // XXX: This assumes timestamp valid bits is the same for all queue types.
    indices.timestampValidBits = queueFamilies[indices.familyIndices[RHIQueueType_Graphics]].queueFamilyProperties.timestampValidBits;

    // Prefer standalone compute queue. If not, fall back to another graphics queue.
    if (!FindVacantQueue(RHIQueueType_Compute, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f)
        && !FindVacantQueue(RHIQueueType_Compute, VK_QUEUE_COMPUTE_BIT, 0, 1.0f))
    {
        // Fallback to the graphics queue if we must.
        indices.familyIndices[RHIQueueType_Compute] = indices.familyIndices[RHIQueueType_Graphics];
        indices.queueIndices[RHIQueueType_Compute] = indices.queueIndices[RHIQueueType_Graphics];
    }

    // For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
    // If not, fallback to a dedicated compute queue.
    // Finally, fallback to same queue as compute.
    if (!FindVacantQueue(RHIQueueType_Copy, VK_QUEUE_TRANSFER_BIT, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0.5f)
        && !FindVacantQueue(RHIQueueType_Copy, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_GRAPHICS_BIT, 0.5f))
    {
        indices.familyIndices[RHIQueueType_Copy] = indices.familyIndices[RHIQueueType_Compute];
        indices.queueIndices[RHIQueueType_Copy] = indices.queueIndices[RHIQueueType_Compute];
    }

    if (supportsVideoQueue)
    {
#if TODO_VIDEO
        if (!FindVacantQueue(indices.familyIndices[GPUCommandQueueType_VideoDecode],
            indices.queueIndices[GPUCommandQueueType_VideoDecode],
            VK_QUEUE_VIDEO_DECODE_BIT_KHR, 0, 0.5f))
        {
            indices.familyIndices[GPUCommandQueueType_VideoDecode] = VK_QUEUE_FAMILY_IGNORED;
            indices.queueIndices[GPUCommandQueueType_VideoDecode] = UINT32_MAX;
        }
#endif // TODO_VIDEO


#ifdef VK_ENABLE_BETA_EXTENSIONS
        //if ((flags & CONTEXT_CREATION_ENABLE_VIDEO_ENCODE_BIT) != 0)
        //{
        //    if (!find_vacant_queue(queue_info.family_indices[QUEUE_INDEX_VIDEO_ENCODE],
        //        queue_indices[QUEUE_INDEX_VIDEO_ENCODE],
        //        VK_QUEUE_VIDEO_ENCODE_BIT_KHR, 0, 0.5f))
        //    {
        //        queue_info.family_indices[QUEUE_INDEX_VIDEO_ENCODE] = VK_QUEUE_FAMILY_IGNORED;
        //        queue_indices[QUEUE_INDEX_VIDEO_ENCODE] = UINT32_MAX;
        //    }
        //}
#endif
    }

    return indices;
}

bool VulkanRHIFactory::GetPresentationSupport(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex)
{
#if defined(_WIN32)
    //PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
    if (!vkGetPhysicalDeviceWin32PresentationSupportKHR)
    {
        return false;
    }

    return vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex) == VK_TRUE;
#elif defined(__ANDROID__)
    return true;
#elif defined(__APPLE__)
    return true;
#else
    return true;
#endif
}

struct VK_State {
#if defined(_WIN32)
    HMODULE vk_module;
#else
    void* vk_module;
#endif

    ~VK_State()
    {
        if (vk_module)
        {
#if defined(_WIN32)
            FreeLibrary(vk_module);
#else
            dlclose(vk_module);
#endif
            vk_module = nullptr;
        }
    }

} vk_state;

bool Vulkan_IsSupported(void)
{
    static bool available_initialized = false;
    static bool available = false;

    if (available_initialized) {
        return available;
    }

    available_initialized = true;
#if defined(_WIN32)
    vk_state.vk_module = LoadLibraryW(L"vulkan-1.dll");
    if (!vk_state.vk_module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vk_state.vk_module, "vkGetInstanceProcAddr");
#elif defined(__APPLE__)
    vk_state.vk_module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    // Add support for using Vulkan and MoltenVK in a Framework. App store rules for iOS
    // strictly enforce no .dylib's. If they aren't found it just falls through
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("vulkan.framework/vulkan", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        vk_state.vk_module = dlopen("MoltenVK.framework/MoltenVK", RTLD_NOW | RTLD_LOCAL);
    // modern versions of macOS don't search /usr/local/lib automatically contrary to what man dlopen says
    // Vulkan SDK uses this as the system-wide installation location, so we're going to fallback to this if all else fails
    if (!vk_state.vk_module && getenv("DYLD_FALLBACK_LIBRARY_PATH") == NULL)
        vk_state.vk_module = dlopen("/usr/local/lib/libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk_state.vk_module, "vkGetInstanceProcAddr");
#else
    vk_state.vk_module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!vk_state.vk_module) {
        vk_state.vk_module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (!vk_state.vk_module) {
        return false;
    }
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk_state.vk_module, "vkGetInstanceProcAddr");
#endif

#define VULKAN_GLOBAL_FUNCTION(name) \
    name = (PFN_##name)vkGetInstanceProcAddr(VK_NULL_HANDLE, #name); \
    if (name == NULL) { \
        arhi_log_warn("vkGetInstanceProcAddr(VK_NULL_HANDLE, \"" #name "\") failed"); \
        return false; \
    }
#include "arhi_vulkan_funcs.h"

    // We require vulkan 1.2
    uint32_t apiVersion;
    if (vkEnumerateInstanceVersion(&apiVersion) != VK_SUCCESS)
        return false;

    // Check if the Vulkan API version is sufficient.
    static constexpr uint32_t kMinimumVulkanVersion = VK_API_VERSION_1_2;
    if (apiVersion < kMinimumVulkanVersion)
    {
        arhi_log_warn("The Vulkan API version supported on the system (%d.%d.%d) is too low, at least %d.%d.%d is required.",
            VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion),
            VK_API_VERSION_MAJOR(kMinimumVulkanVersion), VK_API_VERSION_MINOR(kMinimumVulkanVersion), VK_API_VERSION_PATCH(kMinimumVulkanVersion)
        );
        return false;
    }

    // Spec says: A non-zero variant indicates the API is a variant of the Vulkan API and applications will typically need to be modified to run against it.
    if (VK_API_VERSION_VARIANT(apiVersion) != 0)
    {
        arhi_log_warn("The Vulkan API supported on the system uses an unexpected variant: %d.", VK_API_VERSION_VARIANT(apiVersion));
        return false;
    }

    available = true;
    return true;
}

RHIFactoryImpl* Vulkan_CreateFactory(const RHIFactoryDesc* desc)
{
    VulkanRHIFactory* factory = new VulkanRHIFactory();
    if (!factory->Init(desc))
    {
        delete factory;
        return nullptr;
    }

    ARHI_UNUSED(desc);
    return factory;
}

#endif /* defined(ARHI_VULKAN) */
