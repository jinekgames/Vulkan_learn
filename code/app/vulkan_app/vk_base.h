#pragma once

#if defined(WIN32) || defined(LINUX) || defined(MAC_OS)
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <vulkan/vulkan.h>
#endif
