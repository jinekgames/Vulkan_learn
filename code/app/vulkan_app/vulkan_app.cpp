#include <vulkan_app/vulkan_app.h>
#include <app_consts.h>

#include <algorithm>
#include <map>
#include <string_view>

VulkanApp::VulkanApp() {
    requiredParams.instanseExtensions = {};
    requiredParams.deviceExtensions = {};
    requiredParams.deviceFeatures.geometryShader = true;
    requiredParams.validationLayers.reserve(vulkanValidationLayers.size());
    for (auto layer : vulkanValidationLayers) {
        requiredParams.validationLayers.push_back(layer);
    }
}

VulkanApp::~VulkanApp() {
    Clear();
}

AppResult VulkanApp::Init() {

    // Create Vk instanse
    APP_CHECK_CALL(CreateVkInstance());
    setupDebugMessenger();
    // Find physical device
    APP_CHECK_CALL(FindPhysicalDevice());
    // Create logical device
    APP_CHECK_CALL(CreateLogicalDevice());

    return APP_CODE_OK;
}

AppResult VulkanApp::CreateVkInstance() {

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Get required instance layers
#if VALIDATION_LAYERS_ENABLED
    // Filter unavailable instance layers
    {
        LayersList unsupportedLayers{};
        AppResult r = CheckSupportedInstanceLayers(requiredParams.validationLayers, unsupportedLayers);
        if (APP_CHECK_RESULT(r)) {
            if (unsupportedLayers.empty()) {
                PRINT("All requested Vulkan validation layers are supported");
            } else {
                PRINT_W("Following Vulkan validation layers couldn't be registered. They will be skipped");
                for (auto layer : unsupportedLayers) {
                    PRINT_W(" - %s", layer);
                    requiredParams.validationLayers.erase(
                        std::find(requiredParams.validationLayers.begin(), requiredParams.validationLayers.end(), layer)
                    );
                }
            }
        } else {
            return APP_CODE_VK_INIT_FAIURE;
        }
    }
#endif

    // Get required Vk extensions
#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    {
        uint32_t extensionCount = 0;
        glfwGetRequiredInstanceExtensions(&extensionCount);
        const char** glfwExts = glfwGetRequiredInstanceExtensions(&extensionCount);
        requiredParams.instanseExtensions.resize(extensionCount);
        for (size_t i = 0; i < extensionCount; ++i) {
            requiredParams.instanseExtensions[i] = glfwExts[i];
        }
#if defined(MAC_OS)
        extensions.push_back("VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME");
#endif
#if VALIDATION_LAYERS_ENABLED
        requiredParams.instanseExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    }
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif

    // Check for instance extensions support
    {
        ExtensionsList unsupportedExts{};
        AppResult r = CheckSupportedInstanceExtensions(requiredParams.instanseExtensions, unsupportedExts);
        // @remind can also be checked in layers
        if (APP_CHECK_RESULT(r)) {
            if (unsupportedExts.empty()) {
                PRINT("All required Vulkan extensions are supported");
            } else {
                PRINT_E("Following Vulkan extensions are not supported by your system");
                for (auto ext : unsupportedExts) {
                    PRINT_E(" - %s", ext);
                }
                return APP_CODE_VK_INIT_FAIURE;
            }
        } else {
            return APP_CODE_VK_INIT_FAIURE;
        }
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &appInfo;
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(requiredParams.instanseExtensions.size());
    createInfo.ppEnabledExtensionNames  = requiredParams.instanseExtensions.data();
#if VALIDATION_LAYERS_ENABLED
    createInfo.enabledLayerCount        = static_cast<uint32_t>(requiredParams.validationLayers.size());
    createInfo.ppEnabledLayerNames      = requiredParams.validationLayers.data();
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#else
    createInfo.enabledLayerCount        = 0;
    createInfo.pNext = nullptr;
#endif
#if defined(MAC_OS)
    createInfo.flags                   |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkResult r = vkCreateInstance(&createInfo, nullptr, &vkInst);
    if (r != VK_SUCCESS) {
        PRINT_E("Failed to create Vulkan instanse. Vk error code: %d", r);
        return APP_CODE_VK_INIT_FAIURE;
    }
    PRINT("Vulkan instanse created");

    return APP_CODE_OK;
}

