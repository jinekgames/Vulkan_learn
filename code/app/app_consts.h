#pragma once

#include <array>


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

#define POWER_SAVE 0
