#pragma once

#include "Export.h"

#include "SContainer.hpp"
#include "NonCopyable.h"
#include "Logger.h"

#include <string_view>
#include <functional>
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

			std::set<Observer*> observers;
		public:
			const int priority;
		public:
			Listener(int priority = 0) : priority(priority) {};
			/// @brief 생성자
			/// @param func 호출 될 함수
			/// @param priority 우선 순위. 높을 수록 옵저버에서 우선 실행된다.
			Listener(const std::function<void(Args...)>& func, int priority = 0) :
				func(func), priority(priority)
			{}
			/// @brief 주의) 콜백 함수 재설정 권장
			/// @param other 다른 Listener 객체
			Listener(Listener&& other) noexcept:
				observers(std::move(other.observers)), func(std::move(other.func)), priority(other.priority)
			{
				for (auto observer : observers)
				{
					observer->UnRegister(other);
					observer->Register(*this);
				}
			}
			~Listener()
			{
				if (observers.empty())
					return;
				// UnRegister하는 순간 observers의 반복자가 깨지기 때문에 이런식으로 처리
				std::vector<Observer*> tmp;
				tmp.reserve(observers.size());
				for (auto observer : observers)
					tmp.push_back(observer);
				for (auto observer : tmp)
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
				if (func)
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

		std::set<Listener*, ListenerComparator> listeners;
	public:
		Observer();
		Observer(Observer&& other) noexcept;
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
		listeners()
	{
	}
	template<bool OneTimeListener, typename ...Args>
	inline Observer<OneTimeListener, Args...>::Observer(Observer&& other) noexcept :
		listeners(std::move(other.listeners))
	{
		for (Listener* listener : listeners)
		{
			if (listener)
			{
				listener->observers.erase(&other);
				listener->observers.insert(this);
			}
		}
	}
	template<bool OneTimeListener, typename ...Args>
	Observer<OneTimeListener, Args...>::~Observer()
	{
		for (Listener* listener : listeners)
		{
			if (listener)
				listener->observers.erase(this);
		}
	}
	template<bool OneTimeListener, typename ...Args>
	void Observer<OneTimeListener, Args...>::Register(Listener& event)
	{
		event.observers.insert(this);
		listeners.insert(&event);
	}
	template<bool OneTimeListener, typename ...Args>
	bool Observer<OneTimeListener, Args...>::UnRegister(Listener& event)
	{
		auto it = listeners.find(&event);
		if (it == listeners.end())
			return false;

		event.observers.erase(this);
		listeners.erase(it);

		return true;
	}
	template<bool OneTimeListener, typename ...Args>
	void Observer<OneTimeListener, Args...>::Notify(const Args&... args)
	{
		for (auto& event : listeners)
		{
			event->Execute(args...);
		}
		if constexpr (OneTimeListener)
			Clear();
	}

	template<bool OneTimeListener, typename ...Args>
	inline void Observer<OneTimeListener, Args...>::Clear()
	{
		for (Listener* listener : listeners)
		{
			listener->observers.erase(this);
		}
		listeners.clear();
	}

	template<bool OneTimeListener, typename ...Args>
	inline bool Observer<OneTimeListener, Args...>::Empty()
	{
		return listeners.empty();
	}
}//namespace
