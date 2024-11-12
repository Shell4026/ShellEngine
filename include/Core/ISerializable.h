#pragma once

#include "nlohmann/json.hpp"

namespace sh::core
{
	using Json = nlohmann::json;

	class ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual auto Serialize() const -> Json = 0;
		virtual void Deserialize(const Json& json) = 0;
	};

	template<typename T>
	inline void SerializeProperty(core::Json& json, const std::string& key, const T& value)
	{
		json[key] = value;
	}
	template<typename T>
	inline void DeserializeProperty(const core::Json& json, const std::string& key, T& value)
	{
		if (json.contains(key))
			value = json.at(key).get<T>();
	}
}