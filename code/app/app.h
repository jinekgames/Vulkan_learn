#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

typedef int AppResult;

class App {

// AppResult codes

#define APP_CODE_OK               0
#define APP_CODE_UNSUPPORTED_OS   1
#define APP_CODE_WND_INIT_FAIURE  2
#define APP_CODE_VK_INIT_FAIURE   3
#define APP_CODE_DEV_ENUM_FAILED  4
#define APP_CODE_UNKNOWN         ~0

#define CHECK_RESULT(res) (res == APP_CODE_OK)


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


private:

    GLFWwindow* wnd = nullptr;

    VkInstance vkInst;


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
