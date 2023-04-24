#pragma once

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <app_result.h>

#include <optional>
#include <vector>

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

    AppResult FindUnsupportedExtensions(const ExtensionsList& exts, ExtensionsList& unsupportedExts);
    AppResult FindUnsupportedLayers(const LayersList& layers, LayersList& unsupportedLayers);
    void FindUnsuitablePhysDevices(const std::vector<VkPhysicalDevice>& devices, std::vector<VkPhysicalDevice>& unsuitableDevices);


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
