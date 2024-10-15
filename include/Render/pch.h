#pragma once

#include <glm/glm.hpp>

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

#include "Export.h"

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <cassert>
#include <stdexcept>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <initializer_list>
#include <array>
#include <chrono>

#undef min
#undef max