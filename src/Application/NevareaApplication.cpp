#include "NevareaApplication.hpp"

namespace Nevarea {
	uint32_t window_width = 1280;
	uint32_t window_height = 720;
	const char* window_title = "Nevarea Renderer";

	void application_init(ApplicationState* app)
	{
		// init glfw
		glfwSetErrorCallback([](int error, const char* description) {
			std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
		});

		if (!window_system_init(&app->window, window_width, window_width, window_title)) {
			std::cerr << "Window failed to initialize!\n";
			return;
		}

		// init nevarea context
		VulkanContext vulkan_context{};
		vulkan_context_init(vulkan_context, &app->window);

		app->running = true;
	}

	void application_run(ApplicationState* app)
	{
		while (!window_system_should_close(&app->window) && app->running)
		{
			window_system_poll_events(&app->window);
		}
	}

	void application_shutdown(ApplicationState* app)
	{
		window_system_cleanup(&app->window);
	}
}