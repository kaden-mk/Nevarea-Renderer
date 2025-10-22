#include <iostream>

#include <Application/NevareaApplication.hpp>

using namespace Nevaera;

int main() {
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
	});

	ApplicationState app{};
	application_init(&app);
	application_run(&app);
	application_shutdown(&app);

	return 0;
}