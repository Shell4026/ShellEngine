#include "ComputeShader.h"
#include "ComputeShaderCreateInfo.h"
#include "BufferFactory.h"

namespace sh::render
{
	ComputeShader::ComputeShader(const IRenderContext& ctx, ComputeShaderCreateInfo createInfo) :
		ctx(ctx)
	{
		SetName(createInfo.shaderNode.shaderName);

		shaderNode = std::move(createInfo.shaderNode);
		spirv = std::move(createInfo.spirv);

		// 코드 텍스트는 보존할 필요 없음
		shaderNode.code.clear();
		shaderNode.functions.clear();
		shaderNode.declaration.clear();
	}
	ComputeShader::~ComputeShader() = default;

	SH_RENDER_API auto ComputeShader::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		core::Json& json = mainJson["computeShader"];
		json["AST"] = shaderNode.Serialize();
		json["spirv"] = spirv;

		return mainJson;
	}

	SH_RENDER_API void ComputeShader::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);

		if (!json.contains("computeShader"))
			return;

		const core::Json& csJson = json["computeShader"];

		if (csJson.contains("AST"))
			shaderNode.Deserialize(csJson["AST"]);
		if (csJson.contains("spirv"))
			spirv = csJson["spirv"].get<std::vector<uint8_t>>();
	}

	SH_RENDER_API void ComputeShader::SetFloats(const std::string& name, float* values, std::size_t count)
	{
		uint32_t binding = 0;
		bool bFind = false;
		for (const ShaderAST::BufferNode& bufferNode : shaderNode.buffers)
		{
			if (bufferNode.name == name)
			{
				if (!bufferNode.vars.empty() && bufferNode.vars[0].type == ShaderAST::VariableType::Float)
				{
					binding = bufferNode.binding;
					bFind = true;
					break;
				}
			}
		}
		if (!bFind)
			return;

		if (buffers.size() <= binding)
			buffers.resize(binding + 1);
		if (buffers[binding] == nullptr)
		{
			BufferFactory::CreateInfo ci{};
			ci.bDynamic = true;
			ci.size = sizeof(float) * count;
			buffers[binding] = BufferFactory::Create(ctx, ci);
			buffers[binding]->SetData(values);

			if (shaderBinding == nullptr)
				shaderBinding = BufferFactory::CreateShaderBinding(ctx, *this);
			shaderBinding->Link(binding, *buffers[binding]);
		}
		else
		{
			buffers[binding]->Resize(sizeof(float) * count);
			buffers[binding]->SetData(values);

			if (shaderBinding == nullptr)
				shaderBinding = BufferFactory::CreateShaderBinding(ctx, *this);
			shaderBinding->Link(binding, *buffers[binding]);
		}
	}

	SH_RENDER_API auto ComputeShader::GetBuffer(const std::string& name) -> IBuffer*
	{
		std::optional<uint32_t> binding = GetBinding(name);
		if (!binding.has_value())
			return nullptr;
		if (*binding >= buffers.size())
			return nullptr;
		return buffers[*binding].get();
	}

	auto ComputeShader::GetBinding(const std::string& name) -> std::optional<uint32_t>
	{
		for (const ShaderAST::BufferNode& bufferNode : shaderNode.buffers)
		{
			if (bufferNode.name == name)
				return bufferNode.binding;
		}
		return {};
	}
}//namespace
