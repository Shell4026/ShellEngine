#pragma once

#include "Component.h"

#include "Game/Export.h"

#include "Render/IDrawable.h"
#include "Render/Material.h"
#include "Render/Mesh.h"

#include "glm/mat4x4.hpp"
#include <unordered_map>
#include <memory>

namespace sh::game
{
	class MeshRenderer : public Component
	{
		SCLASS(MeshRenderer)
	private:
		struct Ubo
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		} ubo;
	private:
		PROPERTY(mesh)
		sh::render::Mesh* mesh;
		PROPERTY(mat)
		sh::render::Material* mat;
		static std::unordered_map<sh::render::Mesh*, std::unique_ptr<sh::render::IDrawable>> drawables;

		sh::render::IDrawable* drawable;
	private:
		void CreateDrawable();
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);
		SH_GAME_API ~MeshRenderer();

		SH_GAME_API void SetMesh(sh::render::Mesh& mesh);
		SH_GAME_API auto GetMesh() const -> const sh::render::Mesh&;

		SH_GAME_API void SetMaterial(sh::render::Material& mat);
		SH_GAME_API auto GetMaterial() const -> const sh::render::Material&;

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;
	};

	std::unordered_map<sh::render::Mesh*, std::unique_ptr<sh::render::IDrawable>> MeshRenderer::drawables{};
}