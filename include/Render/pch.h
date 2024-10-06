#pragma once

#include "Core/SContainer.hpp"
#include "Core/Logger.h"
#include "Core/Singleton.hpp"

#include <glm/glm.hpp>
#include "VulkanImpl/VulkanConfig.h"

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

#undef min
#undef max