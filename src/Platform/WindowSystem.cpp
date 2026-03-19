#include "lib/WindowSystem.hpp"

#include "Core/InternalState.hpp"
#include "Core/n_pch.hpp"

namespace Nevarea {
	void NevareaWindowState::init(void* handle) {
		#ifdef NEVAREA_PLATFORM_WINDOWS
			this->hwnd = (HWND)handle;
			this->hinstance = (HINSTANCE)GetWindowLongPtr(this->hwnd, GWLP_HINSTANCE);

			RECT rect;
			if (GetClientRect(this->hwnd, &rect)) {
				this->width = rect.right - rect.left;
				this->height = rect.bottom - rect.top;
			}
		#endif

		this->framebuffer_resized = false;
	}

	void window_system_wait_events() {
		#ifdef NEVAREA_PLATFORM_WINDOWS
			MSG msg;
			if (GetMessage(&msg, NULL, 0, 0)) {
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		#endif
	}
}