#pragma once

#if _WIN32
	#ifndef VK_USE_PLATFORM_WIN32_KHR
	#define VK_USE_PLATFORM_WIN32_KHR
	#endif
#elif __unix__
	#ifndef VK_USE_PLATFORM_XLIB_KHR
	#define VK_USE_PLATFORM_XLIB_KHR
	#endif
#endif
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#ifndef VMA_VULKAN_VERSION
#define VMA_VULKAN_VERSION 1001000
#endif

#include "../vma-src/include/vk_mem_alloc.h"