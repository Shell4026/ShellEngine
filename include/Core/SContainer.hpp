#pragma once

#include "Memory/SAllocator.hpp"

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace sh::core
{
	template<typename T>
	using SVector = std::vector<T>;

	template<typename T, std::size_t defaultSize = 32, typename _Pr = std::less<T>>
	using SSet = std::set<T, _Pr, memory::SAllocator<T, defaultSize>>;

	template<typename T, std::size_t defaultSize = 32, typename _hasher = std::hash<T>, typename _Keyeq = std::equal_to<T>>
	using SHashSet = std::unordered_set<T, _hasher, _Keyeq, memory::SAllocator<T, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 32, typename _Pr = std::less<KeyT>>
	using SMap = std::map<KeyT, T, _Pr, memory::SAllocator<std::pair<const KeyT, T>, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 32, typename _hasher = std::hash<KeyT>, typename _Keyeq = std::equal_to<KeyT>>
	using SHashMap = std::unordered_map<KeyT, T, _hasher, _Keyeq, memory::SAllocator<std::pair<const KeyT, T>, defaultSize>>;

    /// @brief 잠금을 쓰지 않는 원자적 큐
    /// @tparam T 타입
    template <typename T>
    class LockFreeQueue 
    {
    private:
        struct Node 
        {
            T data;
            std::atomic<Node*> next;

            Node(const T& data) : data(data), next(nullptr) {}
        };

        std::atomic<Node*> head;
        std::atomic<Node*> tail;
    public:
        LockFreeQueue() 
        {
            Node* dummy = new Node(T{});
            //처음엔 헤드와 테일이 같다.
            head.store(dummy, std::memory_order::memory_order_relaxed);
            tail.store(dummy, std::memory_order::memory_order_relaxed);
        }

        ~LockFreeQueue() 
        {
            while (Node* node = head.load()) 
            {
                head.store(node->next);
                delete node;
            }
        }

        /// @brief 큐에 데이터를 삽입한다.
        /// @param value 삽입 할 값
        /// @return 성공하면 true, 그 외 false(정의 안 됨)
        bool Enqueue(const T& value) 
        {
            Node* newNode = new Node(value);
            Node* oldTail;

            while (true) 
            {
                //이 시점에서 tail을 읽어오지만, 이 값은 정확히 최신 상태일 필요는 없다. (memory_order_relaxed)
                //나중에 compare_exchange_weak를 사용해 제대로 동작할지 확인.
                oldTail = tail.load(std::memory_order_relaxed);
                Node* next = oldTail->next.load(std::memory_order_acquire);

                //old_tail이 현재 큐의 마지막 노드다.
                if (next == nullptr) 
                {
                    //여전히 oldTail의 다음 값이 next와 같다면 new node를 삽입 후 break으로 탈출한다.
                    if (oldTail->next.compare_exchange_weak(next, newNode, std::memory_order_release))
                    {
                        break;
                    }
                }
                //다른 스레드가 이미 새로운 노드를 old_tail 뒤에 추가한 상태
                else 
                {
                    //tail == oldTail이라면 next로
                    tail.compare_exchange_weak(oldTail, next, std::memory_order_release);
                }
            }

            tail.compare_exchange_weak(oldTail, newNode, std::memory_order_release);
            return true;
        }

        /// @brief 큐에서 데이터를 뺀다.
        /// @param result 뺀 데이터를 받을 참조
        /// @return 성공하면 true, 큐가 비었으면 false
        bool Dequeue(T& result) 
        {
            Node* oldHead;

            while (true) 
            {
                oldHead = head.load(std::memory_order_relaxed);
                Node* oldTail = tail.load(std::memory_order_relaxed);
                Node* next = oldHead->next.load(std::memory_order_acquire);

                if (oldHead == oldTail) 
                {
                    if (next == nullptr) 
                    {
                        //큐가 빔
                        return false;
                    }
                    tail.compare_exchange_weak(oldTail, next, std::memory_order_release);
                }
                else 
                {
                    if (next != nullptr) 
                    {
                        result = std::move(next->data);
                        if (head.compare_exchange_weak(oldHead, next, std::memory_order_release)) 
                        {
                            break;
                        }
                    }
                }
            }

            delete oldHead; // 기존의 head 노드는 삭제
            return true;
        }
    };
}//namespace