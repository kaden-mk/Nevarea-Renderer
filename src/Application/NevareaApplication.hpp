#pragma once

#include <iostream>
#include <Platform/WindowSystem.hpp>

namespace Nevaera {
	struct ApplicationState {
		WindowSystemState window;
		bool running;
	};

	void application_init(ApplicationState* app);
	void application_run(ApplicationState* app);
	void application_shutdown(ApplicationState* app);

	extern uint32_t window_width;
	extern uint32_t window_height;
	extern const char* window_title;
}