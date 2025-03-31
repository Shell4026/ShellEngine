#pragma once
#include "Export.h"

#include "Core/SObject.h"

#include "glm/mat4x4.hpp"

#include <string>
#include <unordered_map>
#include <optional>

namespace sh::render
{
	class Texture;

	/// @brief 메테리얼에서 프로퍼티 값을 보관하는 객체
	class MaterialPropertyBlock : public core::SObject
	{
		SCLASS(MaterialPropertyBlock)
	private:
		std::unordered_map<std::string, float> scalarProperties;
		std::unordered_map<std::string, glm::vec4> vecProperties;
		std::unordered_map<std::string, glm::mat4> matProperties;
		PROPERTY(textureProperties)
		std::unordered_map<std::string, const Texture*> textureProperties;
	public:
		SH_RENDER_API void Clear();

		              template<typename T>
		              void SetProperty(const std::string& name, const T& data);
		SH_RENDER_API void SetProperty(const std::string& name, const Texture* data);

		SH_RENDER_API auto GetScalarProperty(const std::string& name) const -> std::optional<float>;
		SH_RENDER_API auto GetVectorProperty(const std::string& name) const -> const glm::vec4*;
		SH_RENDER_API auto GetMatrixProperty(const std::string& name) const -> const glm::mat4*;
		SH_RENDER_API auto GetTextureProperty(const std::string& name) const -> const Texture*;
		SH_RENDER_API auto GetTextureProperties() const -> const std::unordered_map<std::string, const Texture*>&;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	};

	template<typename T>
	inline void MaterialPropertyBlock::SetProperty(const std::string& name, const T& data)
	{
		if constexpr (std::is_arithmetic_v<T>)
			scalarProperties.insert_or_assign(name, static_cast<const float&>(data));
		else if constexpr (std::is_same_v<T, glm::vec2>)
			vecProperties.insert_or_assign(name, glm::vec4{ data, 0.0f, 0.0f });
		else if constexpr (std::is_same_v<T, glm::vec3>)
			vecProperties.insert_or_assign(name, glm::vec4{ data, 0.0f });
		else if constexpr (std::is_same_v<T, glm::vec4>)
			vecProperties.insert_or_assign(name, data);
		else if constexpr (std::is_same_v<T, glm::mat2>)
			matProperties.insert_or_assign(name, core::Util::ConvertMat2ToMat4(data));
		else if constexpr (std::is_same_v<T, glm::mat3>)
			matProperties.insert_or_assign(name, core::Util::ConvertMat3ToMat4(data));
		else if constexpr (std::is_same_v<T, glm::mat4>)
			matProperties.insert_or_assign(name, data);
		else
			static_assert(true);
	}
}//namespace