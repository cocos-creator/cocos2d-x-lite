#include "VKStd.h"
#include "VKContext.h"
#include "VKUtils.h"
#include "VKGPUObjects.h"
#include "VKDevice.h"
#include "VKRenderPass.h"
#include "VKTexture.h"
#include "VKTextureView.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"

#define CC_GFX_DEBUG

NS_CC_BEGIN

namespace
{
#ifdef CC_GFX_DEBUG
    VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void * user_data)
    {
        // Log debug messge
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            CC_LOG_WARNING("%s: %s", callbackData->pMessageIdName, callbackData->pMessage);
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            CC_LOG_ERROR("%s: %s", callbackData->pMessageIdName, callbackData->pMessage);
            assert(false);
        }
        return VK_FALSE;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT type,
        uint64_t object, size_t location, int32_t messageCode, const char *layerPrefix, const char *message, void *userData)
    {
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
        {
            CC_LOG_ERROR("VError: %s: %s", layerPrefix, message);
            assert(false);
        }
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
        {
            CC_LOG_ERROR("VWarning: %s: %s", layerPrefix, message);
        }
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
        {
            CC_LOG_INFO("VPerfWarning: %s: %s", layerPrefix, message);
        }
        else
        {
            CC_LOG_INFO("VInfo: %s: %s", layerPrefix, message);
        }
        return VK_FALSE;
    }
#endif
}

CCVKContext::CCVKContext(GFXDevice* device)
    : GFXContext(device){
}

CCVKContext::~CCVKContext() {
}

bool CCVKContext::initialize(const GFXContextInfo &info) {

    _vsyncMode = info.vsyncMode;
    _windowHandle = info.windowHandle;

    if (!info.sharedCtx)
    {
        _isPrimaryContex = true;
        _windowHandle = info.windowHandle;

        _gpuContext = CC_NEW(CCVKGPUContext);

        // only enable the absolute essentials for now
        std::vector<const char *> requestedLayers
        {

        };
        std::vector<const char *> requestedExtensions
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
        };

        ///////////////////// Instance Creation /////////////////////

        if (volkInitialize())
        {
            return false;
        }

        uint availableLayerCount;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr));
        std::vector<VkLayerProperties> supportedLayers(availableLayerCount);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&availableLayerCount, supportedLayers.data()));

        uint availableExtensionCount;
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr));
        std::vector<VkExtensionProperties> supportedExtensions(availableExtensionCount);
        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, supportedExtensions.data()));

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        requestedExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        requestedExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        requestedExtensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        requestedExtensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        requestedExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        requestedExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#else
#   pragma error Platform not supported
#endif

#ifdef CC_GFX_DEBUG
        // Determine the optimal validation layers to enable that are necessary for useful debugging
        std::vector<std::vector<const char *>> validationLayerPriorityList
        {
            // The preferred validation layer is "VK_LAYER_KHRONOS_validation"
            {"VK_LAYER_KHRONOS_validation"},

            // Otherwise we fallback to using the LunarG meta layer
            {"VK_LAYER_LUNARG_standard_validation"},

            // Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
            {
                "VK_LAYER_GOOGLE_threading",
                "VK_LAYER_LUNARG_parameter_validation",
                "VK_LAYER_LUNARG_object_tracker",
                "VK_LAYER_LUNARG_core_validation",
                "VK_LAYER_GOOGLE_unique_objects",
            },

            // Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
            {"VK_LAYER_LUNARG_core_validation"}
        };
        for (auto &validationLayers : validationLayerPriorityList)
        {
            bool found = true;
            for (auto &layer : validationLayers)
            {
                if (!isLayerSupported(layer, supportedLayers))
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                requestedLayers.insert(requestedLayers.end(), validationLayers.begin(), validationLayers.end());
                break;
            }
        }

        // Check if VK_EXT_debug_utils is supported, which supersedes VK_EXT_Debug_Report
        bool debugUtils = false;
        if (isExtensionSupported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, supportedExtensions))
        {
            debugUtils = true;
            requestedExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
        {
            requestedExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
#endif

        // just filter out the unsupported layers & extensions
        for (auto &layer : requestedLayers)
        {
            if (isLayerSupported(layer, supportedLayers))
            {
                _layers.push_back(layer);
            }
        }
        for (auto &extension : requestedExtensions)
        {
            if (isExtensionSupported(extension, supportedExtensions))
            {
                _extensions.push_back(extension);
            }
        }

        uint apiVersion;
        vkEnumerateInstanceVersion(&apiVersion);

        VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        app.pEngineName = "Cocos Creator";
        app.apiVersion = apiVersion;

        VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        instanceInfo.pApplicationInfo = &app;
        instanceInfo.enabledExtensionCount = toU32(_extensions.size());
        instanceInfo.ppEnabledExtensionNames = _extensions.data();
        instanceInfo.enabledLayerCount = toU32(_layers.size());
        instanceInfo.ppEnabledLayerNames = _layers.data();

#ifdef CC_GFX_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT };
        if (debugUtils)
        {
            debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
            debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
            debugUtilsCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;

            instanceInfo.pNext = &debugUtilsCreateInfo;
        }
        else
        {
            debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugReportCreateInfo.pfnCallback = debugReportCallback;

            instanceInfo.pNext = &debugReportCreateInfo;
        }
#endif

        // Create the Vulkan instance
        VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &_gpuContext->vkInstance));

        volkLoadInstance(_gpuContext->vkInstance);

