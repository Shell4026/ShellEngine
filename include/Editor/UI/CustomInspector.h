#pragma once
#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/Reflection.hpp"
#include "Core/SObject.h"

#include <type_traits>
#include <unordered_map>
#include <memory>

/// @brief ICustomInspector를 상속 후 클래스 선언의 내부에 넣으면 에디터 Inspector에서 어떻게 보여질지 정의 할 수 있다.
/// @brief void RenderUI(void* obj)를 오버라이딩 하고 그 함수에서 구현하면 된다.
/// @param Class 클래스
/// @param Type Inspector에 보여질 객체 타입
#define INSPECTOR(Class, Type)\
private:\
	struct InspectorRegisterFactory\
	{\
		InspectorRegisterFactory()\
		{\
			CustomInspectorManager::GetInstance()->Register<Type, Class>();\
		}\
	} static inline inspectorFactory{};

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
}//namespace