#pragma once
#include "Export.h"
#include "IImporter.h"

#include "Core/Util.h"

#include "Render/Mesh.h"

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <string_view>
#include <filesystem>
namespace sh::render
{
	class IRenderContext;
	class Model;
}
namespace sh::editor
{
	class MeshImporter : public IImporter
	{
	private:
		const char* name = "MeshImporter";
	public:
		SH_EDITOR_API auto GetName() const -> const char* override;
		SH_EDITOR_API auto Serialize() const -> core::Json override;
		SH_EDITOR_API void Deserialize(const core::Json& json) override;
	};

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
	private:
		auto CalculateTangent(
			const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
			const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) const->glm::vec3;
		void CreateTangents(std::vector<render::Mesh::Vertex>& verts, const std::vector<uint32_t>& indices);
	public:
		SH_EDITOR_API ModelLoader(const render::IRenderContext& context);
		SH_EDITOR_API virtual ~ModelLoader() = default;
		SH_EDITOR_API virtual auto Load(const std::filesystem::path& filename) -> render::Model*;
		SH_EDITOR_API auto LoadGLTF(const std::filesystem::path& dir) -> render::Model*;
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
