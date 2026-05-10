#pragma once
#include "Export.h"
#include "Texture.h"

#include "Core/ISerializable.h"
#include "Core/SContainer.hpp"

#include "glm/mat4x4.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace sh::render
{
	/// @brief 메테리얼에서 프로퍼티 값을 보관하는 객체
	class MaterialPropertyBlock : public core::ISerializable
	{
	public:
		SH_RENDER_API void Clear();

		              template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>>>
		              void SetProperty(const std::string& name, const T& data);
		              template<typename T>
		              void SetArrayProperty(const std::string& name, const T* data, std::size_t count);
		SH_RENDER_API void SetProperty(const std::string& name, const Texture* data);

		SH_RENDER_API auto GetScalarProperty(const std::string& name) const -> std::optional<float>;
		SH_RENDER_API auto GetVectorProperty(const std::string& name) const -> const glm::vec4*;
		SH_RENDER_API auto GetMatrixProperty(const std::string& name) const -> const glm::mat4*;
		SH_RENDER_API auto GetTextureProperty(const std::string& name) const -> const Texture*;
		SH_RENDER_API auto GetIntArrayProperty(const std::string& name) const -> const std::vector<int>*;
		SH_RENDER_API auto GetScalarArrayProperty(const std::string& name) const -> const std::vector<float>*;
		SH_RENDER_API auto GetVectorArrayProperty(const std::string& name) const -> const std::vector<glm::vec4>*;
		SH_RENDER_API auto GetMatrixArrayProperty(const std::string& name) const -> const std::vector<glm::mat4>*;
		SH_RENDER_API auto GetTextureProperties() const -> const std::unordered_map<std::string, const Texture*>&;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;
	private:
		std::unordered_map<std::string, float> scalarProperties;
		std::unordered_map<std::string, glm::vec4> vecProperties;
		std::unordered_map<std::string, glm::mat4> matProperties;
		std::unordered_map<std::string, std::vector<int>> intArrayProperties;
		std::unordered_map<std::string, std::vector<float>> scalarArrayProperties;
		std::unordered_map<std::string, std::vector<glm::vec4>> vecArrayProperties;
		std::unordered_map<std::string, std::vector<glm::mat4>> matArrayProperties;
		core::SHashMap<std::string, const Texture*> textureProperties;
	};

	template<typename T, typename>
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
	template<typename T>
	inline void MaterialPropertyBlock::SetArrayProperty(const std::string& name, const T* data, std::size_t count)
	{
		if constexpr (std::is_arithmetic_v<T>)
		{
			if constexpr (std::is_integral_v<T>)
			{
				std::vector<int> values(count);
				for (std::size_t i = 0; i < count; ++i)
					values[i] = static_cast<int>(data[i]);
				intArrayProperties.insert_or_assign(name, std::move(values));
			}
			else
			{
				std::vector<float> values(count);
				for (std::size_t i = 0; i < count; ++i)
					values[i] = static_cast<float>(data[i]);
				scalarArrayProperties.insert_or_assign(name, std::move(values));
			}
		}
		else if constexpr (std::is_same_v<T, glm::vec2>)
		{
			std::vector<glm::vec4> values(count);
			for (std::size_t i = 0; i < count; ++i)
				values[i] = glm::vec4{ data[i], 0.0f, 0.0f };
			vecArrayProperties.insert_or_assign(name, std::move(values));
		}
		else if constexpr (std::is_same_v<T, glm::vec3>)
		{
			std::vector<glm::vec4> values(count);
			for (std::size_t i = 0; i < count; ++i)
				values[i] = glm::vec4{ data[i], 0.0f };
			vecArrayProperties.insert_or_assign(name, std::move(values));
		}
		else if constexpr (std::is_same_v<T, glm::vec4>)
		{
			if (count == 0)
				vecArrayProperties.insert_or_assign(name, std::vector<glm::vec4>{});
			else
				vecArrayProperties.insert_or_assign(name, std::vector<glm::vec4>{ data, data + count });
		}
		else if constexpr (std::is_same_v<T, glm::mat2>)
		{
			std::vector<glm::mat4> values(count);
			for (std::size_t i = 0; i < count; ++i)
				values[i] = core::Util::ConvertMat2ToMat4(data[i]);
			matArrayProperties.insert_or_assign(name, std::move(values));
		}
		else if constexpr (std::is_same_v<T, glm::mat3>)
		{
			std::vector<glm::mat4> values(count);
			for (std::size_t i = 0; i < count; ++i)
				values[i] = core::Util::ConvertMat3ToMat4(data[i]);
			matArrayProperties.insert_or_assign(name, std::move(values));
		}
		else if constexpr (std::is_same_v<T, glm::mat4>)
		{
			if (count == 0)
				matArrayProperties.insert_or_assign(name, std::vector<glm::mat4>{});
			else
				matArrayProperties.insert_or_assign(name, std::vector<glm::mat4>{ data, data + count });
		}
		else
			static_assert(true);
	}
}//namespace
