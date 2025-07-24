#pragma once
#include "Export.h"
#include "ShaderAST.h"

#include <vector>
#include <memory>
namespace sh::render
{
	class Shader;
	class ShaderPass;

	struct ShaderCreateInfo
	{
		ShaderAST::ShaderNode shaderNode;
		std::vector<ShaderPass*> passes;
	};
}//namespace