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
        using ConstVecIterator = typename VectorType::const_iterator;

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
        class ConstIterator
        {
        private:
            ConstVecIterator itVec;
            ConstVecIterator itVecEnd;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const std::pair<const KeyT*, ValueT>;
            using difference_type = std::ptrdiff_t;
            using pointer = const value_type*;
            using reference = const value_type&;

            ConstIterator(ConstVecIterator it, ConstVecIterator endIt) :
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
        auto begin() const -> ConstIterator
        {
            return ConstIterator{ vec.begin(), vec.end() };
        }

        auto end() -> Iterator
        {
            return Iterator{ vec.end(), vec.end() };
        }
        auto end() const -> ConstIterator
        {
            return ConstIterator{ vec.end(), vec.end() };
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
        using ConstVecIterator = typename core::SVector<std::optional<T>>::const_iterator;
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

            Iterator(const VecIterator& it, const VecIterator& endIt) :
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
        class ConstIterator
        {
        private:
            ConstVecIterator itVec;
            ConstVecIterator itVecEnd;
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = const T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            ConstIterator(ConstVecIterator it, ConstVecIterator endIt) :
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
        auto begin() const -> ConstIterator
        {
            return ConstIterator(vec.begin(), vec.end());
        }

        auto end() -> Iterator
        {
            return Iterator{ vec.end(), vec.end() };
        }
        auto end() const -> ConstIterator
        {
            return ConstIterator{ vec.end(), vec.end() };
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