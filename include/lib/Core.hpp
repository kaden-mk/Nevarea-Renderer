#pragma once

#if defined(_MSC_VER)
	#define NEVAREA_FORCE_INLINE __forceinline
	#define NEVAREA_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
	#define NEVAREA_FORCE_INLINE inline __attribute__((always_inline))
	#define NEVAREA_BREAK() __builtin_trap()
#else
	#define NEVAREA_FORCE_INLINE inline
	#define NEVAREA_BREAK()
#endif

#ifndef NEVAREA_DEBUG
	#define NEVAREA_ASSERT(expr, context, message) ((void)0)
#else
	#define NEVAREA_ASSERT(expr, context, message) \
		if (!(expr)) { \
			std::cerr << "[NEVAREA ASSERT]: [" << (context) << "] " << (message) << std::endl; \
			std::cerr << "File: " << __FILE__ << " Line: " << __LINE__ << std::endl; \
			NEVAREA_BREAK(); \
		}
#endif

#if defined(_WIN32)
	#define NEVAREA_PLATFORM_WINDOWS
#elif defined(__linux__)
	#define NEVAREA_PLATFORM_LINUX
#elif defined(__APPLE__)
	#define NEVAREA_PLATFORM_APPLE
#endif

#ifdef NEVAREA_PLATFORM_WINDOWS
	#ifdef NEVAREA_BUILD_DLL
		#define NEVAREA_API __declspec(dllexport)
	#else
		#define NEVAREA_API __declspec(dllimport)
	#endif
#else
	#if __GNUC__ >= 4
		#define NEVAREA_API __attribute__ ((visibility ("default")))
	#else
		#define NEVAREA_API
	#endif
#endif