#include "UniformStructLayout.h"

namespace sh::render
{
	static auto DefaultLayoutForKind(UniformStructLayout::Kind kind) -> UniformStructLayout::Layout
	{
		return (kind == UniformStructLayout::Kind::Storage)
			? UniformStructLayout::Layout::STD430
			: UniformStructLayout::Layout::STD140;
	}

	UniformStructLayout::UniformStructLayout(std::string name, uint32_t binding, Usage usage, ShaderStage stage, Kind kind) :
		kind(kind), layout(DefaultLayoutForKind(kind)),
		usage(usage), stage(stage), binding(binding), name(std::move(name))
	{
	}
	UniformStructLayout::UniformStructLayout(std::string name, uint32_t binding, Usage usage, ShaderStage stage, Kind kind, Layout layout) :
		kind(kind), layout(layout),
		usage(usage), stage(stage), binding(binding), name(std::move(name))
	{
	}
	SH_RENDER_API auto UniformStructLayout::GetMember(const std::string& name) const -> const UniformMember*
	{
		for (const UniformMember& member : members)
		{
			if (member.name == name)
				return &member;
		}
		return nullptr;
	}
	SH_RENDER_API auto UniformStructLayout::HasMember(const std::string& name) const -> bool
	{
		return GetMember(name) != nullptr;
	}
	SH_RENDER_API auto UniformStructLayout::GetSize() const -> std::size_t
	{
		if (members.empty())
			return 0;

		const UniformMember& last = members.back();
		const uint32_t rawSize = last.offset + last.layoutSize;

		uint32_t maxAlignment = 0;
		for (const UniformMember& m : members)
			maxAlignment = std::max(maxAlignment, m.baseAlignment);
		if (maxAlignment == 0)
			return rawSize;
		return core::Util::AlignTo(rawSize, maxAlignment);
	}
}//namespace