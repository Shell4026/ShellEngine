#include "Component/OutlineComponent.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	OutlineComponent::OutlineComponent(game::GameObject& owner) :
		game::MeshRenderer(owner)
	{
		mat = static_cast<render::Material*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f13" }));
		assert(mat);
		renderer = owner.GetComponent<MeshRenderer>();
		canPlayInEditor = true;
	}
	SH_EDITOR_API void OutlineComponent::BeginUpdate()
	{
		if (!core::IsValid(renderer))
			renderer = gameObject.GetComponent<MeshRenderer>();
		if (!core::IsValid(renderer))
			return;

		if (mesh != renderer->GetMesh())
			SetMesh(renderer->GetMesh());
	}
	SH_EDITOR_API auto OutlineComponent::Serialize() const -> core::Json
	{
		return core::Json{};
	}
}//namespace