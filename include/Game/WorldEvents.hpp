#pragma once
#include "Export.h"
#include "GameObject.h"
#include "Component/Camera.h"

#include "Core/IEvent.h"
#include "Core/Reflection/TypeTraits.hpp"
#include "Core/SContainer.hpp"
namespace sh::game
{
	struct WorldEvents
	{
		struct CameraEvent : core::IEvent
		{
			const core::SObjWeakPtr<Camera> camPtr;

			enum class Type
			{
				Added,
				Removed,
				MainCameraChanged
			} const type;

			CameraEvent(Camera& camera, Type type) :
				camPtr(&camera), type(type)
			{
			}

			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<CameraEvent>();
			}
		};
		struct GameObjectEvent : core::IEvent
		{
			const core::SObjWeakPtr<GameObject> objPtr;

			enum class Type
			{
				Added,
				Removed
			} const type;

			GameObjectEvent(GameObject& obj, Type type) :
				objPtr(&obj), type(type)
			{
			}

			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<GameObjectEvent>();
			}
		};
	};//WorldEvents
}//namespace