#pragma once

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __unix__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

#define VMA_VULKAN_VERSION 1003000
#include "../vma-src/include/vk_mem_alloc.h"