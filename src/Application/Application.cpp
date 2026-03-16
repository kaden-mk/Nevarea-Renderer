#include "lib/Application.hpp"
#include "Core/InternalState.hpp"

#include <Platform/WindowSystem.hpp>
#include <Renderer/Vulkan/VulkanContext.hpp>

namespace Nevarea {
    struct ApplicationState {
        WindowSystemState window;
        Nevarea::Renderer::VulkanContext vulkan_context;
        bool running = false;
    };

    void application_init(Application &app) {
        app.state = new ApplicationState{};

        // init glfw
        glfwSetErrorCallback([](int error, const char *description) {
            std::cerr << "GLFW Error [" << error << "]: " << description << std::endl;
        });

        WindowConfig config = Internal::get_window_config();

        if (!window_system_init(&app.state->window, config.width, config.height, config.title)) {
            std::cerr << "Window failed to initialize!\n";
            return;
        }

        // init vulkan context
        vulkan_context_init(app.state->vulkan_context, &app.state->window);

        app.state->running = true;
    }

    void application_run(Application &app) {
        if (!app.state) return;

        while (!window_system_should_close(&app.state->window) && app.state->running) {
            vulkan_context_draw(app.state->vulkan_context);
            window_system_poll_events();
        }
    }

    void application_shutdown(Application &app) {
        if (!app.state) return;

        vulkan_context_destroy(app.state->vulkan_context);
        window_system_cleanup(&app.state->window);

        delete app.state;
        app.state = nullptr;
    }
}