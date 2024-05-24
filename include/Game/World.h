#pragma once

#include "Export.h"
#include "ResourceManager.hpp"

#include "Core/NonCopyable.h"
#include "Core/SObject.h"
#include "Core/Reflection.hpp"

#include "Render/Shader.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/Texture.h"

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace sh::core
{
	class GC;
}

namespace sh::render
{
	class Renderer;
}

namespace sh::game
{
	class GameObject;
	class Camera;

	class World : public sh::core::SObject, public sh::core::INonCopyable
	{
		SCLASS(World)
	private:
		std::vector<std::unique_ptr<GameObject>> objs;
		std::unordered_map<std::string, uint32_t> objsMap;
		std::queue<int> objsEmptyIdx;

		float _deltaTime;
	public:
		sh::render::Renderer& renderer;
		sh::core::GC& gc;
		const float& deltaTime;

		sh::game::ResourceManager<sh::render::Shader> shaders;
		sh::game::ResourceManager<sh::render::Material> materials;
		sh::game::ResourceManager<sh::render::Mesh> meshes;
		sh::game::ResourceManager<sh::render::Texture> textures;

		PROPERTY(mainCamera)
		Camera* mainCamera;
	public:
		SH_GAME_API World(sh::render::Renderer& renderer, sh::core::GC& gc);
		SH_GAME_API World(World&& other) noexcept;
		SH_GAME_API ~World();

		SH_GAME_API void Clean();

		SH_GAME_API auto AddGameObject(const std::string& name) -> GameObject*;
		SH_GAME_API void DestroyGameObject(const std::string& name);
		SH_GAME_API auto ChangeGameObjectName(const std::string& objName, const std::string& to) -> std::string;

		SH_GAME_API void Start();
		SH_GAME_API void Update(float deltaTime);
	};
}