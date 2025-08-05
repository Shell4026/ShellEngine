#pragma once
#include "Singleton.hpp"
#include "Reflection/TypeTraits.hpp"

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>
namespace sh::core
{
	/// @brief 이름을 받고 등록된 객체를 반환 하는 팩토리 클래스.
	/// @tparam T 타입
	/// @tparam ReturnType 반환 타입
	template<typename T, typename ReturnType = std::unique_ptr<T>>
	class Factory : public Singleton<Factory<T, ReturnType>>
	{
		friend Singleton<Factory<T, ReturnType>>;
	public:
		using AssetFactoryFn = std::function<ReturnType()>;
	private:
		std::unordered_map<std::string, AssetFactoryFn> factories;
	protected:
		Factory() = default;
	public:
		virtual ~Factory() = default;

		void Register(const std::string& name, AssetFactoryFn fn);
		void UnRegister(const std::string& name);

		/// @brief 객체를 생성한다.
		/// @param name 이름
		/// @return 성공 시 ReturnType 반환
		/// @return 실패 시 ReturnType이 포인터 형식이라면 nullptr, 아니라면 예외를 던진다.
		auto Create(const std::string& name) const -> ReturnType;
	};
	template<typename T, typename ReturnType>
	inline void Factory<T, ReturnType>::Register(const std::string& name, AssetFactoryFn fn)
	{
		factories.insert_or_assign(name, fn);
	}
	template<typename T, typename ReturnType>
	inline void Factory<T, ReturnType>::UnRegister(const std::string& name)
	{
		factories.erase(name);
	}
	template<typename T, typename ReturnType>
	inline auto Factory<T, ReturnType>::Create(const std::string& name) const -> ReturnType
	{
		auto it = factories.find(name);
		if (it == factories.end())
		{
			if constexpr (std::is_pointer_v<ReturnType> || reflection::IsUniquePtr<ReturnType>::value)
				return nullptr;
			else
				throw std::runtime_error{ "Unregistered object!" };
		}
		return it->second();
	}
}//namespace