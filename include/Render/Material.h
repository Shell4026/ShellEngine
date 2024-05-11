#pragma once

#include "Export.h"
#include "Shader.h"

#include "Core/SObject.h"
#include "Core/Reflection.hpp"

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
	};
}