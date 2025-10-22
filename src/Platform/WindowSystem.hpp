#pragma once

#include <iostream>

#include <GLFW/glfw3.h>

namespace Nevaera {
	struct WindowSystemState {
		GLFWwindow* window;
		uint32_t width;
		uint32_t height;
		bool framebuffer_resized = false;
	};

	bool window_system_init(WindowSystemState* state, uint32_t width, uint32_t height, const char* title);
	bool window_system_should_close(WindowSystemState* state);

	void window_system_poll_events(WindowSystemState* state);
	void window_system_cleanup(WindowSystemState* state);
}