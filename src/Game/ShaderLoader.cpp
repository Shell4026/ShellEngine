#include "ShaderLoader.h"

#include "ShaderBuilder.h"

#include "Core/FileLoader.h"

#include "render/VulkanShader.h"

namespace sh::game
{
	ShaderLoader::ShaderLoader(ShaderBuilder* builder) :
		builder(builder)
	{

	}
	ShaderLoader::~ShaderLoader()
	{
	}

	auto ShaderLoader::LoadShader(std::string_view vertexShader, std::string_view fragShader) -> render::Shader*
	{
		if (!builder)
			return nullptr;

		auto vertData = loader->LoadBinary(vertexShader);
		if (!vertData.has_value())
			return nullptr;

		auto fragData = loader->LoadBinary(fragShader);
		if (!fragData.has_value())
			return nullptr;

		builder->SetData(ShaderBuilder::shaderType::Vertex, std::move(vertData.value()));
		builder->SetData(ShaderBuilder::shaderType::Fragment, std::move(fragData.value()));

		return builder->Build();
	}
}