#pragma once

#include "Export.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <string_view>

namespace sh::core
{
	class GC;
}
namespace sh::render
{
	class Renderer;
	class Shader;
	class Material;
	class Mesh;
}
namespace sh::game
{
	class ResourceManager : public sh::core::INonCopyable
	{
	private:
		sh::core::GC& gc;

		std::unordered_map<std::string, std::unique_ptr<sh::render::Shader>> shaders;
		std::unordered_map<std::string, std::unique_ptr<sh::render::Material>> mats;
		std::unordered_map<std::string, std::unique_ptr<sh::render::Mesh>> meshes;
	private:
		auto LoadShader(std::string_view dir) -> sh::render::Shader*;
	public:
		SH_GAME_API ResourceManager(sh::core::GC& gc);

		SH_GAME_API void Clean();

		SH_GAME_API auto AddShader(std::string_view name, std::unique_ptr<sh::render::Shader>&& shader) -> sh::render::Shader*;
		SH_GAME_API bool DestroyShader(std::string_view name);
		SH_GAME_API auto GetShader(std::string_view name) -> sh::render::Shader*;

		SH_GAME_API auto AddMaterial(std::string_view name, sh::render::Material&& mat) -> sh::render::Material*;
		SH_GAME_API auto AddMaterial(std::string_view name, std::unique_ptr<sh::render::Material>&& mat) -> sh::render::Material*;
		SH_GAME_API bool DestroyMaterial(std::string_view name);
		SH_GAME_API auto GetMaterial(std::string_view name) -> sh::render::Material*;

		SH_GAME_API auto AddMesh(std::string_view name) -> sh::render::Mesh*;
		SH_GAME_API auto AddMesh(std::string_view name, sh::render::Mesh&& mesh) -> sh::render::Mesh*;
		SH_GAME_API bool DestroyMesh(std::string_view name);
		SH_GAME_API auto GetMesh(std::string_view name) -> sh::render::Mesh*;
	};
}