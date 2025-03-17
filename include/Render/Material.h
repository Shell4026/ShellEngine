#pragma once
#include "Export.h"
#include "Shader.h"
#include "Texture.h"
#include "IRenderResource.h"
#include "MaterialPropertyBlock.h"
#include "MaterialData.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"
#include "Core/Util.h"

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

namespace sh::render
{
	class Material : 
		public core::SObject, 
		public IRenderResource
	{
		SCLASS(Material)
	private:
		const IRenderContext* context = nullptr;

		PROPERTY(shader)
		Shader* shader;

		PROPERTY(propertyBlock)
		MaterialPropertyBlock* propertyBlock = nullptr;

		std::set<std::pair<const ShaderPass*, const UniformStructLayout*>> dirtyPropSet;

		std::unique_ptr<MaterialData> materialData;

		bool bPropertyDirty = false;
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
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);
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
		SH_RENDER_API void SetProperty(const std::string& name, const Texture* data);

		SH_RENDER_API auto GetMaterialData() const -> const MaterialData&;

		SH_RENDER_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_RENDER_API void Deserialize(const core::Json& json) override;
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

		assert(*propInfo->type == core::reflection::GetType<T>());
		if (*propInfo->type != core::reflection::GetType<T>())
			return;

		propertyBlock->SetProperty(name, data);

		for (auto& location : propInfo->locations)
			dirtyPropSet.insert({ location.passPtr, location.layoutPtr });

		bPropertyDirty = true;
	}
}