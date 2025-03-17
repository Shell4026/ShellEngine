#pragma once
#include "Export.h"

#include "Core/Util.h"

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string_view>

namespace sh::render
{
	class IRenderContext;
	class Mesh;
}
namespace sh::editor
{
	class ModelLoader
	{
	private:
		struct Indices
		{
			uint32_t vert;
			uint32_t normal;
			uint32_t uv;
			
			bool operator==(const Indices& other) const
			{
				return vert == other.vert 
					&& normal == other.normal 
					&& uv == other.uv;
			}
		};
		friend std::hash<sh::editor::ModelLoader::Indices>;
	public:
		const render::IRenderContext& context;
	public:
		SH_EDITOR_API ModelLoader(const render::IRenderContext& context);
		SH_EDITOR_API virtual ~ModelLoader() = default;
		SH_EDITOR_API virtual auto Load(std::string_view filename) -> render::Mesh*;
	};
}//namespace

namespace std
{
	template <>
	class hash<sh::editor::ModelLoader::Indices>
	{
	public:
		auto operator()(const sh::editor::ModelLoader::Indices& indices) const -> std::uint64_t
		{
			std::hash<uint32_t> hasher{};
			std::size_t hash1 = sh::core::Util::CombineHash(hasher(indices.vert), hasher(indices.normal));
			
			return sh::core::Util::CombineHash(hash1, hasher(indices.uv));
		}
	};
}
