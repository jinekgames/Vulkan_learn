#pragma once

#include <array>


// @todo move some of this options to CMake or to cli options


// Default window size

#define APP_DEFAULT_WINDOW_WIDTH  800
#define APP_DEFAULT_WINDOW_HEIGHT 600


// Application name

#define APP_NAME "Vulkan prog"


// Vulkan validation layers' consts

#define VALIDATION_LAYERS_ENABLED 1
constexpr size_t vulkanValidationLayersCount = 2;
constexpr std::array<const char*, vulkanValidationLayersCount> vulkanValidationLayers{
    "VK_LAYER_KHRONOS_validation",
    "RenderDoc_Vulkan_GLES_Layer",
};


// If POWER_SAVE is true an integrated GPU will be used. If false, then discrete
#define POWER_SAVE 1


// Enables logs if true
#define DEBUG_LOGS 1