#ifdef CC_GFX_DEBUG
        if (debugUtils)
        {
            VK_CHECK(vkCreateDebugUtilsMessengerEXT(_gpuContext->vkInstance, &debugUtilsCreateInfo, nullptr, &_gpuContext->vkDebugUtilsMessenger));
        }
        else
        {
            VK_CHECK(vkCreateDebugReportCallbackEXT(_gpuContext->vkInstance, &debugReportCreateInfo, nullptr, &_gpuContext->vkDebugReport));
        }
#endif

        _majorVersion = VK_VERSION_MAJOR(apiVersion);
        _minorVersion = VK_VERSION_MINOR(apiVersion);

        ///////////////////// Surface Creation /////////////////////

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.window = (ANativeWindow*)_windowHandle;
        VK_CHECK(vkCreateAndroidSurfaceKHR(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(0);
        surfaceCreateInfo.hwnd = (HWND)_windowHandle;
        VK_CHECK(vkCreateWin32SurfaceKHR(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#elif defined(VK_USE_PLATFORM_IOS_MVK)
        VkIOSSurfaceCreateInfoMVK surfaceCreateInfo{ VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK };
        surfaceCreateInfo.pView = (void*)_windowHandle;
        VK_CHECK(vkCreateIOSSurfaceMVK(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
        VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo{ VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK };
        surfaceCreateInfo.pView = (void*)_windowHandle;
        VK_CHECK(vkCreateMacOSSurfaceMVK(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo{ VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.display = nullptr; // TODO
        surfaceCreateInfo.surface = (wl_surface*)_windowHandle;
        VK_CHECK(vkCreateWaylandSurfaceKHR(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
        VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
        surfaceCreateInfo.connection = nullptr; // TODO
        surfaceCreateInfo.window = (xcb_window_t)_windowHandle;
        VK_CHECK(vkCreateXcbSurfaceKHR(_gpuContext->vkInstance, &surfaceCreateInfo, nullptr, &_gpuContext->vkSurface));
#else
#   pragma error Platform not supported
#endif

        ///////////////////// Physical Device Selection /////////////////////

        // Querying valid physical devices on the machine
        uint physicalDeviceCount{ 0 };
        VK_CHECK(vkEnumeratePhysicalDevices(_gpuContext->vkInstance, &physicalDeviceCount, nullptr));

        if (physicalDeviceCount < 1)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> physicalDeviceHandles(physicalDeviceCount);
        VK_CHECK(vkEnumeratePhysicalDevices(_gpuContext->vkInstance, &physicalDeviceCount, physicalDeviceHandles.data()));

        std::vector<VkPhysicalDeviceProperties> physicalDeviceProperties(physicalDeviceCount);

        uint deviceIndex;
        for (deviceIndex = 0u; deviceIndex < physicalDeviceCount; ++deviceIndex)
        {
            VkPhysicalDeviceProperties& properties = physicalDeviceProperties[deviceIndex];
            vkGetPhysicalDeviceProperties(physicalDeviceHandles[deviceIndex], &properties);
        }

        for (deviceIndex = 0u; deviceIndex < physicalDeviceCount; ++deviceIndex)
        {
            VkPhysicalDeviceProperties& properties = physicalDeviceProperties[deviceIndex];
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                break;
            }
        }

        _gpuContext->physicalDevice = physicalDeviceHandles[deviceIndex];
        _gpuContext->physicalDeviceProperties = physicalDeviceProperties[deviceIndex];
        vkGetPhysicalDeviceFeatures(_gpuContext->physicalDevice, &_gpuContext->physicalDeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(_gpuContext->physicalDevice, &_gpuContext->physicalDeviceMemoryProperties);
        uint queueFamilyPropertiesCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_gpuContext->physicalDevice, &queueFamilyPropertiesCount, nullptr);
        _gpuContext->queueFamilyProperties.resize(queueFamilyPropertiesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(_gpuContext->physicalDevice, &queueFamilyPropertiesCount, _gpuContext->queueFamilyProperties.data());
        _gpuContext->queueFamilyPresentables.resize(queueFamilyPropertiesCount);
        for (uint propertyIndex = 0U; propertyIndex < queueFamilyPropertiesCount; propertyIndex++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(_gpuContext->physicalDevice, propertyIndex,
                _gpuContext->vkSurface, &_gpuContext->queueFamilyPresentables[propertyIndex]);
        }

        ///////////////////// Swapchain Preperation /////////////////////

        _colorFmt = GFXFormat::BGRA8;
        _depthStencilFmt = GFXFormat::D24S8;

        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_gpuContext->physicalDevice, _gpuContext->vkSurface, &surfaceCapabilities);

        uint surfaceFormatCount = 0u;
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuContext->physicalDevice, _gpuContext->vkSurface, &surfaceFormatCount, nullptr));
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_gpuContext->physicalDevice, _gpuContext->vkSurface, &surfaceFormatCount, surfaceFormats.data()));

        uint presentModeCount = 0u;
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuContext->physicalDevice, _gpuContext->vkSurface, &presentModeCount, nullptr));
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_gpuContext->physicalDevice, _gpuContext->vkSurface, &presentModeCount, presentModes.data()));

        VkExtent2D imageExtent{ surfaceCapabilities.currentExtent.width, surfaceCapabilities.currentExtent.height };

        VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
        // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
        if ((surfaceFormatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
        {
            colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
            colorSpace = surfaceFormats[0].colorSpace;
        }
        else
        {
            // iterate over the list of available surface format and
            // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
            bool imageFormatFound = false;
            for (auto&& surfaceFormat : surfaceFormats)
            {
                if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
                {
                    colorFormat = surfaceFormat.format;
                    colorSpace = surfaceFormat.colorSpace;
                    imageFormatFound = true;
                    break;
                }
            }

            // in case VK_FORMAT_B8G8R8A8_UNORM is not available
            // select the first available color format
            if (!imageFormatFound)
            {
                colorFormat = surfaceFormats[0].format;
                colorSpace = surfaceFormats[0].colorSpace;
            }
        }

        std::vector<std::pair<GFXFormat, VkFormat>> depthFormatPriorityList =
        {
            { GFXFormat::D32F_S8, VK_FORMAT_D32_SFLOAT_S8_UINT },
            { GFXFormat::D24S8, VK_FORMAT_D24_UNORM_S8_UINT },
            { GFXFormat::D16S8, VK_FORMAT_D16_UNORM_S8_UINT },
            { GFXFormat::D32F, VK_FORMAT_D32_SFLOAT },
            { GFXFormat::D16, VK_FORMAT_D16_UNORM }
        };
        for (auto& format : depthFormatPriorityList)
        {
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(_gpuContext->physicalDevice, format.second, &formatProperties);
            // Format must support depth stencil attachment for optimal tiling
            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
                {
                    _depthStencilFmt = format.first;
                    break;
                }
            }
        }

        // Select a present mode for the swapchain

        std::vector<VkPresentModeKHR> presentModePriorityList;

        switch (_vsyncMode)
        {
        case GFXVsyncMode::OFF: presentModePriorityList.insert(presentModePriorityList.end(), { VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR }); break;
        case GFXVsyncMode::ON: presentModePriorityList.insert(presentModePriorityList.end(), { VK_PRESENT_MODE_FIFO_KHR }); break;
        case GFXVsyncMode::RELAXED: presentModePriorityList.insert(presentModePriorityList.end(), { VK_PRESENT_MODE_FIFO_RELAXED_KHR, VK_PRESENT_MODE_FIFO_KHR }); break;
        case GFXVsyncMode::MAILBOX: presentModePriorityList.insert(presentModePriorityList.end(), { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR }); break;
        case GFXVsyncMode::HALF: presentModePriorityList.insert(presentModePriorityList.end(), { VK_PRESENT_MODE_FIFO_KHR }); break; // TODO
        }

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        for (auto &presentMode : presentModePriorityList)
        {
            if (std::find(presentModes.begin(), presentModes.end(), presentMode) != presentModes.end())
            {
                swapchainPresentMode = presentMode;
            }
        }

        // Determine the number of images
        uint desiredNumberOfSwapchainImages = surfaceCapabilities.minImageCount + 1;
        if ((surfaceCapabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfaceCapabilities.maxImageCount))
        {
            desiredNumberOfSwapchainImages = surfaceCapabilities.maxImageCount;
        }

        // Find the transformation of the surface
        VkSurfaceTransformFlagBitsKHR preTransform;
        if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        {
            // We prefer a non-rotated transform
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else
        {
            preTransform = surfaceCapabilities.currentTransform;
        }

        // Find a supported composite alpha format (not all devices support alpha opaque)
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        // Simply select the first composite alpha format available
        std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (auto& compositeAlphaFlag : compositeAlphaFlags)
        {
            if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag)
            {
                compositeAlpha = compositeAlphaFlag;
                break;
            };
        }
        VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // Enable transfer source on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        {
            imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        // Enable transfer destination on swap chain images if supported
        if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        {
            imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        _gpuContext->swapchainCreateInfo.surface = _gpuContext->vkSurface;
        _gpuContext->swapchainCreateInfo.minImageCount = desiredNumberOfSwapchainImages;
        _gpuContext->swapchainCreateInfo.imageFormat = colorFormat;
        _gpuContext->swapchainCreateInfo.imageColorSpace = colorSpace;
        _gpuContext->swapchainCreateInfo.imageExtent = imageExtent;
        _gpuContext->swapchainCreateInfo.imageUsage = imageUsage;
        _gpuContext->swapchainCreateInfo.imageArrayLayers = 1;
        _gpuContext->swapchainCreateInfo.preTransform = preTransform;
        _gpuContext->swapchainCreateInfo.compositeAlpha = compositeAlpha;
        _gpuContext->swapchainCreateInfo.presentMode = swapchainPresentMode;
        _gpuContext->swapchainCreateInfo.clipped = VK_TRUE; // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area

    } else {
      CCVKContext* sharedCtx = (CCVKContext*)info.sharedCtx;

      _majorVersion = sharedCtx->majorVersion();
      _minorVersion = sharedCtx->minorVersion();

      // TODO
    }

    return true;
}

void CCVKContext::destroy()
{
    if (_gpuContext)
    {
        if (_gpuContext->vkSurface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(_gpuContext->vkInstance, _gpuContext->vkSurface, nullptr);
            _gpuContext->vkSurface = VK_NULL_HANDLE;
        }

#ifdef CC_GFX_DEBUG
        if (_gpuContext->vkDebugUtilsMessenger != VK_NULL_HANDLE)
        {
            vkDestroyDebugUtilsMessengerEXT(_gpuContext->vkInstance, _gpuContext->vkDebugUtilsMessenger, nullptr);
            _gpuContext->vkDebugUtilsMessenger = VK_NULL_HANDLE;
        }
        if (_gpuContext->vkDebugReport != VK_NULL_HANDLE)
        {
            vkDestroyDebugReportCallbackEXT(_gpuContext->vkInstance, _gpuContext->vkDebugReport, nullptr);
            _gpuContext->vkDebugReport = VK_NULL_HANDLE;
        }
#endif

        if (_gpuContext->vkInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(_gpuContext->vkInstance, nullptr);
            _gpuContext->vkInstance = VK_NULL_HANDLE;
        }

        CC_DELETE(_gpuContext);
        _gpuContext = nullptr;
    }
}

NS_CC_END
