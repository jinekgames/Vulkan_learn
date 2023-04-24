#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

typedef int AppResult;

class App {

public:

// AppResult codes

#define APP_CODE_OK                 0
#define APP_CODE_UNSUPPORTED_OS     1
#define APP_CODE_WND_INIT_FAIURE    2
#define APP_CODE_VK_INIT_FAIURE     3
#define APP_CODE_VK_COMMAND_FAIURE  4
#define APP_CODE_DEV_ENUM_FAILED    5
#define APP_CODE_UNKNOWN           ~0

#define CHECK_RESULT(res) (res == APP_CODE_OK)


typedef std::vector<const char*> NamesList;
// Vulkan extinsions' list
typedef NamesList ExtensionsList;
// Vulkan layers' list
typedef NamesList LayersList;


// Public methods
public:

    AppResult Run();

// Main Private methods
private:

    AppResult Init();
    AppResult Loop();
    void Clear();

// App init Private methods
private:

    AppResult InitWindow();
    AppResult InitVk();
    AppResult FindUnsupportedExtensions(const ExtensionsList& exts, ExtensionsList& unsupportedExts);
    AppResult FindUnsupportedLayers(const LayersList& layers, LayersList& unsupportedLayers);
    void FindUnsuitablePhysDevices(const std::vector<VkPhysicalDevice>& devices, std::vector<VkPhysicalDevice>& unsuitableDevices);


// Window objects
private:

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    GLFWwindow* wnd = nullptr;
#endif

// Vulkan objects
private:

    VkInstance vkInst;
    VkPhysicalDevice physDev;


// Singleton realisation
private:
    App() {}
    App(const App&) {}
    App(const App&&) {}
    App& operator = (App&) {}
    App&& operator = (App&&) {}
    ~App();
public:
    static App& Inst();
};
