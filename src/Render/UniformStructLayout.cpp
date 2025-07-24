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

	SH_RENDER_API auto UniformStructLayout::Serialize() const -> core::Json
	{
		core::Json json{};
		json["name"] = name;
		json["binding"] = binding;
		json["layout"] = (layout == Layout::STD140 ? "STD140" : "STD430");
		json["type"] = static_cast<int>(type);
		json["stage"] = static_cast<int>(stage);
		json["constant"] = bConstant;
		json["size"] = GetSize();

		core::Json membersJson = core::Json::array();
		for (auto const& m : members)
		{
			core::Json mjson;
			mjson["name"] = m.name;
			mjson["typeHash"] = m.typeHash;
			mjson["offset"] = m.offset;
			mjson["layoutSize"] = m.layoutSize;
			mjson["count"] = m.count;
			mjson["isArray"] = m.isArray;
			mjson["isSampler"] = m.isSampler;
			membersJson.push_back(std::move(mjson));
		}
		json["members"] = std::move(membersJson);

		return json;
	}
	SH_RENDER_API void UniformStructLayout::Deserialize(const core::Json& json)
	{
		members.clear();

		auto const& memArr = json.at("members");
		for (auto const& mjson : memArr)
		{
			std::string mName = mjson.at("name").get<std::string>();
			std::size_t typeHash = mjson.at("typeHash").get<std::size_t>();
			uint32_t offset = mjson.at("offset").get<uint32_t>();
			uint32_t layoutSize = mjson.at("layoutSize").get<uint32_t>();
			uint32_t count = mjson.at("count").get<uint32_t>();
			bool isArray = mjson.at("isArray").get<bool>();
			bool isSampler = mjson.at("isSampler").get<bool>();

			UniformMember member{ mName, typeHash, offset, layoutSize, count, isArray, isSampler };
			members.push_back(std::move(member));
		}
	}
}//namespace