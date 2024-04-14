#include "ShaderBuilder.h"

namespace sh::render
{
	void ShaderBuilder::SetData(shaderType type, const std::vector<unsigned char>& data)
	{
		if (type == shaderType::Vertex)
			vertShaderData = data;
		else if (type == shaderType::Fragment)
			fragShaderData = data;
	}

	void ShaderBuilder::SetData(shaderType type, std::vector<unsigned char>&& data)
	{
		if (type == shaderType::Vertex)
			vertShaderData = std::move(data);
		else if (type == shaderType::Fragment)
			fragShaderData = std::move(data);
	}

	void ShaderBuilder::ClearData()
	{
		vertShaderData.clear();
		fragShaderData.clear();
	}
}