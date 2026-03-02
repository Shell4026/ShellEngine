#pragma once

#include <atomic>
#include <utility>
#include <functional>
namespace sh::core
{
	/// @brief Lock-free MPSC큐. 생산자는 여러 스레드에서 Push를 하고, 소모자는 하나의 스레드에서 Drain을 한다.
	/// @brief [상세] Push 시 현재 헤드 값을 가져오고 추가 할 노드의 next를 그 헤드로 변경한다.
	/// @brief 만일 다른 스레드에서 헤드를 변경 했다면 바뀐 헤드로 재시도.
	/// @brief 헤드는 Drain에서도 바뀌지만 Push에서 CAS로 비교하므로 스레드 안전함.
	template<typename T>
	class LockFreeMPSCQueue
	{
	private:
		struct Node
		{
			T value;
			Node* next;

			explicit Node(const T& value) :
				value(value), next(nullptr)
			{
			}
			explicit Node(T&& value) :
				value(std::move(value)), next(nullptr)
			{
			}
		};
	private:
		std::atomic<Node*> head = nullptr;
	public:
		LockFreeMPSCQueue() = default;
		LockFreeMPSCQueue(const LockFreeMPSCQueue&) = delete;
		auto operator=(const LockFreeMPSCQueue&) -> LockFreeMPSCQueue& = delete;

		~LockFreeMPSCQueue()
		{
			Clear();
		}

		void Push(const T& value)
		{
			Node* node = new Node(value);
			Node* oldHead = head.load(std::memory_order_relaxed);
			do
			{
				node->next = oldHead;
			}
			while (!head.compare_exchange_weak(
				oldHead,
				node,
				std::memory_order_release,
				std::memory_order_relaxed));
		}
		void Push(T&& value)
		{
			Node* node = new Node(std::move(value));
			Node* oldHead = head.load(std::memory_order_relaxed);
			do
			{
				node->next = oldHead;
			}
			while (!head.compare_exchange_weak(
				oldHead,
				node,
				std::memory_order_release,
				std::memory_order_relaxed));
		}

		void Drain(std::function<void(T&)>&& fn)
		{
			Node* list = head.exchange(nullptr, std::memory_order_acquire);
			list = Reverse(list);

			while (list != nullptr)
			{
				Node* cur = list;
				list = list->next;
				fn(cur->value);
				delete cur;
			}
		}

		void Clear()
		{
			Drain([](T&) {});
		}
	private:
		static auto Reverse(Node* node) -> Node*
		{
			Node* prev = nullptr;
			while (node != nullptr)
			{
				Node* next = node->next;
				node->next = prev;
				prev = node;
				node = next;
			}
			return prev;
		}
	};
}//namespace