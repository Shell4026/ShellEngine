#pragma once

#include "Memory/SAllocator.hpp"

#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <optional>
#include <queue>
#include <assert.h>
#include <stdexcept>

namespace sh::core
{
	template<typename T>
	using SVector = std::vector<T>;

	template<typename T, std::size_t defaultSize = 8, typename _Pr = std::less<T>>
	using SSet = std::set<T, _Pr, memory::SAllocator<T, defaultSize>>;

	template<typename T, std::size_t defaultSize = 8, typename _hasher = std::hash<T>, typename _Keyeq = std::equal_to<T>>
	using SHashSet = std::unordered_set<T, _hasher, _Keyeq, memory::SAllocator<T, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 8, typename _Pr = std::less<KeyT>>
	using SMap = std::map<KeyT, T, _Pr, memory::SAllocator<std::pair<const KeyT, T>, defaultSize>>;

	template<typename KeyT, typename T, std::size_t defaultSize = 8, typename _hasher = std::hash<KeyT>, typename _Keyeq = std::equal_to<KeyT>>
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

    /// @brief 연속된 메모리를 가지는 해쉬맵 + 벡터 컨테이너.
    /// @brief 검색: O(log N) 삽입: O(log N) 삭제: O(log N).
    /// @brief 삭제 시에는 벡터의 메모리를 해제 하지 않고 {}값으로 남으며 삽입시 그 메모리를 재활용한다.
    /// @tparam KeyT 키 타입
    /// @tparam ValueT 값 타입
    /// @tparam Hasher 해쉬 구조체
    /// @tparam KeyEQ 동등 비교 구조체
    /// @tparam CleanSize 삭제시 이 만큼의 빈 공간이 있으면 메모리 재배치
    template<typename KeyT, typename ValueT, typename Hasher = std::hash<KeyT>, typename KeyEq = std::equal_to<KeyT>, std::size_t CleanSize = 32>
    class SHashMapVector
    {
    private:
        using VectorElementType = std::optional<std::pair<const KeyT*, ValueT>>;
        using VectorType = core::SVector<VectorElementType>;
        using MapType = core::SHashMap<KeyT, std::size_t, CleanSize, Hasher>;
        using VecIterator = typename VectorType::iterator;

        MapType hashMap;
        VectorType vec;
        std::stack<std::size_t> emptyIdx;
    public:
        class Iterator
        {
        private:
            VecIterator itVec;
            VecIterator itVecEnd;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = std::pair<const KeyT*, ValueT>;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type&;

            Iterator(VecIterator it, VecIterator endIt) :
                itVec(it), itVecEnd(endIt)
            {
                while (itVec != itVecEnd && !itVec->has_value())
                    ++itVec;
            }

            auto operator*() const -> reference
            {
                return itVec->value();
            }
            auto operator->() const -> pointer
            {
                return &(itVec->value());
            }

            auto operator++() -> Iterator&
            {
                ++itVec;
                while (itVec != itVecEnd && !itVec->has_value())
                    ++itVec;
                return *this;
            }
            auto operator++(int) -> Iterator
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            auto operator--() -> Iterator&
            {
                do 
                {
                    if (itVec == vec.begin())
                        break;
                    --itVec;
                } while (!itVec->has_value());

                return *this;
            }
            auto operator--(int) -> Iterator
            {
                Iterator tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const { return itVec == other.itVec; }
            bool operator!=(const Iterator& other) const { return itVec != other.itVec; }
        };
    private:
        void CleanMemory()
        {
            auto it = std::remove(vec.begin(), vec.end(), std::nullopt);
            vec.erase(it, vec.end());
            for (std::size_t i = 0; i < vec.size(); ++i)
                hashMap[*vec[i]->first] = i;
            while (!emptyIdx.empty())
                emptyIdx.pop();
        }
    public:
        bool Insert(const KeyT& key, const ValueT& value)
        {
            if (hashMap.find(key) != hashMap.end())
                return false;

            std::size_t idx;
            if (emptyIdx.empty())
            {
                idx = vec.size();
                auto result = hashMap.insert({ key, idx });
                vec.push_back(std::make_pair(&result.first->first, value));
            }
            else
            {
                idx = emptyIdx.top();
                auto result = hashMap.insert({ key, idx });
                vec[idx] = { &result.first->first, value };
                emptyIdx.pop();
            }
            return true;
        }
        bool Erase(const KeyT& key)
        {
            if (hashMap.empty())
                return false;

            auto it = hashMap.find(key);
            if (it == hashMap.end())
                return false;

            vec[it->second].reset();
            emptyIdx.push(it->second);
            hashMap.erase(it);
            if (emptyIdx.size() >= CleanSize)
                CleanMemory();

            return true;
        }
        bool Erase(const Iterator& it)
        {
            return Erase(*it->first);
        }
        auto Find(const KeyT& key) -> Iterator
        {
            auto it = hashMap.find(key);
            if (it == hashMap.end())
                return end();
            return Iterator{ vec.begin() + it->second, vec.end() };
        }
        void Clear()
        {
            hashMap.clear();
            vec.clear();
            while (!emptyIdx.empty())
                emptyIdx.pop();
        }

