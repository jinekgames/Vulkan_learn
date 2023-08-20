#pragma once

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <app_result.h>

#include <vector>
#include <map>

class VulkanApp {

public:

typedef std::vector<const char*> NamesList;

// Vulkan extinsions' list
typedef NamesList ExtensionsList;
// Vulkan layers' list
typedef NamesList LayersList;


// Main Private methods
protected:

    virtual AppResult Init();
    AppResult LoopFunc();
    virtual void Clear();

// App init Private methods
private:

    AppResult CreateVkInstance();
    AppResult FindPhysicalDevice();

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

    struct PhysDevInfo {
        VkPhysicalDeviceFeatures features;
        std::vector<VkQueueFamilyProperties> familiesProps;
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
    void CheckSuitablePhysDevices(const PhysDevList& devices, std::vector<VkPhysicalDevice>& unsuitableDevices,
                                  const VkPhysicalDeviceFeatures& features);


// Vulkan objects
private:

    VkInstance vkInst;
    VkPhysicalDevice physDev;
    VkDevice dev;


// friend class App;

// Singleton realisation
protected:
    VulkanApp() {}
    VulkanApp(const VulkanApp&) = delete;
    VulkanApp(const VulkanApp&&) = delete;
    ~VulkanApp();
};
