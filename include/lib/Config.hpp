#pragma once

#include <stdint.h>

namespace Nevarea {
    // initial window state (should probably add some stuff like can_resize?)
    struct WindowConfig {
        uint32_t width = 1280;
        uint32_t height = 720;
        const char* title = "Nevarea Renderer";
        bool fullscreen = false;
    };

    struct RendererConfig {
        uint32_t max_frames_in_flight = 2;
        bool enable_validation_layers = true;
    };

    struct EngineConfig {
        WindowConfig window;
        RendererConfig renderer;
    };
    
    void init_config(const EngineConfig& config);
}