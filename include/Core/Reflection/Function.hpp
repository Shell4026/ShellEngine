#pragma once
#include "TypeTraits.hpp"
#include "TypeInfo.hpp"
#include "../Name.h"

#include <string_view>
#include <initializer_list>
#include <type_traits>
#include <tuple>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <cassert>

#define SFUNCTION(function_name, ...)\
struct _FunctionFactory_##function_name\
{\
    _FunctionFactory_##function_name()\
    {\
        sh::core::reflection::FunctionCreateInfo<\
            This,\
            decltype(&This::function_name),\
            &This::function_name\
        > fn_##function_name{ #function_name, {__VA_ARGS__} };\
        GetStaticType().AddFunction(sh::core::reflection::Function{ fn_##function_name });\
    }\
};\
inline static _FunctionFactory_##function_name _functionFactory_##function_name{};

namespace sh::core
{
    struct FunctionOption
    {
        static constexpr const char* invisible = "invisible";
        static constexpr const char* constant = "const";
        static constexpr const char* sync = "sync";
    };
}

namespace sh::core::reflection
{
    template<typename T>
    struct FunctionTraits;

    template<typename C, typename R, typename... Args>
    struct FunctionTraits<R(C::*)(Args...)>
    {
        using Class = C;
        using Return = R;
        using ArgsTuple = std::tuple<Args...>;
        static constexpr bool isMember = true;
        static constexpr bool isConst = false;
    };

    template<typename C, typename R, typename... Args>
    struct FunctionTraits<R(C::*)(Args...) const>
    {
        using Class = C;
        using Return = R;
        using ArgsTuple = std::tuple<Args...>;
        static constexpr bool isMember = true;
        static constexpr bool isConst = true;
    };

    template<typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)>
    {
        using Class = void;
        using Return = R;
        using ArgsTuple = std::tuple<Args...>;
        static constexpr bool isMember = false;
        static constexpr bool isConst = false;
    };

    class FunctionDataBase
    {
    public:
        virtual ~FunctionDataBase() = default;
        /// @brief 함수를 실행한다.
        /// @param obj 멤버함수면 this 포인터, static이면 무시 가능
        /// @param args 각 인자 주소 배열 (정확한 타입 주소여야 함)
        /// @param ret 반환값 저장 메모리(반환이 void면 nullptr 가능)
        virtual void Invoke(void* obj, void** args, void* ret) const = 0;
    };

    template<typename ThisType, typename FnPtrType, FnPtrType ptr>
    class FunctionData;

    template<typename ThisType, typename R, typename... Args, R(ThisType::* ptr)(Args...)>
    class FunctionData<ThisType, R(ThisType::*)(Args...), ptr> final : public FunctionDataBase
    {
    public:
        void Invoke(void* obj, void** args, void* ret) const override
        {
            ThisType* self = static_cast<ThisType*>(obj);
            assert(self != nullptr);

            InvokeImpl(self, args, ret, std::index_sequence_for<Args...>{});
        }
    private:
        template<std::size_t... I>
        static void InvokeImpl(ThisType* self, void** args, void* ret, std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<R>)
            {
                std::invoke(ptr, *self, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
            }
            else
            {
                R result = std::invoke(ptr, *self, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
                assert(ret != nullptr);
                *reinterpret_cast<R*>(ret) = std::move(result);
            }
        }
    };

    template<typename ThisType, typename R, typename... Args, R(ThisType::* ptr)(Args...) const>
    class FunctionData<ThisType, R(ThisType::*)(Args...) const, ptr> final : public FunctionDataBase
    {
    public:
        void Invoke(void* obj, void** args, void* ret) const override
        {
            const ThisType* self = static_cast<const ThisType*>(obj);
            assert(self != nullptr);

            InvokeImpl(self, args, ret, std::index_sequence_for<Args...>{});
        }

    private:
        template<std::size_t... I>
        static void InvokeImpl(const ThisType* self, void** args, void* ret, std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<R>)
            {
                std::invoke(ptr, *self, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
            }
            else
            {
                R result = std::invoke(ptr, *self, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
                assert(ret != nullptr);
                *reinterpret_cast<R*>(ret) = std::move(result);
            }
        }
    };

    template<typename ThisType, typename R, typename... Args, R(*ptr)(Args...)>
    class FunctionData<ThisType, R(*)(Args...), ptr> final : public FunctionDataBase
    {
    public:
        void Invoke(void* /*obj*/, void** args, void* ret) const override
        {
            InvokeImpl(args, ret, std::index_sequence_for<Args...>{});
        }

    private:
        template<std::size_t... I>
        static void InvokeImpl(void** args, void* ret, std::index_sequence<I...>)
        {
            if constexpr (std::is_void_v<R>)
            {
                std::invoke(ptr, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
            }
            else
            {
                R result = std::invoke(ptr, (*reinterpret_cast<std::remove_reference_t<Args>*>(args[I]))...);
                assert(ret != nullptr);
                *reinterpret_cast<R*>(ret) = std::move(result);
            }
        }
    };

    template<typename ThisType, typename FnPtrType, FnPtrType ptr>
    struct FunctionCreateInfo
    {
        struct Option
        {
            bool bVisible = true;
            bool bConstFlag = false;
            bool bSync = false;
        } option;

        const std::string_view name;

        static Option ParseOption(const std::initializer_list<std::string_view>& options)
        {
            Option o{};
            for (auto& s : options)
            {
                if (s == sh::core::FunctionOption::invisible)
                    o.bVisible = false;
                else if (s == sh::core::FunctionOption::constant)
                    o.bConstFlag = true;
                else if (s == sh::core::FunctionOption::sync)  
                    o.bSync = true;
            }
            return o;
        }

        FunctionCreateInfo(std::string_view name, const std::initializer_list<std::string_view>& options)
            : option(ParseOption(options)), name(name)
        {}
    };

    class Function
    {
    public:
        template<typename ThisType, typename FnPtrType, FnPtrType ptr>
        Function(const FunctionCreateInfo<ThisType, FnPtrType, ptr>& ci) :
            name(ci.name),
            returnType(GetType<typename FunctionTraits<FnPtrType>::Return>()),
            bVisible(ci.option.bVisible),
            bSync(ci.option.bSync),
            isMember(FunctionTraits<FnPtrType>::isMember),
            isConstMethod(FunctionTraits<FnPtrType>::isConst)
        {
            using Traits = FunctionTraits<FnPtrType>;
            FillParamTypes<typename Traits::ArgsTuple>();

            static FunctionData<ThisType, FnPtrType, ptr> data{};
            this->data = &data;
        }
        auto GetName() const -> const core::Name& { return name; }
        auto GetReturnType() const -> const TypeInfo& { return returnType; }
        auto GetParamTypes() const -> const std::vector<const TypeInfo*>& { return paramTypes; }
        auto IsMember() const -> bool { return isMember; }
        auto IsConstMethod() const -> bool { return isConstMethod; }
        auto IsVisible() const -> bool { return bVisible; }
        auto IsSync() const -> bool { return bSync; }

        template<typename R, typename Obj, typename... Args>
        auto Invoke(Obj& obj, Args&&... args) const -> R
        {
            // R이 void면 호출만
            if constexpr (std::is_void_v<R>)
            {
                InvokeVoid(obj, std::forward<Args>(args)...);
                return;
            }
            else
            {
                R ret{};
                InvokeImpl(obj, &ret, std::forward<Args>(args)...);
                return ret;
            }
        }

        template<typename Obj, typename... Args>
        void InvokeVoid(Obj& obj, Args&&... args) const
        {
            InvokeImpl(obj, nullptr, std::forward<Args>(args)...);
        }

        void InvokeRaw(void* obj, void** args, void* ret) const
        {
            data->Invoke(obj, args, ret);
        }
    private:
        template<typename Tuple, std::size_t... I>
        void FillParamTypesImpl(std::index_sequence<I...>)
        {
            (paramTypes.push_back(&GetType<std::remove_cv_t<std::remove_reference_t<std::tuple_element_t<I, Tuple>>>>()), ...);
        }

        template<typename Tuple>
        void FillParamTypes()
        {
            constexpr std::size_t N = std::tuple_size_v<Tuple>;
            paramTypes.reserve(N);
            FillParamTypesImpl<Tuple>(std::make_index_sequence<N>{});
        }

        template<typename Obj, typename... Args>
        void InvokeImpl(Obj& obj, void* ret, Args&&... args) const
        {
            // const 메서드가 아닌데 const obj로 호출하려는 경우 방지
            if constexpr (std::is_const_v<std::remove_reference_t<Obj>>)
            {
                assert(isConstMethod && "Calling non-const method with const object!");
            }

            if constexpr (sizeof...(Args) == 0)
                data->Invoke((void*)std::addressof(obj), nullptr, ret);
            else
            {
                void* argv[] = { (void*)std::addressof(args)... };
                data->Invoke((void*)std::addressof(obj), argv, ret);
            }
        }
    private:
        core::Name name;
        const TypeInfo& returnType;
        std::vector<const TypeInfo*> paramTypes;

        bool bVisible = true;
        bool bSync = false;

        bool isMember = true;
        bool isConstMethod = false;

        const FunctionDataBase* data = nullptr;
    };
}//namespace