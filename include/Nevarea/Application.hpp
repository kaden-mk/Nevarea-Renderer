#pragma once

#include <iostream>
#include <Nevarea/Config.hpp>

namespace Nevarea {
	struct ApplicationState;

	struct Application {
		ApplicationState* state;
	};

	void application_init(Application& app);
	void application_run(Application& app);
	void application_shutdown(Application& app);
}