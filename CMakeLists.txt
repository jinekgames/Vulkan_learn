cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project(Vulkan_learn)


set(BINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/bins)
message("Bins will be written to: \"${BINS_DIR}\"")


# @todo unused
set(VULKAN_DIR "")
if (NOT "$ENV{VK_SDK_PATH}" STREQUAL "")
	set(VULKAN_DIR "$ENV{VK_SDK_PATH}")
elseif(NOT "$ENV{VULKAN_SDK}" STREQUAL "")
	set(VULKAN_DIR "$ENV{VULKAN_SDK}")
else()
	message(SEND_ERROR "Vulkan SDK dir is not specified. Set it to VK_SDK_PATH or VULKAN_SDK env vatiable")
endif()
message("Vulkan found in \"${VULKAN_DIR}\"")


set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/code)
set(SUBMODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules)

set(GLFW_DIR ${SUBMODULES_DIR}/glfw)
set(GLFW_OUT ${BINS_DIR}/glfw)

set(GLM_DIR ${SUBMODULES_DIR}/glm)

set(FINAL_OUT ${BINS_DIR}/final_package)


add_subdirectory(${CODE_DIR} ${FINAL_OUT})
