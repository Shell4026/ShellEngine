#pragma once
#include "Export.h"
#include "ShaderAST.h"
#include "IBuffer.h"
#include "IShaderBinding.h"

#include "Core/SObject.h"
#include "Core/Name.h"
#include "Core/Reflection.hpp"

#include <vector>
#include <cstdint>
#include <optional>
namespace sh::render
{
	struct ComputeShaderCreateInfo;

	class IRenderContext;
	class ComputeShader : public core::SObject
	{
		SCLASS(ComputeShader)
	public:
		SH_RENDER_API ComputeShader(const IRenderContext& ctx, ComputeShaderCreateInfo createInfo);
		SH_RENDER_API ~ComputeShader();

		/// @brief AST와 SPIR-V 바이너리를 직렬화 한다.
		/// @return 직렬화 된 json
		SH_RENDER_API auto Serialize() const -> core::Json override;
		/// @brief AST와 SPIR-V 바이너리를 역직렬화 한다.
		/// @param json 직렬화 된 json
		SH_RENDER_API void Deserialize(const core::Json& json) override;

		SH_RENDER_API void SetFloats(const std::string& name, float* values, std::size_t count);

		SH_RENDER_API auto GetSpirv() const -> const std::vector<uint8_t>& { return spirv; }
		SH_RENDER_API auto GetShaderAST() const -> const ShaderAST::ComputeShaderNode& { return shaderNode; }

		SH_RENDER_API auto GetNumthreadsX() const -> uint32_t { return shaderNode.numthreadsX; }
		SH_RENDER_API auto GetNumthreadsY() const -> uint32_t { return shaderNode.numthreadsY; }
		SH_RENDER_API auto GetNumthreadsZ() const -> uint32_t { return shaderNode.numthreadsZ; }
		SH_RENDER_API auto GetNode() const -> const ShaderAST::ComputeShaderNode& { return shaderNode; }
		SH_RENDER_API auto GetBuffers() const -> const std::vector<std::unique_ptr<IBuffer>>& { return buffers; }
		SH_RENDER_API auto GetShaderBinding() const -> IShaderBinding* { return shaderBinding.get(); }
		SH_RENDER_API auto GetBuffer(const std::string& name) -> IBuffer*;
	private:
		auto GetBinding(const std::string& name) -> std::optional<uint32_t>;
	private:
		const IRenderContext& ctx;

		ShaderAST::ComputeShaderNode shaderNode;
		std::vector<uint8_t> spirv;

		std::vector<std::unique_ptr<IBuffer>> buffers; // binding, buffer
		std::unique_ptr<IShaderBinding> shaderBinding;
	};
}//namespace
