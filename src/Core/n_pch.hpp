#pragma once

#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <functional>
#include <limits>

#include "lib/Core.hpp"

#ifdef NEVAREA_PLATFORM_WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