        auto begin() -> Iterator
        {
            return Iterator{ vec.begin(), vec.end() };
        }

        auto end() -> Iterator
        {
            return Iterator{ vec.end(), vec.end() };
        }

        auto operator[](std::size_t idx) -> std::optional<std::pair<KeyT*, ValueT>>&
        {
            if (idx >= vec.size())
                throw std::out_of_range{};
            assert(idx < vec.size());
            return vec[idx];
        }

        auto Size() const -> std::size_t
        {
            return hashMap.size();
        }
        auto AllocatedSize() const -> std::size_t
        {
            return vec.size();
        }
    };

    /// @brief 연속된 메모리를 가지는 해쉬셋 + 벡터 컨테이너.
    /// @brief 검색: O(log N) 삽입: O(log N) 삭제: O(log N).
    /// @brief 삭제 시에는 벡터의 메모리를 해제 하지 않고 {}값으로 남으며 삽입시 그 메모리를 재활용한다.
    /// @tparam T 타입
    /// @tparam Hasher 해쉬 구조체
    /// @tparam KeyEQ 동등 비교 구조체
    /// @tparam CleanSize 삭제시 이 만큼의 빈 공간이 있으면 메모리 재배치
    template<typename T, typename Hasher = std::hash<T>, typename KeyEq = std::equal_to<T>, std::size_t CleanSize = 32>
    class SHashSetVector
    {
    private:
        using VecIterator = typename core::SVector<std::optional<T>>::iterator;
        using VectorType = core::SVector<std::optional<T>>;
        using MapType = core::SHashMap<T, std::size_t, CleanSize, Hasher>;

        MapType hashMapC;
        VectorType vec;
        std::stack<std::size_t> emptyIdx;
    public:
        class Iterator 
        {
        private:
            VecIterator itVec;
            VecIterator itVecEnd;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            Iterator(VecIterator it, VecIterator endIt) :
                itVec(it), itVecEnd(endIt)
            {
                while (itVec != itVecEnd && !itVec->has_value())
                    ++itVec;
            }

            auto operator*() const -> reference
            { 
                return itVec->value(); 
            }
            auto operator->() const -> pointer
            { 
                return &(itVec->value());
            }

            auto operator++() -> Iterator&
            {
                ++itVec;
                while (itVec != itVecEnd && !itVec->has_value())
                    ++itVec;
                return *this;
            }
            auto operator++(int) -> Iterator
            {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }
            auto operator--() -> Iterator&
            {
                do
                {
                    if (itVec == vec.begin())
                        break;
                    --itVec;
                } while (!itVec->has_value());

                return *this;
            }
            auto operator--(int) -> Iterator
            {
                Iterator tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const { return itVec == other.itVec; }
            bool operator!=(const Iterator& other) const { return itVec != other.itVec; }
        };
    private:
        void CleanMemory()
        {
            auto it = std::remove(vec.begin(), vec.end(), std::nullopt);
            vec.erase(it, vec.end());
            for (std::size_t i = 0; i < vec.size(); ++i)
                hashMapC[vec[i].value()] = i;
            while (!emptyIdx.empty())
                emptyIdx.pop();
        }
    public:
        bool Insert(const T& value)
        {
            if (hashMapC.find(value) != hashMapC.end())
                return false;

            std::size_t idx;
            if (emptyIdx.empty())
            {
                idx = vec.size();
                vec.push_back(value);
            }
            else
            {
                idx = emptyIdx.top();
                vec[idx] = value;
                emptyIdx.pop();
            }
            auto result = hashMapC.insert({ value, idx });
            return result.second;
        }
        bool Erase(const T& value)
        {
            if (hashMapC.empty())
                return false;

            auto it = hashMapC.find(value);
            if (it == hashMapC.end())
                return false;

            vec[it->second].reset();
            emptyIdx.push(it->second);
            hashMapC.erase(it);
            if (emptyIdx.size() >= CleanSize)
                CleanMemory();

            return true;
        }
        bool Erase(const Iterator& it)
        {
            return Erase(*it);
        }
        auto Find(const T& value) -> Iterator
        {
            auto it = hashMapC.find(value);
            if (it == hashMapC.end())
                return end();
            return Iterator{ vec.begin() + it->second, vec.end() };
        }
        void Clear()
        {
            hashMapC.clear();
            vec.clear();
            while (!emptyIdx.empty())
                emptyIdx.pop();
        }

        auto begin() -> Iterator
        {
            return Iterator(vec.begin(), vec.end());
        }

        auto end() -> Iterator
        {
            return Iterator(vec.end(), vec.end());
        }

        auto operator[](std::size_t idx) -> std::optional<T>&
        {
            if (idx >= vec.size())
                throw std::out_of_range{};
            assert(idx < vec.size());
            return vec[idx];
        }

        auto Size() const -> std::size_t
        {
            return hashMapC.size();
        }
        auto AllocatedSize() const -> std::size_t
        {
            return vec.size();
        }
    };
}//namespace