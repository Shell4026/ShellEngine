#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/Reflection.hpp"

#include "imgui.h"

#include <unordered_map>
#include <memory>
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

	/// @brief 해당 클래스를 상속 하면 에디터 Hierarchy의 행동을 정의 할 수 있다.
	/// @tparam Hierarchy 해당 클래스를 상속한 자식의 타입
	/// @tparam T SObject 타입
	/// @tparam IsSObject 
	template<typename Hierarchy, typename T, typename IsSObject = std::enable_if_t<core::reflection::IsSObject<T>::value>>
	class CustomHierarchy : public ICustomHierarchy
	{
	private:
		struct RegisterFactory
		{
			RegisterFactory()
			{
				CustomHierarchyManager::GetInstance()->Register<T, Hierarchy>();
			}
		} static inline factory{};
	};
}//namespace