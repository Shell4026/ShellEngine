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
		SCLASS(LineRenderer)
	private:
		PROPERTY(start)
		glm::vec3 start;
		PROPERTY(end)
		glm::vec3 end;

		render::Mesh mesh;
	public:
		SH_GAME_API LineRenderer();

		SH_GAME_API void Awake() override;
		SH_GAME_API void Update() override;

#if SH_EDITOR
		SH_CORE_API void OnPropertyChanged(const core::reflection::Property& prop) override;
#endif
	};
}//namespace