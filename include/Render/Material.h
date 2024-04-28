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
	};
}