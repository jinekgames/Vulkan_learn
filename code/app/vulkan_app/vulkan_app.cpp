#include <vulkan_app/vulkan_app.h>
#include <app_consts.h>

#include <logs.h>

#include <algorithm>
#include <map>
#include <string_view>

VulkanApp::~VulkanApp() {
    Clear();
}

AppResult VulkanApp::Init() {

    // Create Vk instanse
    APP_CHECK_CALL(CreateVkInstance());

    // Find physical device
    APP_CHECK_CALL(FindPhysicalDevice());

    // Create logical device
    

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

    // Get required Vk extensions
    ExtensionsList extensions;
#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    {
        uint32_t extensionCount = 0;
        glfwGetRequiredInstanceExtensions(&extensionCount);
        const char** glfwExts = glfwGetRequiredInstanceExtensions(&extensionCount);
        extensions.resize(extensionCount);
        for (size_t i = 0; i < extensionCount; ++i) {
            extensions[i] = glfwExts[i];
        }
#if defined(MAC_OS)
        ++extensionCount;
        extensions.push_back("VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME");
#endif
    }
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif

    // Check for extensions support
    {
        ExtensionsList unsupportedExts{};
        AppResult r = FindUnsupportedExtensions(extensions, unsupportedExts);
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

    LayersList validationLayers{};
#if VALIDATION_LAYERS_ENABLED
    validationLayers.reserve(vulkanValidationLayers.size());
    for (auto layer : vulkanValidationLayers) {
        validationLayers.push_back(layer);
    }
    // Check for layers available
    {
        LayersList unsupportedLayers{};
        AppResult r = FindUnsupportedLayers(validationLayers, unsupportedLayers);
        if (APP_CHECK_RESULT(r)) {
            if (unsupportedLayers.empty()) {
                PRINT("All requested Vulkan validation layers are supported");
            } else {
                PRINT_W("Following Vulkan validation layers couldn't be registered. They will be skipped");
                for (auto layer : unsupportedLayers) {
                    PRINT_W(" - %s", layer);
                    validationLayers.erase(std::find(validationLayers.begin(), validationLayers.end(), layer));
                }
            }
        } else {
            return APP_CODE_VK_INIT_FAIURE;
        }
    }
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo         = &appInfo;
    createInfo.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames  = extensions.data();
#if VALIDATION_LAYERS_ENABLED
    createInfo.enabledLayerCount        = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames      = validationLayers.data();
#else
    createInfo.enabledLayerCount        = 0;
#endif
#if defined(MAC_OS)
    createInfo.flags                   |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    // @todo Setup debug messenger


    VkResult r = vkCreateInstance(&createInfo, nullptr, &vkInst);
    if (r != VK_SUCCESS) {
        PRINT_E("Failed to create Vulkan instanse. Vk error code: %d", r);
        return APP_CODE_VK_INIT_FAIURE;
    }
    PRINT("Vulkan instanse created");

    return APP_CODE_OK;
}

AppResult VulkanApp::FindPhysicalDevice() {

    uint32_t deviceCount = 0;
    std::vector<VkPhysicalDevice> devices;
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, nullptr);
    if (!deviceCount) {
        PRINT_E("None of your graphics adapters support Vulkan");
        return APP_CODE_VK_INIT_FAIURE;
    }
    devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(vkInst, &deviceCount, devices.data());

    // Filter appropriate devices
    std::vector<VkPhysicalDevice> unsuitableDevices;
    FindUnsuitablePhysDevices(devices, unsuitableDevices);
    for (auto device : unsuitableDevices) {
        devices.erase(std::find(devices.begin(), devices.end(), device));
    }

    std::map<VkPhysicalDevice, VkPhysicalDeviceProperties> devicesWhithProps;
    for (auto device : devices) {
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(device, &prop);
        devicesWhithProps.emplace(device, prop);
    }

    // Prefer to use more or less efficient GPU
    for (auto d : devicesWhithProps) {
        if (d.second.deviceType == 
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
        physDev = devicesWhithProps.begin()->first;
    }

    if (physDev == VK_NULL_HANDLE) {
        PRINT_E("None of your GPUs is appropriate. You must buy an expensive cool adapter");
        return APP_CODE_VK_INIT_FAIURE;
    }

    VkPhysicalDeviceProperties seletedDevProps = devicesWhithProps.at(physDev);
    PRINT("Using %s GPU: \"%s\"",
        (seletedDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        ? "discrete"
        : (seletedDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            ? "integrated"
            : (seletedDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            ? "virtual"
            : "uncknown-type",
        seletedDevProps.deviceName);

    return APP_CODE_OK;
}

AppResult VulkanApp::FindUnsupportedExtensions(const VulkanApp::ExtensionsList& exts, VulkanApp::ExtensionsList& unsupportedExts) {

    unsupportedExts.clear();

    // Get supported extensios
    uint32_t extCount = 0;
    std::vector<VkExtensionProperties> extProps;
    vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
    extProps.resize(extCount);
    VkResult r = vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProps.data());
    if (r != VK_SUCCESS) {
        PRINT_E("Can't get supported extensions list. Vk error code: %d", r);
        return APP_CODE_VK_COMMAND_FAIURE;
    }

    // Check specified list
    for (auto ext : exts) {
        bool isStated = false;
        for (auto supportedExt : extProps) {
            if (ext && std::string_view(ext) == std::string_view(supportedExt.extensionName)) {
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

AppResult VulkanApp::FindUnsupportedLayers(const VulkanApp::LayersList& layers, VulkanApp::LayersList& unsupportedLayers) {

    unsupportedLayers.clear();

    // Get supported extensios
    uint32_t layersCount = 0;
    std::vector<VkLayerProperties> layersProps;
    vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    layersProps.resize(layersCount);
    VkResult r = vkEnumerateInstanceLayerProperties(&layersCount, layersProps.data());
    if (r != VK_SUCCESS) {
        PRINT_E("Can't get supported extensions list. Vk error code: %d", r);
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

void VulkanApp::FindUnsuitablePhysDevices(const std::vector<VkPhysicalDevice>& devices, std::vector<VkPhysicalDevice>& unsuitableDevices) {

    unsuitableDevices.clear();

    for (auto device : devices) {

        VkPhysicalDeviceFeatures features{};
        vkGetPhysicalDeviceFeatures(device, &features);
        bool doesSupportFeatures = features.geometryShader;

        bool hasGraphicsQFamily = false;
        uint32_t familyCount = 0;
        std::vector<VkQueueFamilyProperties> families;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
        families.resize(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
        for (auto family : families) {
            if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                hasGraphicsQFamily = true;
                break;
            }
        }

        if (!(doesSupportFeatures && hasGraphicsQFamily)) {
            unsuitableDevices.push_back(device);
        }
    }
}

AppResult VulkanApp::LoopFunc() {

    // Now do nothing
    return APP_CODE_OK;
}

void VulkanApp::Clear() {

    vkDestroyInstance(vkInst, nullptr);
}
