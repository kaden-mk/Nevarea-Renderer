#pragma once

#include "Core.hpp"

#ifdef NEVAREA_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
#endif

namespace Nevarea {
	struct NvWinExtent {
		uint32_t width;
		uint32_t height;
	};

	struct NevareaWindowState {
		NvWinExtent get_extent() const { return { width, height }; }
		void update_extent(NvWinExtent extent) {
			width = extent.width;
			height = extent.height;
			framebuffer_resized = true;
		};

		void init(void* handle);

		#ifdef NEVAREA_PLATFORM_WINDOWS
			HWND get_hwnd() { return hwnd; }
			HINSTANCE get_hinstance() { return hinstance; }
		#endif

		private:
			uint32_t width;
			uint32_t height;
			bool framebuffer_resized = false;

			#ifdef NEVAREA_PLATFORM_WINDOWS
				HWND hwnd;
				HINSTANCE hinstance;
			#endif
	};

	NEVAREA_FORCE_INLINE void window_system_wait_events();
}