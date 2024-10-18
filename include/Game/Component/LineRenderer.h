#pragma once

#include "Export.h"
#include "Component.h"
#include "MeshRenderer.h"

#include "Core/Reflection.hpp"
#include "Core/SContainer.hpp"

#include "Render/Mesh.h"

#include <glm/vec3.hpp>

namespace sh::game
{
	class LineRenderer : public MeshRenderer
	{
		COMPONENT(LineRenderer)
	private:
		PROPERTY(start)
		glm::vec3 start;
		PROPERTY(end)
		glm::vec3 end;

		render::Mesh mesh;
		PROPERTY(mat)
		render::Material* mat;

		bool bUpdate;
	public:
		SH_GAME_API LineRenderer(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void Update() override;

		SH_GAME_API void SetStart(const glm::vec3& start);
		SH_GAME_API void SetEnd(const glm::vec3& end);
#if SH_EDITOR
		SH_CORE_API void OnPropertyChanged(const core::reflection::Property& prop) override;
#endif
	};
}//namespace