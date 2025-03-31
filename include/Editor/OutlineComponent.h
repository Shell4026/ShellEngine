#pragma once
#include "Export.h"

#include "Game/Component/MeshRenderer.h"

namespace sh::editor
{
	class OutlineComponent : public game::MeshRenderer
	{
		COMPONENT(OutlineComponent)
	private:
		PROPERTY(renderer)
		MeshRenderer* renderer = nullptr;
	public:
		SH_EDITOR_API OutlineComponent(game::GameObject& owner);

		SH_EDITOR_API void BeginUpdate() override;
	};
}//namespace