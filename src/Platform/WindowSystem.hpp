#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Core/n_pch.hpp"

namespace Nevarea {
	struct WindowSystemState {
		GLFWwindow* window;
		uint32_t width;
		uint32_t height;
		bool framebuffer_resized = false;

		VkExtent2D get_extent() { return { width, height }; }
	};

	bool window_system_init(WindowSystemState* state, uint32_t width, uint32_t height, const char* title);
	bool window_system_should_close(WindowSystemState* state);

	void window_system_wait_events();
	void window_system_poll_events();
	void window_system_cleanup(WindowSystemState* state);
	void window_system_create_surface(WindowSystemState* window, VkInstance instance, VkSurfaceKHR* surface);
}