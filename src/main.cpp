#include <iostream>

#include <Application/NevareaApplication.hpp>

using namespace Nevarea;

int main() {
	ApplicationState app{};
	application_init(&app);
	application_run(&app);
	application_shutdown(&app);

	return 0;
}