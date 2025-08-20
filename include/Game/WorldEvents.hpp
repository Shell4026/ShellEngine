#pragma once
#include "Core/IEvent.h"
#include "Core/Reflection/TypeTraits.hpp"
namespace sh::game
{
	class GameObject;
	class Component;
	class Camera;

	namespace events
	{
		struct CameraEvent : core::IEvent
		{
			Camera& camera;

			enum class Type
			{
				Added,
				Removed,
				MainCameraChanged
			} const type;

			CameraEvent(Camera& camera, Type type) :
				camera(camera), type(type)
			{
			}

			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<CameraEvent>();
			}
		};
		struct GameObjectEvent : core::IEvent
		{
			GameObject& gameObject;

			enum class Type
			{
				Added,
				Removed,
			} const type;

			GameObjectEvent(GameObject& obj, Type type) :
				gameObject(obj), type(type)
			{
			}

			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<GameObjectEvent>();
			}
		};
		struct ComponentEvent : core::IEvent
		{
			enum class Type
			{
				Added,
				Removed,
			} const type;

			Component& component;

			ComponentEvent(Component& comp, Type type) :
				component(comp), type(type)
			{
			}

			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<ComponentEvent>();
			}
		};
		struct WorldEvent : core::IEvent
		{
			enum class Type
			{
				Play,
				Stop,
				Pause
			} const type;

			WorldEvent(Type type) :
				type(type)
			{}


			auto GetTypeHash() const -> std::size_t override
			{
				return core::reflection::TypeTraits::GetTypeHash<WorldEvent>();
			}
		};
	}//events namespace
}//namespace