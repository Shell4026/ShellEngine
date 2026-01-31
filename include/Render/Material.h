#pragma once
#include "Export.h"
#include "Shader.h"
#include "Texture.h"
#include "IRenderResource.h"
#include "MaterialPropertyBlock.h"
#include "MaterialData.h"
#include "RenderTexture.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/Util.h"
#include "Core/Logger.h"

#include "glm/mat4x4.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <map>
#include <memory>
#include <any>
#include <unordered_set>
#include <cassert>
#include <optional>

namespace sh::render
{
	class RenderTexture;

	class Material : 
		public core::SObject, 
		public IRenderResource
	{
		SCLASS(Material)
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);
		SH_RENDER_API Material(const Material& other);
		SH_RENDER_API Material(Material&& other) noexcept;

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;
		
		/// @brief 렌더러API에 맞는 버퍼와 유니폼 버퍼를 생성한다.
		/// @param renderer 렌더러
		SH_RENDER_API void Build(const IRenderContext& context) override;

		/// @brief 유니폼 버퍼를 갱신하는 함수. 변화가 있던 경우에만 갱신한다.
		SH_RENDER_API void UpdateUniformBuffers();

		              template<typename T>
		              void SetProperty(const std::string& name, const T& data);
		SH_RENDER_API void SetProperty(const std::string& name, const glm::vec4& data);
		SH_RENDER_API void SetProperty(const std::string& name, const Texture* data);
		SH_RENDER_API void SetProperty(const std::string& name, Texture* data);
		SH_RENDER_API void SetProperty(const std::string& name, const RenderTexture* data);
		SH_RENDER_API void SetProperty(const std::string& name, RenderTexture* data);
		              template<typename T>
					  auto GetProperty(const std::string& name) const -> std::optional<T>;

		SH_RENDER_API auto GetMaterialData() const -> const MaterialData&;
		              template<typename T>
		              void SetConstant(const std::string& name, const T& value);
		SH_RENDER_API auto GetConstantData(const ShaderPass& pass) const -> const std::vector<uint8_t>*;

		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_RENDER_API auto Serialize() const -> core::Json override;
		SH_RENDER_API void Deserialize(const core::Json& json) override;

		SH_RENDER_API auto GetCachedRenderTextures() const -> const core::SHashMap<std::string, const RenderTexture*>& { return cachedRts; }
	private:
		void Clear();
		void SetDefaultProperties();
		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset, std::size_t size)
		{
			std::memcpy(uniformData.data() + offset, &data, size);
		}
		template<typename T>
		void SetData(const T& data, std::vector<uint8_t>& uniformData, std::size_t offset)
		{
			std::memcpy(uniformData.data() + offset, &data, sizeof(T));
		}
		void UpdateListener();
	public:
		mutable core::Observer<false, const Shader*> onShaderChanged;
	private:
		const IRenderContext* context = nullptr;

		PROPERTY(shader)
		Shader* shader;

		MaterialPropertyBlock propertyBlock;

		std::unique_ptr<MaterialData> materialData;

		std::vector<std::pair<const ShaderPass*, const UniformStructLayout*>> dirtyProps;
		std::vector<std::vector<uint8_t>> cachedConstantData;
		core::SHashMap<std::string, const RenderTexture*> cachedRts;

		core::Observer<false, const Texture*>::Listener onBufferUpdateListener;

		bool bPropertyDirty = false;
	};

	template<typename T>
	inline void Material::SetProperty(const std::string& name, const T& data)
	{
		if (!core::IsValid(shader))
			return;

		const Shader::PropertyInfo* propInfo = shader->GetProperty(name);
		assert(propInfo != nullptr);
		if (propInfo == nullptr)
			return;

		assert(propInfo->type == core::reflection::GetType<T>());
		if (propInfo->type != core::reflection::GetType<T>())
			return;

		propertyBlock.SetProperty(name, data);

		for (auto& location : propInfo->locations)
		{
			auto it = std::find(dirtyProps.begin(), dirtyProps.end(), std::pair{ location.passPtr.Get(), location.layoutPtr});
			if (it == dirtyProps.end())
				dirtyProps.push_back({ location.passPtr.Get(), location.layoutPtr});
		}

		bPropertyDirty = true;
	}
	template<typename T>
	inline void Material::SetConstant(const std::string& name, const T& value)
	{
		if (shader == nullptr)
			return;
		if (cachedConstantData.empty())
			return;

		const auto& passDatas = shader->GetAllShaderPass();

		for (auto& lightingPass : passDatas)
		{
			int passIdx = 0;
			for (ShaderPass& pass : lightingPass.passes)
			{
				const auto* info = pass.GetConstantsInfo(name);
				if (info == nullptr)
					continue;

				std::size_t expected = sizeof(T);
				if constexpr (std::is_same_v<T, bool>)
					expected = 4;
				if (expected != info->size)
				{
					SH_ERROR_FORMAT("{} size mismatch!", name);
					continue;
				}
				std::vector<uint8_t>& buffer = cachedConstantData[passIdx];
				if (buffer.size() < info->offset + info->size) 
					buffer.resize(pass.GetConstantSize());
				std::memcpy(buffer.data() + info->offset, &value, sizeof(T));
			}
		}
	}
	template<typename T>
	inline auto Material::GetProperty(const std::string& name) const -> std::optional<T>
	{
		if constexpr (std::is_arithmetic_v<T>)
		{
			auto scalar = propertyBlock.GetScalarProperty(name);
			if (scalar.has_value())
				return scalar.value();
			return {};
		}
		else if constexpr (std::is_same_v<T, glm::vec2>)
		{
			auto ptr = propertyBlock.GetVectorProperty(name);
			if (!ptr)
				return {};
			return glm::vec2{ ptr->x, ptr->y };
		}
		else if constexpr (std::is_same_v<T, glm::vec3>)
		{
			auto ptr = propertyBlock.GetVectorProperty(name);
			if (!ptr)
				return {};
			return glm::vec3{ ptr->x, ptr->y, ptr->z };
		}
		else if constexpr (std::is_same_v<T, glm::vec4>)
		{
			auto ptr = propertyBlock.GetVectorProperty(name);
			if (!ptr)
				return {};
			return *ptr;
		}
		else if constexpr (std::is_same_v<T, glm::mat2>)
		{
			auto ptr = propertyBlock.GetMatrixProperty(name);
			if (!ptr)
				return {};
			return core::Util::ConvertMat4ToMat2(*ptr);
		}
		else if constexpr (std::is_same_v<T, glm::mat3>)
		{
			auto ptr = propertyBlock.GetMatrixProperty(name);
			if (!ptr)
				return {};
			return core::Util::ConvertMat4ToMat3(*ptr);
		}
		else if constexpr (std::is_same_v<T, glm::mat4>)
		{
			auto ptr = propertyBlock.GetMatrixProperty(name);
			if (!ptr)
				return {};
			return *ptr;
		}
		else if constexpr (std::is_same_v<T, const Texture*>)
		{
			return propertyBlock.GetTextureProperty(name);
		}
		else
			static_assert(true);
		return {};
	}
}