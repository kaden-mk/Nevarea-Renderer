#pragma once

#include "Core.hpp"

#include <cstdint>

#ifdef NEVAREA_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
#endif

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

	#ifdef NEVAREA_PLATFORM_WINDOWS
		HWND window_get_hwnd(WindowHandle window);
		HINSTANCE window_get_hinstance(WindowHandle window);
	#endif

	NEVAREA_FORCE_INLINE void window_system_wait_events();
}