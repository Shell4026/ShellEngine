#include "Packet.h"

namespace sh::network
{
	Packet::Packet() :
		packetUUID(core::UUID::Generate())
	{
	}
	SH_NET_API auto Packet::GetPacketUUID() const -> const core::UUID&
	{
		return packetUUID;
	}
	SH_NET_API auto Packet::Serialize() const -> core::Json
	{
		core::Json mainJson{};
		mainJson["id"] = GetId();
		mainJson["packetUUID"] = packetUUID.ToString();
		return mainJson;
	}
	SH_NET_API void Packet::Deserialize(const core::Json& json)
	{
		if (json.contains("packetUUID"))
			packetUUID = core::UUID{ json["packetUUID"].get<std::string>() };
	}
}//namespace