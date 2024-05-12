#pragma once

#include "Export.h"
#include "Shader.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"
#include "Core/Util.h"

#include <string_view>

namespace sh::render
{
	class Material : public sh::core::SObject
	{
		SCLASS(Material)
	private:
		PROPERTY(shader)
		Shader* shader;
	public:
		SH_RENDER_API Material();
		SH_RENDER_API Material(Shader* shader);

		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;

		template<typename T>
		bool SetAttribute(std::string_view name, T value);
	};

	template<typename T>
	inline bool Material::SetAttribute(std::string_view name, T value)
	{
		if (!sh::core::IsValid(shader))
			return false;

		auto idx = shader->GetPropertyIdx(name);
		if (!idx)
			return false;


	}
}