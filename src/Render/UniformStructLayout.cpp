#include "UniformStructLayout.h"

namespace sh::render
{
	UniformStructLayout::UniformStructLayout(const std::string name, uint32_t binding, Type type, ShaderStage stage, bool bConstant, Layout layout) :
		name(name), binding(binding), type(type), stage(stage), bConstant(bConstant), layout(layout)
	{

	}
	SH_RENDER_API auto UniformStructLayout::GetMember(const std::string& name) const -> const UniformMember*
	{
		for (auto& member : members)
		{
			if (member.name == name)
				return &member;
		}
		return nullptr;
	}
	SH_RENDER_API auto UniformStructLayout::HasMember(const std::string& name) const -> bool
	{
		for (auto& member : members)
		{
			if (member.name == name)
				return true;
		}
		return false;
	}
	SH_RENDER_API auto UniformStructLayout::GetSize() const -> std::size_t
	{
		if (members.back().offset == 0)
			return members.back().layoutSize;
		return members.back().offset + members.back().layoutSize;
	}
	SH_RENDER_API auto UniformStructLayout::GetMembers() const -> const std::vector<UniformMember>&
	{
		return members;
	}
}//namespace