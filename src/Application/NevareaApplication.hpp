#pragma once

#include <iostream>
#include <Platform/WindowSystem.hpp>
#include <Renderer/Vulkan/VulkanContext.hpp>

namespace Nevarea {
	struct ApplicationState {
		WindowSystemState window;
		VulkanContext vulkan_context;
		bool running;
	};

	void application_init(ApplicationState* app);
	void application_run(ApplicationState* app);
	void application_shutdown(ApplicationState* app);

	extern uint32_t window_width;
	extern uint32_t window_height;
	extern const char* window_title;
}