AppResult VulkanApp::FindPhysicalDevice() {

    PhysDevList devices;
    GetPhysicalDevicesInfos(devices);

    // Filter appropriate devices
    std::vector<VkPhysicalDevice> unsuitableDevices;
    requiredParams.deviceFeatures.geometryShader = true;
    CheckSuitablePhysDevices(devices, unsuitableDevices);
    for (auto device : unsuitableDevices) {
        devices.erase(device);
    }

    // Prefer to use more or less efficient GPU
    for (auto d : devices) {
        if (d.second.properties.deviceType == 
#if POWER_SAVE
            VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
#else
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
#endif
        ) {
            physDev = d.first;
        }
    }

    if (physDev == VK_NULL_HANDLE) {
        physDev = devices.begin()->first;
    }

    if (physDev == VK_NULL_HANDLE) {
        PRINT_E("None of your GPUs is appropriate. You must buy an expensive cool adapter");
        return APP_CODE_VK_INIT_FAIURE;
    }

    physDevInfo = devices[physDev];

    PRINT("Using %s GPU: \"%s\"",
        (physDevInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        ? "discrete"
        : (physDevInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            ? "integrated"
            : (physDevInfo.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            ? "virtual"
            : "uncknown-type",
        physDevInfo.properties.deviceName);

    return APP_CODE_OK;
}

AppResult VulkanApp::CreateLogicalDevice() {

    // Create device queue family
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext            = nullptr;
    queueCreateInfo.flags            = 0;
    if (!physDevInfo.familiesIndicies.graphics.has_value()) {
        return APP_CODE_UNKNOWN;
    }
    queueCreateInfo.queueFamilyIndex = physDevInfo.familiesIndicies.graphics.value();
    queueCreateInfo.queueCount       = 1;
    float queuePriority = 1.0;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext                   = nullptr;
    deviceCreateInfo.flags                   = 0;
    deviceCreateInfo.pQueueCreateInfos       = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount    = 1;
    deviceCreateInfo.pEnabledFeatures        = &requiredParams.deviceFeatures;
    deviceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(requiredParams.deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = requiredParams.deviceExtensions.data();
#if VALIDATION_LAYERS_ENABLED
    deviceCreateInfo.enabledLayerCount       = static_cast<uint32_t>(requiredParams.validationLayers.size());
    deviceCreateInfo.ppEnabledLayerNames     = requiredParams.validationLayers.data();
#else
    deviceCreateInfo.enabledLayerCount       = 0;
#endif

    VkResult r = vkCreateDevice(physDev, &deviceCreateInfo, nullptr, &dev);
    if (r != VK_SUCCESS) {
        PRINT_E("Failed to create Vulkan device. Vk error code: %d", r);
        return APP_CODE_VK_COMMAND_FAIURE;
    }
    PRINT("Vulkan logical device created");

    return APP_CODE_OK;
}

AppResult VulkanApp::CheckSupportedInstanceExtensions(const VulkanApp::ExtensionsList& exts,
                                                      VulkanApp::ExtensionsList& unsupportedExts,
                                                      const char* layer) {

    unsupportedExts.clear();

    // Get supported extensios
    uint32_t extCount = 0;
    std::vector<VkExtensionProperties> extProps;
    vkEnumerateInstanceExtensionProperties(layer, &extCount, nullptr);
    extProps.resize(extCount);
    VkResult r = vkEnumerateInstanceExtensionProperties(layer, &extCount, extProps.data());
    if (r != VK_SUCCESS) {
        PRINT_E("Can't get supported extensions list. Vk error code: %d", r);
        return APP_CODE_VK_COMMAND_FAIURE;
    }

    // Check specified list
    for (auto ext : exts) {
        bool isStated = false;
        for (auto supportedExt : extProps) {
            if (ext && std::string_view(ext) == supportedExt.extensionName) {
                isStated = true;
                break;
            }
        }
        if (!isStated) {
            unsupportedExts.push_back(ext);
        }
    }

    return APP_CODE_OK;
}

AppResult VulkanApp::CheckSupportedInstanceLayers(const VulkanApp::LayersList& layers,
                                                  VulkanApp::LayersList& unsupportedLayers) {

    unsupportedLayers.clear();

    // Get supported extensios
    uint32_t layersCount = 0;
    std::vector<VkLayerProperties> layersProps;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    layersProps.resize(layersCount);
    VkResult r = vkEnumerateInstanceLayerProperties(&layersCount, layersProps.data());
    if (r != VK_SUCCESS) {
        PRINT_E("Can't get available layers list. Vk error code: %d", r);
        return APP_CODE_VK_COMMAND_FAIURE;
    }

    // Check specified list
    for (auto layer : layers) {
        bool isStated = false;
        for (auto supportedExt : layersProps) {
            if (layer && std::string_view(layer) == std::string_view(supportedExt.layerName)) {
                isStated = true;
                break;
            }
        }
        if (!isStated) {
            unsupportedLayers.push_back(layer);
        }
    }

    return APP_CODE_OK;
}

AppResult VulkanApp::GetPhysicalDevicesInfos(PhysDevList& physDevList) {

    uint32_t count = 0;
    std::vector<VkPhysicalDevice> devices;
    vkEnumeratePhysicalDevices(vkInst, &count, nullptr);
    if (!count) {
        PRINT_E("None of your graphics adapters support Vulkan");
        return APP_CODE_VK_COMMAND_FAIURE;
    }
    devices.resize(count);
    vkEnumeratePhysicalDevices(vkInst, &count, devices.data());

    for (auto& device : devices) {
        auto& devInfo = physDevList[device];

        vkGetPhysicalDeviceFeatures(device, &devInfo.features);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
        devInfo.familiesProps.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, devInfo.familiesProps.data());
        vkGetPhysicalDeviceProperties(device, &devInfo.properties);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        devInfo.familiesProps.resize(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, devInfo.extensions.data());
        // @remind can also get from layers
        vkGetPhysicalDeviceMemoryProperties(device, &devInfo.memoryProps);
        GetQueueFamIndicies(devInfo.familiesProps, devInfo.familiesIndicies);
    }

    return APP_CODE_OK;
}

void VulkanApp::CheckSuitablePhysDevices(const PhysDevList& devices,
                                         std::vector<VkPhysicalDevice>& unsuitableDevices) {

    unsuitableDevices.clear();

    for (auto device : devices) {
        bool hasGraphicsQFamily    = false;
        bool doesSupportFeatures   = false;
        bool doesSupportExtensions = false;

        // Check for support of graphics family
        hasGraphicsQFamily = device.second.familiesIndicies.graphics.has_value();

        // Check for support of features
        {
            auto& devFeatures = device.second.features;
            auto& features = requiredParams.deviceFeatures;
            doesSupportFeatures = 
                (features.robustBufferAccess)                      ? devFeatures.robustBufferAccess                      : true &&
                (features.fullDrawIndexUint32)                     ? devFeatures.fullDrawIndexUint32                     : true &&
                (features.imageCubeArray)                          ? devFeatures.imageCubeArray                          : true &&
                (features.independentBlend)                        ? devFeatures.independentBlend                        : true &&
                (features.geometryShader)                          ? devFeatures.geometryShader                          : true &&
                (features.tessellationShader)                      ? devFeatures.tessellationShader                      : true &&
                (features.sampleRateShading)                       ? devFeatures.sampleRateShading                       : true &&
                (features.dualSrcBlend)                            ? devFeatures.dualSrcBlend                            : true &&
                (features.logicOp)                                 ? devFeatures.logicOp                                 : true &&
                (features.multiDrawIndirect)                       ? devFeatures.multiDrawIndirect                       : true &&
                (features.drawIndirectFirstInstance)               ? devFeatures.drawIndirectFirstInstance               : true &&
                (features.depthClamp)                              ? devFeatures.depthClamp                              : true &&
                (features.depthBiasClamp)                          ? devFeatures.depthBiasClamp                          : true &&
                (features.fillModeNonSolid)                        ? devFeatures.fillModeNonSolid                        : true &&
                (features.depthBounds)                             ? devFeatures.depthBounds                             : true &&
                (features.wideLines)                               ? devFeatures.wideLines                               : true &&
                (features.largePoints)                             ? devFeatures.largePoints                             : true &&
                (features.alphaToOne)                              ? devFeatures.alphaToOne                              : true &&
                (features.multiViewport)                           ? devFeatures.multiViewport                           : true &&
                (features.samplerAnisotropy)                       ? devFeatures.samplerAnisotropy                       : true &&
                (features.textureCompressionETC2)                  ? devFeatures.textureCompressionETC2                  : true &&
                (features.textureCompressionASTC_LDR)              ? devFeatures.textureCompressionASTC_LDR              : true &&
                (features.textureCompressionBC)                    ? devFeatures.textureCompressionBC                    : true &&
                (features.occlusionQueryPrecise)                   ? devFeatures.occlusionQueryPrecise                   : true &&
                (features.pipelineStatisticsQuery)                 ? devFeatures.pipelineStatisticsQuery                 : true &&
                (features.vertexPipelineStoresAndAtomics)          ? devFeatures.vertexPipelineStoresAndAtomics          : true &&
                (features.fragmentStoresAndAtomics)                ? devFeatures.fragmentStoresAndAtomics                : true &&
                (features.shaderTessellationAndGeometryPointSize)  ? devFeatures.shaderTessellationAndGeometryPointSize  : true &&
                (features.shaderImageGatherExtended)               ? devFeatures.shaderImageGatherExtended               : true &&
                (features.shaderStorageImageExtendedFormats)       ? devFeatures.shaderStorageImageExtendedFormats       : true &&
                (features.shaderStorageImageMultisample)           ? devFeatures.shaderStorageImageMultisample           : true &&
                (features.shaderStorageImageReadWithoutFormat)     ? devFeatures.shaderStorageImageReadWithoutFormat     : true &&
                (features.shaderStorageImageWriteWithoutFormat)    ? devFeatures.shaderStorageImageWriteWithoutFormat    : true &&
                (features.shaderUniformBufferArrayDynamicIndexing) ? devFeatures.shaderUniformBufferArrayDynamicIndexing : true &&
                (features.shaderSampledImageArrayDynamicIndexing)  ? devFeatures.shaderSampledImageArrayDynamicIndexing  : true &&
                (features.shaderStorageBufferArrayDynamicIndexing) ? devFeatures.shaderStorageBufferArrayDynamicIndexing : true &&
                (features.shaderStorageImageArrayDynamicIndexing)  ? devFeatures.shaderStorageImageArrayDynamicIndexing  : true &&
                (features.shaderClipDistance)                      ? devFeatures.shaderClipDistance                      : true &&
                (features.shaderCullDistance)                      ? devFeatures.shaderCullDistance                      : true &&
                (features.shaderFloat64)                           ? devFeatures.shaderFloat64                           : true &&
                (features.shaderInt64)                             ? devFeatures.shaderInt64                             : true &&
                (features.shaderInt16)                             ? devFeatures.shaderInt16                             : true &&
                (features.shaderResourceResidency)                 ? devFeatures.shaderResourceResidency                 : true &&
                (features.shaderResourceMinLod)                    ? devFeatures.shaderResourceMinLod                    : true &&
                (features.sparseBinding)                           ? devFeatures.sparseBinding                           : true &&
                (features.sparseResidencyBuffer)                   ? devFeatures.sparseResidencyBuffer                   : true &&
                (features.sparseResidencyImage2D)                  ? devFeatures.sparseResidencyImage2D                  : true &&
                (features.sparseResidencyImage3D)                  ? devFeatures.sparseResidencyImage3D                  : true &&
                (features.sparseResidency2Samples)                 ? devFeatures.sparseResidency2Samples                 : true &&
                (features.sparseResidency4Samples)                 ? devFeatures.sparseResidency4Samples                 : true &&
                (features.sparseResidency8Samples)                 ? devFeatures.sparseResidency8Samples                 : true &&
                (features.sparseResidency16Samples)                ? devFeatures.sparseResidency16Samples                : true &&
                (features.sparseResidencyAliased)                  ? devFeatures.sparseResidencyAliased                  : true &&
                (features.variableMultisampleRate)                 ? devFeatures.variableMultisampleRate                 : true &&
                (features.inheritedQueries)                        ? devFeatures.inheritedQueries                        : true;
        }

        // @remind Check for support of device extensions
        doesSupportExtensions = true;

        if (!(doesSupportFeatures && hasGraphicsQFamily && doesSupportExtensions)) {
            unsuitableDevices.push_back(device.first);
        }
    }
}

void VulkanApp::GetQueueFamIndicies(const std::vector<VkQueueFamilyProperties>& familiesProps, QueueFamIndicies& indicies) {
    for (uint32_t i = 0u; i < familiesProps.size(); ++i) {
        auto& family = familiesProps[i];
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indicies.graphics = i;
            return;
        }
        // @remind think what to do with others
    }
}

void VulkanApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional 
}

void VulkanApp::setupDebugMessenger() {
#if VALIDATION_LAYERS_ENABLED
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);
    if (VkExt::CreateDebugUtilsMessengerEXT(vkInst, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw "failed to setup debug messenger";
    }
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData) {

    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
            PRINT_TAG_V(LOGS_LAYER_TAG, "%s", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
            PRINT_TAG_W(LOGS_LAYER_TAG, "%s", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
            PRINT_TAG_E(LOGS_LAYER_TAG, "%s", pCallbackData->pMessage);
        } break;
        default: {
            PRINT_TAG_V(LOGS_LAYER_TAG, "%s", pCallbackData->pMessage);
        } break;
    }

    return VK_FALSE;
}

AppResult VulkanApp::LoopFunc() {

    // Now do nothing
    return APP_CODE_OK;
}

void VulkanApp::Clear() {
    VkExt::DestroyDebugUtilsMessengerEXT(vkInst, debugMessenger, nullptr);
    vkDestroyDevice(dev, nullptr);
    vkDestroyInstance(vkInst, nullptr);
}
