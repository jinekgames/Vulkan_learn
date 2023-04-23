#include <app.h>
#include <app_consts.h>

#include <logs.h>

#include <vector>

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
    if (!CHECK_RESULT(r)) {
        Clear();
        return r;
    }
    r = Loop();
    if (!CHECK_RESULT(r)) {
        Clear();
        return r;
    }
    Clear();
    return APP_CODE_OK;
}

AppResult App::Init() {

    AppResult r = APP_CODE_OK;

    r = InitWindow();
    if(!CHECK_RESULT(r)) {
        return r;
    }

    r = InitVk();
    if(!CHECK_RESULT(r)) {
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

    if (wnd) {
        PRINT("Window created");
        return APP_CODE_OK;
    }
    PRINT_E("Failed to create window");
    return APP_CODE_WND_INIT_FAIURE;
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif
}

AppResult App::InitVk() {

    VkResult r = VK_SUCCESS;

    // Create Vk instanse

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = APP_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Get required Vk extensions
#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
#else
    PRINT_E("Your OS is not supported yet");
    return APP_CODE_UNSUPPORTED_OS;
#endif

    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount       = 0;

    r = vkCreateInstance(&createInfo, nullptr, &vkInst);
    if (r != VK_SUCCESS) {
        PRINT_E("Failed to create Vulkan instanse");
        return APP_CODE_VK_INIT_FAIURE;
    }
    PRINT("Vulkan instanse created");

    uint32_t extensionCount = 0;
    std::vector<VkExtensionProperties> extProps;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    extProps.resize(extensionCount);
    r = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extProps.data());

    if (r == VK_SUCCESS) {
        PRINT("%lu Vulkan extensions supported. Namely:", extensionCount);
        for (auto ext : extProps) {
            PRINT("%s", ext.extensionName);
        }
    } else {
        PRINT_E("Failed to enumerate exts. code: %d", r);
        return APP_CODE_DEV_ENUM_FAILED;
    }

    return APP_CODE_OK;
}

AppResult App::Loop() {

    while(!glfwWindowShouldClose(wnd)) {
        glfwPollEvents();
    }
    return APP_CODE_OK;
}

void App::Clear() {

    glfwDestroyWindow(wnd);
    glfwTerminate();
}
