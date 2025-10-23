#include <iostream>

#include <Application/NevareaApplication.hpp>
#define NEVAREA_DEBUG // enable validation layers

using namespace Nevarea;

int main() {
	ApplicationState app{};
	application_init(&app);
	application_run(&app);
	application_shutdown(&app);

	return 0;
}