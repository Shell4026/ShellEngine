#pragma once
#include "Export.h"

#include "Core/Util.h"
#include "Core/Asset.h"
#include "Core/IAssetLoader.h"

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
namespace sh::game
{
	class ModelLoader : public core::IAssetLoader
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
		friend std::hash<sh::game::ModelLoader::Indices>;
		static constexpr const char* ASSET_NAME = "mesh";
	public:
		const render::IRenderContext& context;
	private:
		auto CalculateTangent(
			const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
			const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) const->glm::vec3;
		void CreateTangents(std::vector<render::Mesh::Vertex>& verts, const std::vector<uint32_t>& indices);
	protected:
		SH_GAME_API auto LoadObj(const std::filesystem::path& dir) -> render::Model*;
		SH_GAME_API auto LoadGLTF(const std::filesystem::path& dir) -> render::Model*;
	public:
		SH_GAME_API ModelLoader(const render::IRenderContext& context);

		SH_GAME_API auto Load(const std::filesystem::path& filename) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;

		SH_GAME_API auto GetAssetName() const -> const char*;
	};
}//namespace

namespace std
{
	template <>
	class hash<sh::game::ModelLoader::Indices>
	{
	public:
		auto operator()(const sh::game::ModelLoader::Indices& indices) const -> std::uint64_t
		{
			std::hash<uint32_t> hasher{};
			std::size_t hash1 = sh::core::Util::CombineHash(hasher(indices.vert), hasher(indices.normal));
			
			return sh::core::Util::CombineHash(hash1, hasher(indices.uv));
		}
	};
}
