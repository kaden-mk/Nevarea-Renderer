#include "WindowSystem.hpp"

namespace Nevaera {
	bool window_system_init(WindowSystemState* state, uint32_t width, uint32_t height, const char* title)
	{
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (!window) {
			std::cerr << "Failed to create GLFW window\n";
			glfwTerminate();
			return false;
		}

		state->window = window;
		state->width = width;
		state->height = height;

		return true;
	}

	bool window_system_should_close(WindowSystemState* state)
	{
		return glfwWindowShouldClose(state->window);
	}

	void window_system_poll_events(WindowSystemState* state)
	{
		glfwPollEvents();
	}

	void window_system_cleanup(WindowSystemState* state)
	{
		glfwDestroyWindow(state->window);
		glfwTerminate();
	}
}