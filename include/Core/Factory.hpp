#pragma once
#include "Singleton.hpp"
#include "Reflection/TypeTraits.hpp"

#include <memory>
#include <unordered_map>
#include <string>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <optional>
namespace sh::core
{
	/// @brief 이름을 받고 등록된 객체를 반환 하는 팩토리 클래스.
	/// @tparam T 타입
	/// @tparam ReturnType 반환 타입
	template<typename T, typename ReturnType = std::unique_ptr<T>, typename Key = std::string>
	class Factory : public Singleton<Factory<T, ReturnType, Key>>
	{
		friend Singleton<Factory<T, ReturnType, Key>>;
	public:
		using AssetFactoryFn = std::function<ReturnType()>;
		using CreateReturnType =
			std::conditional_t<(std::is_pointer_v<ReturnType> || reflection::IsUniquePtr<ReturnType>::value), ReturnType, std::optional<ReturnType>>;
	private:
		std::unordered_map<Key, AssetFactoryFn> factories;
	protected:
		Factory() = default;
	public:
		virtual ~Factory() = default;

		auto HasKey(const Key& key) const  -> bool;
		void Register(const Key& key, AssetFactoryFn fn);
		void UnRegister(const Key& key);

		/// @brief 객체를 생성한다.
		/// @param name 이름
		/// @return 성공 시 ReturnType이 포인터 형식이면 그대로 반환, 아니라면 std::optional<ReturnType>을 반환한다.
		/// @return 실패 시 ReturnType이 포인터 형식이라면 nullptr, 아니라면 nullopt를 던진다.
		auto Create(const Key& key) const -> CreateReturnType;
	};
	template<typename T, typename ReturnType, typename Key>
	inline auto Factory<T, ReturnType, Key>::HasKey(const Key& key) const -> bool
	{
		auto it = factories.find(key);
		return it != factories.end();
	}
	template<typename T, typename ReturnType, typename Key>
	inline void Factory<T, ReturnType, Key>::Register(const Key& key, AssetFactoryFn fn)
	{
		factories.insert_or_assign(key, fn);
	}
	template<typename T, typename ReturnType, typename Key>
	inline void Factory<T, ReturnType, Key>::UnRegister(const Key& key)
	{
		factories.erase(key);
	}
	template<typename T, typename ReturnType, typename Key>
	inline auto Factory<T, ReturnType, Key>::Create(const Key& key) const -> CreateReturnType
	{
		auto it = factories.find(key);
		if (it == factories.end())
		{
			if constexpr (std::is_pointer_v<ReturnType> || reflection::IsUniquePtr<ReturnType>::value)
				return nullptr;
			else
				return {};
		}
		return it->second();
	}
}//namespace