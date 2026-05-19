#pragma once

#include "Core.hpp"

#include <cstdint>

namespace Nevarea {
	enum class WindowHandle : uint32_t { INVALID = 0 };

	struct NvWinExtent {
		uint32_t width;
		uint32_t height;
	};

	WindowHandle window_create(void* native_handle);
	void window_destroy(WindowHandle window);

	NvWinExtent window_get_extent(WindowHandle window);
	void window_set_extent(WindowHandle window, NvWinExtent extent);

	void window_system_wait_events();
}