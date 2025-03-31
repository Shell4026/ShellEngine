﻿#include "OutlineComponent.h"

#include "Game/GameObject.h"

namespace sh::editor
{
	OutlineComponent::OutlineComponent(game::GameObject& owner) :
		game::MeshRenderer(owner)
	{
		this->mat = world.materials.GetResource("OutlinePreMaterial");
		renderer = owner.GetComponent<MeshRenderer>();
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
}//namespace