#pragma once
#include "Export.h"
#include "ShaderEnum.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"

#include <glm/fwd.hpp>

#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>
namespace sh::render
{
	class Texture;

	/// @brief 셰이더 유니폼 구조체 레이아웃
	class UniformStructLayout
	{
	public:
		enum class Layout
		{
			STD140, STD430
		};
		enum class Type
		{
			Camera = 0, // 카메라 데이터
			Object = 1, // 객체마다 고유
			Material = 2, // 메테리얼간 공유
		};
		struct UniformMember
		{
			std::string name;
			core::reflection::TypeInfo& type;
			uint32_t offset;
			uint32_t layoutSize;
			uint32_t count;
			bool isArray = false;
			bool isSampler = false;
		};
		struct Std140LayoutInfo 
		{
			uint32_t baseAlignment; // 오프셋의 정렬 단위
			uint32_t std140Size; // 실제 차지하는 총 크기(패딩 포함)
		};
	private:
		std::vector<UniformMember> members;
	public:
		const Layout layout;
		const Type type;
		const ShaderStage stage;
		const uint32_t binding;
		const std::string name;
		const bool bConstant;
	private:
		template<typename T>
		static auto GetSTD140Layout() -> Std140LayoutInfo;
	public:
		SH_RENDER_API UniformStructLayout(const std::string name, uint32_t binding, Type type, ShaderStage stage, bool bConstant = false, Layout layout = Layout::STD140);
		/// @brief 구조체의 맴버 변수를 추가 하는 함수. 구조체의 변수 순서대로 호출 해야 한다.
		/// @tparam T 타입
		/// @param name 이름
		              template<typename T>
		              void AddMember(const std::string& name);
		/// @brief 구조체의 배열 맴버 변수를 추가 하는 함수. 구조체의 변수 순서대로 호출 해야 한다.
		/// @tparam T 타입
		/// @param name 이름
		/// @param count 배열 원소 갯수
					  template<typename T>
		              void AddArrayMember(const std::string& name, std::size_t count);

		SH_RENDER_API auto GetMember(const std::string& name) const -> const UniformMember*;
		SH_RENDER_API auto GetMembers() const -> const std::vector<UniformMember>&;
		SH_RENDER_API auto HasMember(const std::string& name) const -> bool;

		/// @brief 유니폼 구조체의 사이즈를 반환 하는 함수.
		/// @return 바이트
		SH_RENDER_API auto GetSize() const -> std::size_t;
	};
	template<typename T>
	inline auto UniformStructLayout::GetSTD140Layout() -> Std140LayoutInfo
	{
		if constexpr (std::is_same_v<T, float>)
			return { 4, 4 };
		else if (std::is_same_v<T, int>)
			return { 4, 4 };
		else if (std::is_same_v<T, glm::vec2>)
			return { 8, 8 };
		else if (std::is_same_v<T, glm::vec3>)
			return { 16, 16 };
		else if (std::is_same_v<T, glm::vec4>)
			return { 16, 16 };
		else if (std::is_same_v<T, glm::mat2>)
			return { 16, 32 };
		else if (std::is_same_v<T, glm::mat3>)
			return { 16, 48 };
		else if (std::is_same_v<T, glm::mat4>)
			return { 16, 64 };
		else
			static_assert(true, "Unknown type");
	}
	template<typename T>
	inline void UniformStructLayout::AddMember(const std::string& name)
	{
		AddArrayMember<T>(name, 1);
	}
	template<typename T>
	inline void UniformStructLayout::AddArrayMember(const std::string& name, std::size_t count)
	{
		if constexpr (std::is_same_v<T, Texture>)
		{
			UniformMember member{ name, core::reflection::GetType<Texture>(), 0, 0, count, (count > 1), true };
			members.push_back(std::move(member));
		}
		else
		{
			if (layout == Layout::STD140)
			{
				Std140LayoutInfo info = GetSTD140Layout<T>();
				uint32_t arrayBaseAlignment = (count > 1) ?
					std::max(info.baseAlignment, 16u) : info.baseAlignment; // 배열이면 최소 16정렬
				uint32_t arrayStride = (count > 1) ?
					arrayBaseAlignment : info.std140Size;
				uint32_t totalSize = arrayStride * static_cast<uint32_t>(count);

				UniformMember member{ name, core::reflection::GetType<T>(), 0, totalSize, count, (count > 1), false };
				if (members.empty())
				{
					members.push_back(std::move(member));
					return;
				}
				auto& prev = members.back();
				uint32_t prevEnd = prev.offset + prev.layoutSize;
				member.offset = core::Util::AlignTo(prevEnd, arrayBaseAlignment);
				members.push_back(std::move(member));
			}
		}
	}
}//namespace