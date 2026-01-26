#pragma once
#include "Editor/Export.h"

#include "Game/Component/Render/MeshRenderer.h"

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

		SH_EDITOR_API auto Serialize() const -> core::Json override;
	};
}//namespace