#pragma once

#include <app_result.h>
#include <logs.h>
#include <vulkan_app/vk_base.h>

#include <map>
#include <optional>
#include <vector>

class VulkanApp {

public:

// Main Private methods
protected:

    virtual AppResult Init();
    AppResult LoopFunc();
    virtual void Clear();

// App init Private methods
private:

    AppResult CreateVkInstance();
    AppResult FindPhysicalDevice();
    AppResult CreateLogicalDevice();

    typedef std::vector<const char*> NamesList;

    // Vulkan extensions' list
    typedef NamesList ExtensionsList;
    // Vulkan layers' list
    typedef NamesList LayersList;

    /**
     * @brief
     * Check if specified extensions are supported by the layer or instance
     * @param exts
     * extensions to be checked
     * @param unsupportedExts
     * extensionts that ill not be supported
     * @param layer
     * set or nullptr to check from instance, eigther c-string with name of layer to check from layer
     * @return
     * AppResult code
    */
    AppResult CheckSupportedInstanceExtensions(const ExtensionsList& exts, ExtensionsList& unsupportedExts,
                                               const char* layer = nullptr);
    AppResult CheckSupportedInstanceLayers(const LayersList& layers, LayersList& unsupportedLayers);

    struct QueueFamIndicies {
        std::optional<uint32_t> graphics;
        // std::optional<uint32_t> compute;
        // std::optional<uint32_t> transfer;
        // std::optional<uint32_t> sparseBinding;
        // std::optional<uint32_t> videoEncode;
        // std::optional<uint32_t> opticalFlow;
    };

    struct PhysDevInfo {
        std::vector<VkExtensionProperties> extensions;
        std::vector<VkQueueFamilyProperties> familiesProps;
        QueueFamIndicies familiesIndicies;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memoryProps;
        VkPhysicalDeviceProperties properties;
    };

    typedef std::map<VkPhysicalDevice, PhysDevInfo> PhysDevList;

    AppResult GetPhysicalDevicesInfos(PhysDevList& physDevList);
    /**
     * @brief
     * Check if physical evices support specified features and have a graphics family
     * @param devices
     * list of devices with treir properties
     * @param unsuitableDevices
     * list of unsuitable device handles
     * @param features
     * features to check
    */
    void CheckSuitablePhysDevices(const PhysDevList& devices, std::vector<VkPhysicalDevice>& unsuitableDevices);
    static void GetQueueFamIndicies(const std::vector<VkQueueFamilyProperties>& familiesProps,
                                    QueueFamIndicies& indicies);


    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    // Vk debug callback
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void* pUserData);


private:

class VkExt {

    friend class VulkanApp;

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

};


// Vulkan objects
private:

    VkInstance vkInst;
    VkPhysicalDevice physDev;
    PhysDevInfo physDevInfo;
    VkDevice dev;

    struct RequiredParams {
        ExtensionsList instanseExtensions;
        ExtensionsList deviceExtensions;
        VkPhysicalDeviceFeatures deviceFeatures;
        LayersList validationLayers;
    } requiredParams;

    VkDebugUtilsMessengerEXT debugMessenger;


// friend class App;

// Singleton realisation
protected:
    VulkanApp();
    VulkanApp(const VulkanApp&) = delete;
    VulkanApp(const VulkanApp&&) = delete;
    ~VulkanApp();
};
