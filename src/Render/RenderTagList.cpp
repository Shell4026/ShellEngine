#include "RenderTagList.h"

#include <algorithm>
#include <cassert>

namespace sh::render
{
	SH_RENDER_API void RenderTagList::AddTag(const std::string& name)
	{
		assert(tags.size() < 32);
		uint32_t nextId = 1 << tags.size();
		tags.push_back({ name, nextId });
	}
	SH_RENDER_API auto RenderTagList::GetTagId(const std::string& name) const -> uint32_t
	{
		auto it = std::find_if(tags.begin(), tags.end(), [&](const Tag& tag)
		{
			return tag.name == name;
		});
		if (it == tags.end())
			return 0;
		return it->id;
	}
	SH_RENDER_API auto RenderTagList::GetTagName(uint32_t tagId) const -> const std::string*
	{
		auto it = std::find_if(tags.begin(), tags.end(), [&](const Tag& tag)
		{
			return tag.id == tagId;
		});
		if (it == tags.end())
			return nullptr;
		return &it->name;
	}
	SH_RENDER_API auto RenderTagList::GetAllTags() const -> const std::vector<Tag>&
	{
		return tags;
	}
}//namespace