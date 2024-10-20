#pragma once

#include "Export.h"

#include "SContainer.hpp"
#include "NonCopyable.h"
#include "Logger.h"

#include <string_view>
#include <functional>
#include <map>
#include <set>
#include <memory>
#include <stdint.h>

namespace sh::core
{
	/// @brief 리스너를 등록하면 특정 시점에 그 리스너의 함수를 호출하는 클래스.
	/// @tparam OneTimeListener Notify 후 리스너가 제거 되는지
	/// @tparam ...Args 리스너 매개변수
	template<bool OneTimeListener, typename... Args>
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

			Observer* observer = nullptr;
		public:
			const int priority;
		public:
			Listener(int priority = 0) : priority(priority) {};
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

			void SetCallback(const std::function<void(Args...)>& func)
			{
				this->func = func;
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
				if (left->priority == right->priority)
					return left < right;
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

		/// @brief 리스너가 없는지 반환 하는 함수
		/// @return 없으면 true, 있으면 false
		bool Empty();
	};//class



	template<bool OneTimeListener, typename ...Args>
	inline Observer<OneTimeListener, Args...>::Observer() :
		events()
	{
	}
	template<bool OneTimeListener, typename ...Args>
	Observer<OneTimeListener, Args...>::~Observer()
	{
		for (Listener* listener : events)
		{
			if (listener)
				listener->observer = nullptr;
		}
	}
	template<bool OneTimeListener, typename ...Args>
	void Observer<OneTimeListener, Args...>::Register(Listener& event)
	{
		event.observer = this;
		events.insert(&event);
	}
	template<bool OneTimeListener, typename ...Args>
	bool Observer<OneTimeListener, Args...>::UnRegister(Listener& event)
	{
		auto it = events.find(&event);
		if (it == events.end())
			return false;

		event.observer = nullptr;
		events.erase(it);

		return true;
	}
	template<bool OneTimeListener, typename ...Args>
	void Observer<OneTimeListener, Args...>::Notify(const Args&... args)
	{
		for (auto& event : events)
		{
			event->Execute(args...);
		}
		if constexpr (OneTimeListener)
			Clear();
	}

	template<bool OneTimeListener, typename ...Args>
	inline void Observer<OneTimeListener, Args...>::Clear()
	{
		for (Listener* listener : events)
		{
			listener->observer = nullptr;
		}
		events.clear();
	}

	template<bool OneTimeListener, typename ...Args>
	inline bool Observer<OneTimeListener, Args...>::Empty()
	{
		return events.empty();
	}
}//namespace
