#pragma once
#include "Editor/Export.h"

#include "Core/Singleton.hpp"
#include "Core/Reflection.hpp"

#include "imgui.h"

#include <unordered_map>
#include <memory>

/// @brief ICustomHierarchy를 상속 후 클래스 선언의 내부에 넣으면 에디터의 하이러키에서 어떻게 동작할지 정의 할 수 있다.
/// @param Class 클래스
/// @param Type Hierarchy에 전달할 객체 타입
#define HIERARCHY(Class, Type)\
private:\
	struct HierarchyRegisterFactory\
	{\
		HierarchyRegisterFactory()\
		{\
			CustomHierarchyManager::GetInstance()->Register<Type, Class>();\
		}\
	} static inline hierarchyFactory{};

namespace sh::editor
{
	class EditorWorld;

	class ICustomHierarchy
	{
	public:
		virtual ~ICustomHierarchy() = default;

		virtual void OnHierarchyInstantiated(EditorWorld& world) {};
		virtual void OnHierarchyDraged(EditorWorld& world, const ImGuiPayload& payload) {};
	};

	class CustomHierarchyManager : public core::Singleton<CustomHierarchyManager>
	{
		friend class Hierarchy;
		friend core::Singleton<CustomHierarchyManager>;
	private:
		std::unordered_map<const core::reflection::STypeInfo*, std::unique_ptr<ICustomHierarchy>> map;

		CustomHierarchyManager() = default;
	public:
		template<typename T, typename U>
		void Register()
		{
			map.insert_or_assign(&T::GetStaticType(), std::make_unique<U>());
		}
	};
}//namespace