#include "ComputeShader.h"
#include "ComputeShaderCreateInfo.h"

namespace sh::render
{
	ComputeShader::ComputeShader(ComputeShaderCreateInfo createInfo)
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
}//namespace
