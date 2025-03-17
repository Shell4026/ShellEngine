#include "ShaderPassBuilder.h"

namespace sh::render
{
	ShaderPassBuilder::ShaderPassBuilder()
	{
	}

	ShaderPassBuilder::ShaderPassBuilder(const ShaderPassBuilder& other)
	{
		this->vertShaderData = other.vertShaderData;
		this->fragShaderData = other.fragShaderData;
	}

	ShaderPassBuilder::ShaderPassBuilder(ShaderPassBuilder&& other) noexcept
	{
		this->vertShaderData = std::move(other.vertShaderData);
		this->fragShaderData = std::move(other.fragShaderData);
	}

	SH_RENDER_API auto ShaderPassBuilder::GetNextId() -> int
	{
		return idCount++;
	}

	SH_RENDER_API void ShaderPassBuilder::SetData(shaderType type, const std::vector<uint8_t>& data)
	{
		if (type == shaderType::Vertex)
			vertShaderData = data;
		else if (type == shaderType::Fragment)
			fragShaderData = data;
	}

	SH_RENDER_API void ShaderPassBuilder::SetData(shaderType type, std::vector<uint8_t>&& data)
	{
		if (type == shaderType::Vertex)
			vertShaderData = std::move(data);
		else if (type == shaderType::Fragment)
			fragShaderData = std::move(data);
	}

	SH_RENDER_API void ShaderPassBuilder::Clear()
	{
		vertShaderData.clear();
		fragShaderData.clear();
	}
}