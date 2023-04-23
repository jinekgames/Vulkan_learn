project(Vulkan_main)

include_directories(
	app/
	logs/
	${GLM_DIR}
	${GLFW_DIR}/include
	${VULKAN_DIR}/Include
)

set(SOURCE
	prog.cpp
	app/app.h
	app/app.cpp
)

add_executable(hello
	${SOURCE}
)

add_subdirectory(${GLFW_DIR} ${GLFW_OUT})

target_link_libraries(hello glfw)

find_package(Vulkan REQUIRED)
target_link_libraries(hello ${Vulkan_LIBRARIES})
