#pragma once

#include "Export.h"
#include "Shader.h"

namespace sh::render
{
	class Material
	{
	private:
		int id;

		Shader* shader;
	public:
		SH_RENDER_API void SetShader(Shader* shader);
		SH_RENDER_API auto GetShader() const -> Shader*;
	};
}