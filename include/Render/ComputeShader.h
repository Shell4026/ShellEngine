#pragma once
#include "Export.h"
#include "ShaderAST.h"

#include "Core/SObject.h"
#include "Core/Name.h"
#include "Core/Reflection.hpp"

#include <vector>
#include <cstdint>
namespace sh::render
{
	struct ComputeShaderCreateInfo;

	class ComputeShader : public core::SObject
	{
		SCLASS(ComputeShader)
	public:
		SH_RENDER_API ComputeShader(ComputeShaderCreateInfo createInfo);
		SH_RENDER_API ~ComputeShader();

		/// @brief AST와 SPIR-V 바이너리를 직렬화 한다.
		/// @return 직렬화 된 json
		SH_RENDER_API auto Serialize() const -> core::Json override;
		/// @brief AST와 SPIR-V 바이너리를 역직렬화 한다.
		/// @param json 직렬화 된 json
		SH_RENDER_API void Deserialize(const core::Json& json) override;

		SH_RENDER_API auto GetSpirv() const -> const std::vector<uint8_t>& { return spirv; }
		SH_RENDER_API auto GetShaderAST() const -> const ShaderAST::ComputeShaderNode& { return shaderNode; }

		SH_RENDER_API auto GetNumthreadsX() const -> uint32_t { return shaderNode.numthreadsX; }
		SH_RENDER_API auto GetNumthreadsY() const -> uint32_t { return shaderNode.numthreadsY; }
		SH_RENDER_API auto GetNumthreadsZ() const -> uint32_t { return shaderNode.numthreadsZ; }

		SH_RENDER_API auto GetBuffers() const -> const std::vector<ShaderAST::BufferNode>& { return shaderNode.buffers; }
	private:
		ShaderAST::ComputeShaderNode shaderNode;
		std::vector<uint8_t> spirv;
	};
}//namespace
