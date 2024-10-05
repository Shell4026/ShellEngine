#pragma once

#include "Export.h"

#include "SContainer.hpp"
#include "NonCopyable.h"

#include <string_view>
#include <functional>
#include <map>
#include <set>
#include <memory>
#include <stdint.h>

namespace sh::core
{
	/// @brief 리스너를 등록하면 특정 시점에 그 리스너의 함수를 호출하는 클래스.
	/// @tparam ...Args 리스너 매개변수
	template<typename... Args>
	class Observer : public INonCopyable
	{
	public:
		/// @brief 옵저버에 등록하여 특정 시점에 함수를 호출하는 클래스.
		/// 
		/// 옵저버에 등록된 상태라면 소멸 시 자동으로 옵저버에서 해제 된다.
		class Listener : public INonCopyable
		{
			friend Observer;
		private:
			std::function<void(Args...)> func;

			Observer* observer;
		public:
			const int priority;
		public:
			/// @brief 생성자
			/// @param func 호출 될 함수
			/// @param priority 우선 순위. 높을 수록 옵저버에서 우선 실행된다.
			Listener(const std::function<void(Args...)>& func, int priority = 0) :
				observer(nullptr), func(func), priority(priority)
			{}
			Listener(Listener&& other) noexcept :
				observer(other.observer), func(std::move(other.func)), priority(other.priority)
			{
				other.observer = nullptr;
			}
			~Listener()
			{
				if (observer == nullptr) 
					return;

				observer->UnRegister(*this);
			}

			bool operator<(const Listener& other) const
			{
				return priority > other.priority;
			}

			void Execute(Args... args)
			{
				func(args...);
			}
		};
	private:
		struct ListenerComparator
		{
			bool operator()(const Listener* left, const Listener* right) const 
			{
				return left->priority > right->priority; // 높은 우선순위가 먼저 오도록 설정 (내림차)
			}
		};

		core::SSet<Listener*, 4, ListenerComparator> events;
	public:
		Observer();
		~Observer();

		/// @brief 리스너를 등록하는 함수.
		/// @param listener 리스너
		void Register(Listener& listener);
		/// @brief 리스너를 해제하는 함수.
		/// @param listener 리스너
		/// @return 
		bool UnRegister(Listener& listener);

		/// @brief 모든 리스너에게 알리는 함수.
		/// @param ...args 리스너에게 전달 할 매개 변수
		void Notify(const Args&... args);

		/// @brief 등록된 리스너를 모두 제거하는 함수.
		void Clear();
	};//class



	template<typename ...Args>
	inline Observer<Args...>::Observer() :
		events()
	{
	}
	template<typename ...Args>
	Observer<Args...>::~Observer()
	{
		Clear();
	}
	template<typename ...Args>
	void Observer<Args...>::Register(Listener& event)
	{
		event.observer = this;
		events.insert(&event);
	}
	template<typename ...Args>
	bool Observer<Args...>::UnRegister(Listener& event)
	{
		auto it = events.find(&event);
		if (it == events.end())
			return false;

		event.observer = nullptr;
		events.erase(it);

		return true;
	}
	template<typename ...Args>
	void Observer<Args...>::Notify(const Args&... args)
	{
		for (auto& event : events)
		{
			event->Execute(args...);
		}
	}

	template<typename ...Args>
	inline void Observer<Args...>::Clear()
	{
		for (Listener* listener : events)
			listener->observer = nullptr;

		events.clear();
	}
}//namespace
