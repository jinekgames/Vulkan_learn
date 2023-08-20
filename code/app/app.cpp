#include <app.h>
#include <app_consts.h>

#include <logs.h>

#include <algorithm>
#include <map>
#include <string_view>

App::~App() {
    Clear();
}

App& App::Inst() {

    static App inst;
    return inst;
}

AppResult App::Run() {

    AppResult r = APP_CODE_OK;

    r = Init();
    if (!APP_CHECK_RESULT(r)) {
        Clear();
        return r;
    }
    r = Loop();
    Clear();
    return r;
}

AppResult App::Init() {

    AppResult r = APP_CODE_OK;

    r = InitWindow();
    if(!APP_CHECK_RESULT(r)) {
        return r;
    }

    r = VulkanApp::Init();
    if(!APP_CHECK_RESULT(r)) {
        return r;
    }

    return r;
}

AppResult App::InitWindow() {

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    wnd = glfwCreateWindow(
        APP_DEFAULT_WINDOW_WIDTH,
        APP_DEFAULT_WINDOW_HEIGHT,
        APP_NAME,
        nullptr, nullptr
    );

    if (!wnd) {
        PRINT_E("Failed to create window");
        return APP_CODE_WND_INIT_FAIURE;
    }
    PRINT("Window created");
    return APP_CODE_OK;
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif
}

AppResult App::Loop() {

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    while(!glfwWindowShouldClose(wnd)) {
        glfwPollEvents();
        LoopFunc();
    }
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif
    return APP_CODE_OK;
}

void App::Clear() {

    VulkanApp::Clear();

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    glfwDestroyWindow(wnd);
    glfwTerminate();
#endif
}
