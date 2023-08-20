#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <app_result.h>
#include <vulkan_app/vulkan_app.h>

#include <optional>
#include <vector>

class App final : public VulkanApp {

// Public methods
public:

    AppResult Run();

// Main Private methods
private:

    AppResult Init() override;
    AppResult Loop();
    void Clear() override;

// App init Private methods
private:

    AppResult InitWindow();


// Window objects
private:

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    GLFWwindow* wnd = nullptr;
#endif


// Singleton realisation
private:
    App() {}
    App(const App&) = delete;
    App(const App&&) = delete;
    ~App();
public:
    static App& Inst();
};
