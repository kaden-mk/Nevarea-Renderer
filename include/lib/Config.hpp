#pragma once

#include <stdint.h>

namespace Nevarea {
    struct RendererConfig {
        bool enable_validation_layers = true;
    };

    struct EngineConfig {
        RendererConfig renderer;
    };
    
    void init_config(const EngineConfig& config);
}