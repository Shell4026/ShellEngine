#pragma once
#include "Export.h"

#include <cstdint>
#include <string>
#include <vector>

namespace sh::render
{
	class RenderTagList
	{
	public:
		struct Tag
		{
			std::string name;
			uint32_t id;
		};
	private:
		std::vector<Tag> tags;
	public:
		/// @brief 태그를 추가한다. 최대 32개까지 추가 가능.
		/// @param name 태그 이름
		SH_RENDER_API void AddTag(const std::string& name);
		/// @brief 태그의 id를 반환 하는 함수.
		/// @param name 태그 이름
		/// @return 해당 태그가 존재하면 태그의 id를 반환 한다. 없으면 0.
		SH_RENDER_API auto GetTagId(const std::string& name) const -> uint32_t;
		/// @brief 태그의 이름을 반환 하는 함수.
		/// @param tagId 태그 id
		/// @return 없으면 nullptr 있으면 string
		SH_RENDER_API auto GetTagName(uint32_t tagId) const -> const std::string*;
		/// @brief 모든 태그를 반환하는 함수.
		/// @return 태그 벡터
		SH_RENDER_API auto GetAllTags() const -> const std::vector<Tag>&;
	};
}//namespace