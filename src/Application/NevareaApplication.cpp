#include "NevareaApplication.hpp"

namespace Nevaera {
	uint32_t window_width = 1280;
	uint32_t window_height = 720;
	const char* window_title = "Nevarea Renderer";

	void Nevaera::application_init(ApplicationState* app)
	{
		if (!window_system_init(&app->window, window_width, window_width, window_title)) {
			std::cerr << "Window failed to initialize!\n";
			return;
		}

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