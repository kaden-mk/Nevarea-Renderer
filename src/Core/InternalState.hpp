#pragma once

#include "lib/Config.hpp"
#include "lib/Core.hpp"

namespace Nevarea::Internal {
	void set_global_config(const EngineConfig& config);

	const RendererConfig& get_renderer_config();
}