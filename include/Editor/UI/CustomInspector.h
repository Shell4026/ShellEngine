#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/Reflection.hpp"
#include "Core/SObject.h"

#include <type_traits>
#include <unordered_map>
#include <memory>
namespace sh::editor
{
	struct ICustomInspector
	{
		virtual ~ICustomInspector() = default;
		virtual void RenderUI(void* obj) = 0;
	};

	class CustomInspectorManager : public core::Singleton<CustomInspectorManager>
	{
	private:
		std::unordered_map<const core::reflection::STypeInfo*, std::unique_ptr<ICustomInspector>> map;
	public:
		template<typename T, typename U>
		void Register()
		{
			map.insert_or_assign(&T::GetStaticType(), std::make_unique<U>());
		}

		SH_EDITOR_API auto GetCustomInspector(const core::reflection::STypeInfo* type) const -> ICustomInspector*;
	};

	template<typename T, typename U>
	struct CustomInspectorRegister
	{
		CustomInspectorRegister()
		{
			CustomInspectorManager::GetInstance()->Register<T, U>();
		}
	};

	/// @brief 해당 클래스를 상속 하면 에디터 Inspector에서 어떻게 보여질지 정의 할 수 있다.
	/// @brief void RenderUI(void* obj)를 오버라이딩 하고 그 함수에서 구현하면 된다.
	/// @tparam Inspector 해당 클래스를 상속한 자식의 타입
	/// @tparam T SObject 타입
	/// @tparam IsSObject 
	template<typename Inspector, typename T, typename IsSObject = std::enable_if_t<core::reflection::IsSObject<T>::value>>
	class CustomInspector : public ICustomInspector
	{
	private:
		static inline CustomInspectorRegister<T, Inspector> inspectorRegister{};
	public:
		void RenderUI(void* obj) override 
		{
		}
	};
}//namespace