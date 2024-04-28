#include "ShaderBuilder.h"

namespace sh::render
{
	int ShaderBuilder::idCount = 0;

	ShaderBuilder::ShaderBuilder()
	{
	}

	ShaderBuilder::ShaderBuilder(const ShaderBuilder& other)
	{
		this->vertShaderData = other.vertShaderData;
		this->fragShaderData = other.fragShaderData;
	}

	ShaderBuilder::ShaderBuilder(ShaderBuilder&& other) noexcept
	{
		this->vertShaderData = std::move(other.vertShaderData);
		this->fragShaderData = std::move(other.fragShaderData);
	}

	auto ShaderBuilder::GetNextId() -> int
	{
		return idCount++;
	}

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