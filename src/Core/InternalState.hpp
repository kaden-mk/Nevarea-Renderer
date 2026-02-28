#pragma once

#include "Nevarea/Config.hpp"

namespace Nevarea::Internal {
    void set_global_config(const EngineConfig& config);

    const WindowConfig& get_window_config();
    const RendererConfig& get_renderer_config();
}