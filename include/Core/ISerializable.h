#pragma once

#include "nlohmann/json.hpp"

namespace sh::core
{
	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual auto Serialize() const -> nlohmann::json = 0;
		virtual void Deserialize() = 0;
	};
}