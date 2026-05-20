#include "InternalState.hpp"
#include "lib/Config.hpp"

#include "n_pch.hpp"

namespace {
	Nevarea::EngineConfig g_engine_config{};
	bool g_is_initialized = false;
}

namespace Nevarea {
    void init_config(const EngineConfig& config) {
        Internal::set_global_config(config);
    }
}

namespace Nevarea::Internal {
    void set_global_config(const EngineConfig& config) {
        if (g_is_initialized) throw std::runtime_error("Config already initialized!");

        g_engine_config = config;
        g_is_initialized = true;
    }

    const RendererConfig& get_renderer_config() { return g_engine_config.renderer; }
}