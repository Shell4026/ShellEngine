#pragma once
#include "Export.h"
#include "ShaderEnum.h"
#include "Texture.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"
#include "Core/ISerializable.h"

#include <glm/fwd.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>
namespace sh::render
{
	/// @brief 셰이더 유니폼/스토리지 버퍼 구조체 레이아웃
	class UniformStructLayout
	{
	public:
		enum class Layout : uint8_t
		{
			STD140, // UBO
			STD430  // SSBO
		};
		/// @brief 버퍼 타입
		enum class Kind : uint8_t
		{
			Uniform, // UBO
			Storage, // SSBO
			Sampler,
			PushConstant // 푸시 상수
		};
		/// @brief 용도
		enum class Usage : uint8_t
		{
			Camera = 0,
			Object = 1,
			Material = 2,
		};
		struct UniformMember
		{
			std::string name;
			std::size_t typeHash = 0;
			uint32_t offset = 0;
			uint32_t layoutSize = 0; // 바이트 (가변 배열이면 0)
			uint32_t baseAlignment = 0;
			uint32_t count = 1; // 배열 원소 개수 (단일이면 1)
			bool bDynamicArray = false; // SSBO

			auto IsArray() const -> bool { return count > 1 || bDynamicArray; }
		};
		struct LayoutInfo
		{
			uint32_t baseAlignment; // 오프셋의 정렬 단위
			uint32_t bytes; // 실제 차지하는 총 크기(패딩 포함)
		};
	public:
		SH_RENDER_API UniformStructLayout(std::string name, uint32_t binding, Usage type, ShaderStage stage, Kind kind = Kind::Uniform);
		SH_RENDER_API UniformStructLayout(std::string name, uint32_t binding, Usage type, ShaderStage stage, Kind kind, Layout layout);

		/// @brief 구조체의 맴버 변수를 추가. 구조체의 변수 순서대로 호출해야 한다.
		template<typename T>
		void AddMember(const std::string& name);
		/// @brief 구조체의 고정 배열 맴버를 추가. 구조체의 변수 순서대로 호출해야 한다.
		/// @param count 원소 개수 (>= 1)
		template<typename T>
		void AddArrayMember(const std::string& name, uint32_t count);
		/// @brief SSBO 끝에 들어가는 가변 배열 맴버 추가. 반드시 마지막에 호출.
		template<typename T>
		void AddDynamicArrayMember(const std::string& name);

		SH_RENDER_API auto GetMember(const std::string& name) const -> const UniformMember*;
		SH_RENDER_API auto GetMembers() const -> const std::vector<UniformMember>& { return members; }
		SH_RENDER_API auto HasMember(const std::string& name) const -> bool;

		/// @brief 구조체의 사이즈(바이트). 가변 배열 맴버가 있으면 고정 부분까지의 크기만 반환.
		SH_RENDER_API auto GetSize() const -> std::size_t;
		SH_RENDER_API auto GetLayout() const -> Layout { return layout; }
		SH_RENDER_API auto GetKind() const -> Kind { return kind; }
		SH_RENDER_API auto IsSampler() const -> bool { return kind == Kind::Sampler; }
		SH_RENDER_API auto IsPushConstant() const -> bool { return kind == Kind::PushConstant; }
	private:
		template<typename T>
		static auto GetLayoutInfo() -> LayoutInfo;

		template<typename T>
		void AddMemberImpl(const std::string& name, uint32_t count, bool bDynamic);
	public:
		const Kind kind;
		const Layout layout;
		const Usage usage;
		const ShaderStage stage;
		const uint32_t binding;
		const std::string name;
	private:
		std::vector<UniformMember> members;
	};
	template<typename T>
	inline auto UniformStructLayout::GetLayoutInfo() -> LayoutInfo
	{
		if constexpr (std::is_same_v<T, float>)
			return { 4, 4 };
		else if constexpr (std::is_same_v<T, int>)
			return { 4, 4 };
		else if constexpr (std::is_same_v<T, glm::vec2>)
			return { 8, 8 };
		else if constexpr (std::is_same_v<T, glm::vec3>)
			return { 16, 16 };
		else if constexpr (std::is_same_v<T, glm::vec4>)
			return { 16, 16 };
		else if constexpr (std::is_same_v<T, glm::mat2>)
			return { 16, 32 };
		else if constexpr (std::is_same_v<T, glm::mat3>)
			return { 16, 48 };
		else if constexpr (std::is_same_v<T, glm::mat4>)
			return { 16, 64 };
		else if constexpr (std::is_class_v<T>)
			return { alignof(T), sizeof(T)};
#if defined(_MSC_VER)
		else
			static_assert(std::_Always_false<T>, "Unknown type for GetLayoutInfo: " __FUNCSIG__);
#elif defined(__GNUC__) || defined(__clang__)
		else
			static_assert(core::alwaysFalse<T>, "Unknown type for GetLayoutInfo");
# else
		else
			static_assert(always_false<T>, "Unknown type for GetLayoutInfo: Unknown compiler");
#endif
	}
	template<typename T>
	inline void UniformStructLayout::AddMember(const std::string& name)
	{
		AddMemberImpl<T>(name, 1, false);
	}
	template<typename T>
	inline void UniformStructLayout::AddArrayMember(const std::string& name, uint32_t count)
	{
		AddMemberImpl<T>(name, count, false);
	}
	template<typename T>
	inline void UniformStructLayout::AddDynamicArrayMember(const std::string& name)
	{
		AddMemberImpl<T>(name, 0, true);
	}
	template<typename T>
	inline void UniformStructLayout::AddMemberImpl(const std::string& name, uint32_t count, bool bDynamic)
	{
		if constexpr (std::is_same_v<T, Texture>)
		{
			UniformMember member{};
			member.name = name;
			member.typeHash = core::reflection::GetType<Texture>().hash;
			member.count = count;
			member.bDynamicArray = bDynamic;
			members.push_back(std::move(member));
			return;
		}
		else
		{
			LayoutInfo info = GetLayoutInfo<T>();
			if (layout == Layout::STD140 && std::is_class_v<T>)
				info.baseAlignment = 16;
			const bool isArray = (count > 1 || bDynamic);

			// STD140: 배열 원소 stride는 최소 16바이트 정렬
			// STD430: 타입 고유 alignment 사용
			const uint32_t baseAlignment = (layout == Layout::STD140 && isArray)
				? std::max(info.baseAlignment, 16u)
				: info.baseAlignment;

			// 배열은 stride를 baseAlignment에 맞춤
			const uint32_t elementStride = isArray
				? core::Util::AlignTo(info.bytes, baseAlignment)
				: info.bytes;

			// 가변 배열은 런타임에 크기 결정
			const uint32_t totalSize = bDynamic ? 0 : (elementStride * count);

			UniformMember member{};
			member.name = name;
			member.typeHash = core::reflection::GetType<T>().hash;
			member.layoutSize = totalSize;
			member.baseAlignment = baseAlignment;
			member.count = bDynamic ? 0 : count;
			member.bDynamicArray = bDynamic;

			if (!members.empty())
			{
				const UniformMember& prev = members.back();
				member.offset = core::Util::AlignTo(prev.offset + prev.layoutSize, baseAlignment);
			}
			members.push_back(std::move(member));
		}
	}
}//namespace