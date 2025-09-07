#include "StringPacket.h"

namespace sh::network
{
	SH_NET_API auto StringPacket::GetId() const -> uint32_t
	{
		return 0;
	}
	SH_NET_API auto StringPacket::Serialize() const -> core::Json
	{
		core::Json mainJson = Packet::Serialize();
		mainJson["str"] = str;
		return mainJson;
	}
	SH_NET_API void StringPacket::Deserialize(const core::Json& json)
	{
		if (json.contains("str"))
			str = json["str"];
	}
	SH_NET_API void StringPacket::SetString(const std::string& str)
	{
		this->str = str;
	}
	SH_NET_API void StringPacket::SetString(std::string&& str)
	{
		this->str = std::move(str);
	}
	SH_NET_API auto StringPacket::GetString() const -> const std::string&
	{
		return str;
	}
}//names[ace