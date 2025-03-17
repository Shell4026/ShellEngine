#pragma once
#include "Export.h"
#include "ShaderAST.hpp"

#include <vector>
#include <memory>
#include <cstdint>

namespace sh::render
{
	class ShaderPass;
	class ShaderPassBuilder
	{
	public:
		enum class shaderType
		{
			Vertex,
			Fragment
		};
	private:
		SH_RENDER_API inline static int idCount = 0;
	protected:
		std::vector<uint8_t> vertShaderData;
		std::vector<uint8_t> fragShaderData;
	protected:
		SH_RENDER_API static auto GetNextId() -> int;
	public:
		SH_RENDER_API ShaderPassBuilder();
		SH_RENDER_API ShaderPassBuilder(const ShaderPassBuilder& other);
		SH_RENDER_API ShaderPassBuilder(ShaderPassBuilder&& other) noexcept;

		SH_RENDER_API void SetData(shaderType type, const std::vector<uint8_t>& data);
		SH_RENDER_API void SetData(shaderType type, std::vector<uint8_t>&& data);

		SH_RENDER_API virtual auto Build(const ShaderAST::PassNode& passNode) -> std::unique_ptr<render::ShaderPass> = 0;
		SH_RENDER_API virtual void Clear();
	};
}