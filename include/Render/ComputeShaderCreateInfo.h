#pragma once
#include "ShaderAST.h"

#include <vector>
namespace sh::render
{
	struct ComputeShaderCreateInfo
	{
		ShaderAST::ComputeShaderNode shaderNode;
		std::vector<uint8_t> spirv;
	};
}//namespace
